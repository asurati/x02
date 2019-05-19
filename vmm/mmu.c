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
 * Since we fixed HTABSIZE to 0, the hash function contributes the
 * minimum 11-bits, and we can ignore the AND-ADD operations.
 */

static uint64_t mmu_va_to_hash(uint64_t va)
{
	uint64_t hi, lo, hash;

	hi = bits_get(va, HPTE_VA_HI);
	lo = bits_get(va, HPTE_VA_LO);
	lo >>= BASE_PGSZ_BITS;
	hash = hi ^ lo;
	hash = bits_get(hash, HPTE_HASH);
	return hash;
}

static void mmu_init_map(uint64_t va, uint64_t ra, size_t sz, int rwx)
{
	uint64_t hash, v, a, b, ava, r;
	uint8_t lp;
	struct pteg *pteg;
	struct pte *pte;
	extern char htaborg;
	int i;

	/* We attempt to create one PTE for each block of size BASE_PGSZ. */

	assert(va + sz > va);
	assert(BASE_PGSZ == _64KB);

	for (v = va, r = ra; v < va + sz; v += BASE_PGSZ, r += BASE_PGSZ) {
		r >>= BASE_PGSZ_BITS;	/* We keep p == b */
		lp = r & 0xf;		/* For 64KB p */
		r >>= 4;		/* The ARPN. */

		lp <<= 4;
		lp |= 1;		/* The 8bit LP value. */

		hash = mmu_va_to_hash(va);
		pteg = ((struct pteg *)&htaborg) + hash;

		/* Search for an empty slot. */
		for (i = 0; i < 8; ++i)
			if (!bits_get(pteg->pte[i].v[0], HPTE_V))
				break;

		assert(i < 8);
		pte = &pteg->pte[i];

		a = b = 0;
		ava = VA_TO_AVA(v);
		a |= bits_set(HPTE_AVA, ava);
		a |= bits_on(HPTE_L);
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
	}

	/* Order the updates wrt page table search. */
	ptesync();
}

void mmu_init(struct as *as)
{
	int i;
	uint64_t v, w, esid, vsid, va, ra;
	size_t sz;
	const struct elf64_phdr *ph;
	extern char htaborg, _end;

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

	esid = KVA_BASE >> _256MB_BITS;	/* Keep segsize set to 256MB. */
	vsid = esid;	/* Keep it simple for the moment. */

	v |= bits_set(SLB_B, SLB_B_256MB);
	v |= bits_set(SLB_VSID, vsid);
	v |= bits_on(SLB_L) | bits_set(SLB_LP, SLB_LP_64KB);
	v |= bits_on(SLB_KP);

	w |= bits_set(SLB_ESID, esid);
	w |= bits_on(SLB_V);
	w |= bits_set(SLB_IX, 0);
	slbmte(v, w);

	/* Save the SLB entry which maps the hypervisor. */
	as->slbe.v[0] = v;
	as->slbe.v[1] = w;

	/* Zero the page table. */
	va = (uintptr_t)&htaborg;
	sz = (uintptr_t)&_end - va;
	memset(&htaborg, 0, sz);	/* htab isn't in .bss */

	/* Map segments of the vmm binary. */
	ph = (const struct elf64_phdr *)PHDRS_BASE;
	for (i = 0; i < PHDRS_NUM; ++i) {
		va = ph[i].vaddr;	/* va = ea */
		ra = ph[i].paddr;
		sz = ALIGN_UP(ph[i].memsz, BASE_PGSZ);
		mmu_init_map(va, ra, sz, ph[i].flags);
	}

	/* Page table is not in a loadable segment; process it separately. */
	va = (uintptr_t)&htaborg;
	ra = EA_TO_RA(va);
	sz = (uintptr_t)&_end - va;
	mmu_init_map(va, ra, sz, 6);	/* rw- */

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

	mtmsr(v);
}
