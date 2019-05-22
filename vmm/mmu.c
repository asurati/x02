/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <bits.h>
#include <elf.h>
#include <mmu.h>
#include <vmm.h>

static const uint64_t seg_sizes[SEGSZ_MAX] = {_256MB, _1TB};
static const uint64_t segsz_bits[SEGSZ_MAX] = {_256MB_BITS, _1TB_BITS};

static void mmu_va_add(struct va *va, uint64_t off)
{
	uint64_t lo;

	lo = va->lo;
	va->lo += off;
	if (va->lo < lo)
		++va->hi;
	assert((va->hi & (~VA_HI_MASK)) == 0);
}

/*
static uint64_t mmu_hash_mask(const struct as *as)
{
	assert(as->htabsz >= 0 && as->htabsz <= 28);
	return (1ull << (as->htabsz + 11) - 1);
}
*/

static uint64_t mmu_va_hash(const struct as *as, enum seg_size segsz,
			    const struct va *va)
{
	uint64_t hi, lo, mi;

	(void)as;

	if (segsz == SEGSZ_256MB) {
		lo = bits_get(va->lo, HPTE_256MB_VA_LO);
		hi = bits_get(va->hi, HPTE_256MB_VA_HI);
		return hi ^ lo;
	} else {
		lo  = bits_get(va->lo, HPTE_1TB_VA_LO);
		mi  = bits_get(va->lo, HPTE_1TB_VA_MI_LO);
		mi |= bits_get(va->hi, HPTE_1TB_VA_MI_HI) <<
			bits_width(HPTE_1TB_VA_MI_LO);
		hi  = bits_get(va->lo, HPTE_1TB_VA_HI) <<
			(39 - bits_width(HPTE_1TB_VA_HI));
		return hi ^ mi ^ lo;
	}
}

static int mmu_map_base_page(struct as *as, const struct slbe *slbe,
			     const struct va *va, uint64_t ra, int rwx)
{
	int i, j;
	uint64_t hash[2], a, b, r, lp, ava;
	struct pteg *pteg;
	struct pte *pte;

	hash[HASH_PRIMARY] = mmu_va_hash(as, slbe->segsz, va);
	hash[HASH_SECONDARY] = ~hash[HASH_PRIMARY];

	hash[HASH_PRIMARY] &= as->hash_mask;
	hash[HASH_SECONDARY] &= as->hash_mask;

	/* Search for a primary slot. */
	for (i = HASH_PRIMARY; i <= HASH_SECONDARY; ++i) {
		pteg = &as->htab[hash[i]];
		for (j = 0; j < 8; ++j)
			if (!bits_get(pteg->pte[j].v[0], HPTE_V))
				break;
		if (j < 8)
			break;
	}

	assert(i <= HASH_SECONDARY && j < 8);
	pte = &pteg->pte[j];

	r = ra >> BASE_PGSZ_BITS;	/* Convert to page num. */

	/*
	 * The LP field inside PTE for 64KB base and 64KB actual page sizes is
	 * rrrr 0001, where rrrr are the least significant bits of the RPN.
	 */

	lp = ((r & 0xf) << 4) | 1ull;
	r >>= 4;	/* ARPN. */

	a = b = 0;
	ava = VA_TO_AVA(va);
	a |= bits_set(HPTE_AVA, ava);
	a |= bits_on(HPTE_L);	/* For 64KB:64KB */
	a |= bits_on(HPTE_V);

	b |= bits_set(HPTE_ARPN, r);
	b |= bits_set(HPTE_LP, lp);
	b |= bits_on(HPTE_M);
	if ((rwx & 1) == 0)	/* no execute */
		b |= bits_on(HPTE_N);

	if ((rwx & 2) == 0) {	/* read only */
		b |= bits_on(HPTE_PP0);
		b |= bits_set(HPTE_PP1, 2);
	}

	/*
	 * Ignoring the eieio ordering between the two statements,
	 * and the ptesync after. Fixup when enabling SMP.
	 */
	pte->v[1] = b;
	pte->v[0] = a;
	return 0;
}

static int mmu_map(struct as *as, int ix, off_t off, uint64_t ra,
		   size_t sz, int rwx)
{
	uint64_t r;
	struct va va;
	const struct slbe *slbe;

	slbe = &as->slb[ix];
	assert(ALIGNED(off, slbe->bps));
	assert(ALIGNED(ra, slbe->bps));
	sz = ALIGN_UP(sz, slbe->bps);

	va = slbe->va;
	mmu_va_add(&va, off);
	for (r = ra; sz; r += slbe->bps, sz -= slbe->bps) {
	     mmu_map_base_page(as, slbe, &va, r, rwx);
	     mmu_va_add(&va, slbe->bps);
	}

	/* Order the updates wrt page table search. */
	ptesync();
	return 0;
}

