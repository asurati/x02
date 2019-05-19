/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <bits.h>
#include <elf.h>
#include <mmu.h>
#include <vmm.h>

void vmm_part_init()
{
	extern char parttaborg[];
	extern char htaborg;
	struct part_entry *pe;
	uint64_t v, w, ptab;

	/* Fill PTCR with the partition table address. */
	ptab = EA_TO_RA(&parttaborg);

	/*
	 * Fill the first entry in the partition table,
	 * for LPID = 0 = hypervisor.
	 */
	pe = (struct part_entry *)parttaborg;

	w = EA_TO_RA(&htaborg);/* TODO check alignment */
	w >>= 18;	/* htaborg is high order 42 bits. */

	v  = 0;
	v |= bits_set(HPARTE_HTABORG, w);
	v |= bits_set(HPARTE_VRMA_PS, 5);	/* b=64KB for VRMA SLB. */

	/* The rest of the fields are kept 0. */
	pe->v[0]  = v;
	pe->v[1]  = 0;

	/* Dummy, possibly invalid values. */
	pe[1].v[0] = -1;
	pe[1].v[1] = 0;

	mfspr(v, SPR_LPCR);
	v |= bits_on(LPCR_AIL);	/* Enable AIL. */
	v |= bits_on(LPCR_TC);	/* Disable 2ndary page table search. */

	/*
	 * For p9 cpu, VPM0 bit is considered by ISA to be always 1.
	 * QEMU keeps VPM0 as 0; it also does not support writing on it.
	 * The below statement is effective only because a modified
	 * qemu-system-ppc64 binary is being run.
	 */
	v |= bits_on(LPCR_VPM0);

	hwsync();	/* Order the updates. */

	ptesync();
	mtspr(SPR_PTCR, ptab);
	isync();
	mtspr(SPR_LPCR, v);
	isync();
	mtspr(SPR_LPIDR, 0);
	isync();
}
