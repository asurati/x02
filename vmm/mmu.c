/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <bits.h>
#include <elf.h>
#include <mmu.h>

uint64_t ea_to_ra(void *ea)
{
	return (uint64_t)ea - KVA_BASE;
}

void mmu_init(struct as *as)
{
	uint64_t v, w;
	const struct elf64_phdr *ph;
	struct hpart_entry *ptabe;
	extern char ptaborg;
	extern char htaborg;

	ph = (const struct elf64_phdr *)PHDRS_BASE;
	(void)ph;
	(void)as;

	/* Empty SLB. QEMU slbia doesn't support IH. */
	isync();
	slbmte(0,0);
	isync();
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

	/* Fill PTCR with the partition table address. */
	v = ea_to_ra(&ptaborg);
	mtspr(SPR_PTCR, v);




	/*
	 * Fill the first entry in the partition table,
	 * for partition ID 0 = hypervisor.
	 */
	ptabe = (struct hpart_entry *)(&ptaborg);

	w = ea_to_ra(&htaborg);/* TODO check alignment */
	w >>= 18;	/* htaborg is high order 42 bits. */

	v  = 0;
	v |= bits_set(HPARTE_HTABORG, w);
	/* The rest of the fields are kept 0. */

	ptabe[0].v[0]  = v;
	ptabe[0].v[1]  = 0;


	/* Fill the SLB entry. */
	v = w = 0;
}
