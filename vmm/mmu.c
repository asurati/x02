/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <bits.h>
#include <elf.h>
#include <mmu.h>
#include <vmm.h>

/*
static uint64_t mmu_hash_mask(const struct as *as)
{
	assert(as->htabsz >= 0 && as->htabsz <= 28);
	return (1ull << (as->htabsz + 11) - 1);
}
*/

static uint64_t mmu_va_hash(const struct as *as, uint64_t va)
{
	uint64_t hi, lo;

	(void)as;

	hi = bits_get(va, HPTE_VA_HI);
	lo = bits_get(va, HPTE_VA_LO);
	return hi ^ lo;
}

static int mmu_map_base_page(struct as *as, uint64_t va, uint64_t ra,
				      int rwx)
{
	int i, j;
	uint64_t hash[2], a, b, r, lp, ava;
	struct pteg *pteg;
	struct pte *pte;

	hash[HASH_PRIMARY] = hash[HASH_SECONDARY] = mmu_va_hash(as, va);

	hash[HASH_PRIMARY] &= as->hash_mask;
	hash[HASH_SECONDARY] = ~hash[HASH_SECONDARY];
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
	 * rrrr 0001, where rrrr are the least significant bits of the ARPN.
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

static int mmu_map(struct as *as, uint64_t va, uint64_t ra,
			    size_t sz, int rwx)
{
	uint64_t v, r;

	sz = ALIGN_UP(sz, BASE_PGSZ);
	for (v = va, r = ra; v < va + sz; v += BASE_PGSZ, r += BASE_PGSZ)
		mmu_map_base_page(as, v, r, rwx);

	/* Order the updates wrt page table search. */
	ptesync();
	return 0;
}

void mmu_init(struct as *as)
{
	int i;
	uint64_t v, w, esid, vsid, va, ra;
	size_t sz;
	const struct elf64_phdr *ph;
	extern char _htab_org, _htab_end, _parttab_org, _parttab_end;

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


	/* Fill the SLB entry. */
	v = w = 0;

	/* Keep segsize set to 256MB. */
	assert(SEG_SZ_BITS == _256MB_BITS);

	esid = KVA_BASE >> SEG_SZ_BITS;
	vsid = esid;	/* Keep it simple for the moment. */

	v |= bits_set(SLB_B, SLB_B_256MB);
	v |= bits_set(SLB_VSID, vsid);
	v |= bits_on(SLB_L) | bits_set(SLB_LP, SLB_LP_64KB);
	v |= bits_on(SLB_KP);	/* Key == true. */

	w |= bits_set(SLB_ESID, esid);
	w |= bits_on(SLB_V);
	w |= bits_set(SLB_IX, 0);
	slbmte(v, w);

	/* Save the SLB entry which maps the hypervisor. */
	as->slbe.v[0] = v;
	as->slbe.v[1] = w;

	/* Zero the page table. */
	sz = &_htab_end - &_htab_org;
	memset(&_htab_org, 0, sz);	/* htab isn't in .bss */
	assert(sz == 256 * 1024ull);	/* Minimum required for now. */

	as->htab = (struct pteg *)&_htab_org;
	as->hash_mask = 0x7ff;
	as->htabsz = 0;

	/* Map segments of the vmm binary. */
	ph = (const struct elf64_phdr *)PHDRS_BASE;
	for (i = 0; i < PHDRS_NUM; ++i) {
		va = ph[i].vaddr;	/* va = ea */
		ra = ph[i].paddr;
		sz = ph[i].memsz;
		mmu_map(as, va, ra, sz, ph[i].flags);
	}

	/* Partition table is not in a loadable segment; map it separately. */
	va = (uintptr_t)&_parttab_org;
	sz = (uintptr_t)&_parttab_end - va;
	ra = EA_TO_RA(va);
	mmu_map(as, va, ra, sz, 6);	/* rw- */

	/* Page table is not in a loadable segment; map it separately. */
	va = (uintptr_t)&_htab_org;
	sz = (uintptr_t)&_htab_end - va;
	ra = EA_TO_RA(va);
	mmu_map(as, va, ra, sz, 6);	/* rw- */

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
