/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <bits.h>
#include <elf.h>
#include <mmu.h>
#include <vmm.h>

void mmu_init(struct as *as)
{
	uint64_t v, w, esid, vsid, hash, hi, lo, va, ava;
	const struct elf64_phdr *ph;
	struct pteg *pteg;
	extern char htaborg;

	ph = (const struct elf64_phdr *)PHDRS_BASE;
	(void)ph;
	(void)as;

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

	v = 3 << (63 - 53);	/* IS=3 */
	__asm volatile("tlbie	%0, %1, 2, 0, 0\n\t"
		       "tlbie	%0, %1, 2, 1, 0\n\t"
		       "eieio\n\t"
		       "tlbsync\n\t"
		       "ptesync\n\t" :: "r" (v), "r" (0) : "memory");

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


	/* Fill the PTEs. */

	/*
	 * Since we fixed HTABSIZE to 0, the hash function contributes the
	 * minimum 11-bits, and we can ignore the AND-ADD operations.
	 */

	/* 0xc000....0000 to 0x0000....0000 mapped as S_RO. */

	va = KVA_BASE;	/* because we keep VSID = ESID. */
	hi = bits_get(va, HPTE_VA_HI);
	lo = bits_get(va, HPTE_VA_LO);
	lo >>= BASE_PGSZ_BITS;
	hash = hi ^ lo;
	hash = bits_get(hash, HPTE_HASH);

	pteg = ((struct pteg *)&htaborg) + hash;
	v = w = 0;
	ava = VA_TO_AVA(KVA_BASE);
	v |= bits_set(HPTE_AVA, ava);
	v |= bits_on(HPTE_L);
	v |= bits_on(HPTE_V);

	w |= bits_set(HPTE_LP, 1);
	w |= bits_on(HPTE_M);
	w |= bits_on(HPTE_PP0);
	w |= bits_set(HPTE_PP1, 2);

	pteg->pte[0].v[1] = w;
	eieio();
	pteg->pte[0].v[0] = v;
	ptesync();

	/* Test writing on S_RO address, */
	mfmsr(v);
	v |= bits_on(MSR_IR);
	v |= bits_on(MSR_DR);
	mtmsr(v);

	*(char *)KVA_BASE = 0x10;
	for (;;);
}