/* Caller ascertains that each segment is disjoint. */
static int mmu_create_segment(struct as *as, uint64_t ea, const struct va *va,
			      enum base_page_size bps, enum seg_size segsz,
			      char kp, char ks, char n)
{
	int i, bits;
	uint64_t esid, vsid, v, w;

	assert(bps < BPS_MAX);
	assert(segsz < SEGSZ_MAX);

	assert(ALIGNED(ea, seg_sizes[segsz]));
	assert(ALIGNED(va->lo, seg_sizes[segsz]));
	assert((va->hi & (~VA_HI_MASK)) == 0);

	v = w = 0;

	bits = segsz_bits[segsz];
	esid = EA_ESID(ea, bits);
	vsid = VA_VSID(va->hi, va->lo, bits);

	v |= bits_set(SLBE_VSID, vsid);
	if (bps == BPS_64KB)
		v |= bits_on(SLBE_L) | bits_set(SLBE_LP, SLBE_LP_64KB);
	if (segsz == SEGSZ_1TB)
		v |= bits_set(SLBE_B, SLBE_B_1TB);
	if (kp)
		v |= bits_on(SLBE_KP);
	if (ks)
		v |= bits_on(SLBE_KS);
	if (n)
		v |= bits_on(SLBE_N);

	w |= bits_set(SLBE_ESID, esid);
	w |= bits_on(SLBE_V);

	for (i = 0; i < SLBE_NUM; ++i)
		if (!bits_get(as->slb[i].rs, SLBE_V))
			break;
	assert(i < SLBE_NUM);
	w |= bits_set(SLBE_IX, i);

	as->slb[i].rs = v;
	as->slb[i].rb = w;
	as->slb[i].va = *va;
	as->slb[i].ea = ea;
	as->slb[i].segsz = segsz;
	if (bps == BPS_4KB)
		as->slb[i].bps = _4KB;
	else
		as->slb[i].bps = _64KB;
	return i;
}

void mmu_init(struct as *as)
{
	int i, ix;
	off_t off;
	uint64_t v, w, ra, ea;
	struct va va;
	size_t sz;
	const struct elf64_phdr *ph;
	extern char _htab_org, _htab_end, _parttab_org, _parttab_end;

	/* Zero out page table. */
	sz = &_htab_end - &_htab_org;
	memset(&_htab_org, 0, sz);	/* htab isn't in .bss */
	assert(sz == 256 * 1024ull);	/* Minimum required for now. */

	/* Initialize parameters. */
	as->htab = (struct pteg *)&_htab_org;
	as->htabsz = 11;
	as->hash_mask = (1ull << as->htabsz) - 1;

	/* Empty SLB. QEMU slbia doesn't support IH. */
	slbmte(0,0);
	slbia(4);
	isync();

	/* Empty
	 * process-scoped tlb-entries,
	 * process-scoped page-walk-cache (NA for HPT),
	 * process-table-cache.
	 *
	 * Empty
	 * partition-scoped tlb-entries,
	 * partition-scoped page-walk-cache (NA for HPT),
	 * partition-table-cache
	 */

	/* Given that we are in HV mode, these are the settings:
	 * HV=1 and IS=3  => all-entries
	 * R=0 => HPT-translations,
	 * RIC=2 and PRS=0 => part-scoped tlb, page-walk-cache, table-cache
	 * RIC=2 and PRS=1 => proc-scoped tlb, page-walk-cache, table-cache
	 */

	/* tlbie
	 * eieio
	 * tlbsync
	 * ptesync
	 */

	/*
	 * Setup an SLBE to map a 256MB segment from EA 0xc000.... to 
	 * RA 0x0000.... for the vmm.
	 */

	ea = va.lo = KVA_BASE;	/* ea == va for this segment. */
	va.hi = 0;
	ix = mmu_create_segment(as, ea, &va, BPS_64KB, SEGSZ_256MB, 1, 0, 0);

	/* Map segments of the vmm binary. */
	ph = (const struct elf64_phdr *)PHDRS_BASE;
	for (i = 0; i < PHDRS_NUM; ++i) {
		off = ph[i].vaddr - KVA_BASE;
		ra = ph[i].paddr;
		sz = ph[i].memsz;
		mmu_map(as, ix, off, ra, sz, ph[i].flags);
	}

	/* Partition table is not in a loadable segment; map it separately. */
	off = (uintptr_t)&_parttab_org - KVA_BASE;
	sz = &_parttab_end - &_parttab_org;
	ra = EA_TO_RA(&_parttab_org);
	mmu_map(as, ix, off, ra, sz, 6);	/* rw- */

	/* Page table is not in a loadable segment; map it separately. */
	off = (uintptr_t)&_htab_org - KVA_BASE;
	sz = &_htab_end - &_htab_org;
	ra = EA_TO_RA(&_htab_org);
	mmu_map(as, ix, off, ra, sz, 6);	/* rw- */

	slbmte(as->slb[ix].rs, as->slb[ix].rb);

	mfmsr(v);
	v |= bits_on(MSR_IR);
	v |= bits_on(MSR_DR);

	w = 3 << (63 - 53);
	__asm volatile("tlbie	%0, %1, 2, 0, 0\n\t"
		       "tlbie	%0, %1, 2, 1, 0\n\t"
		       :: "r" (w), "r" (0) : "memory");
	eieio();
	tlbsync();
	ptesync();

	/* Linux kernel fills in srr0/1 and then executes rfid. */
	mtmsr(v);
	return;
}
