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
	extern char parttaborg;
	extern char htaborg;
	struct part_entry *pe;
	uint64_t v, w, ptab;

	/* Fill PTCR with the partition table address. */
	ptab = EA_TO_RA(&parttaborg);

	/*
	 * Fill the first entry in the partition table,
	 * for LPID = 0 = hypervisor.
	 */
	pe = (struct part_entry *)(&parttaborg);

	w = EA_TO_RA(&htaborg);/* TODO check alignment */
	w >>= 18;	/* htaborg is high order 42 bits. */

	v  = 0;
	v |= bits_set(HPARTE_HTABORG, w);
	/* The rest of the fields are kept 0. */
	pe->v[0]  = v;
	pe->v[1]  = 0;

	/* Turn on AIL. */
	mfspr(v, SPR_LPCR);
	v |= bits_on(LPCR_AIL);

	hwsync();	/* Let the updates propagate. */

	ptesync();
	mtspr(SPR_PTCR, ptab);
	isync();
	mtspr(SPR_LPCR, v);
	isync();
	mtspr(SPR_LPIDR, 0);
	isync();
}
