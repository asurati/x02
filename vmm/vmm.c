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
	extern char _parttab_org;
	extern char _parttab_end;
	extern char _htab_org;
	struct part_entry *pe;
	uint64_t v, w, ptab;
	size_t sz;

	/* Zero the partition table. */
	sz = &_parttab_end - &_parttab_org;
	memset(&_parttab_org, 0, sz);

	/*
	 * Fill the first entry in the partition table,
	 * for LPID = 0 = hypervisor.
	 */
	pe = (struct part_entry *)&_parttab_org;

	w = EA_TO_RA(&_htab_org);
	w >>= 18;	/* htaborg is high order 42 bits. */

	v = bits_set(HPARTE_HTABORG, w);
	/* VRMA_PS for LPID=0 is not used. */

	/* The rest of the fields are kept 0. */
	pe->v[0]  = v;
	pe->v[1]  = 0;

	/* Dummy, possibly invalid values for LPID == 1. */
	pe[1].v[0] = -1;
	pe[1].v[1] = 0;

	mfspr(v, SPR_LPCR);
	v |= bits_on(LPCR_AIL);	/* Enable AIL. */
	v |= bits_on(LPCR_TC);	/* Disable 2ndary page table search. */

	hwsync();	/* Order the updates. */

	/* Fill PTCR with the partition table address. */
	ptab = EA_TO_RA(&_parttab_org);

	ptesync();
	mtspr(SPR_PTCR, ptab);	/* PATS field is ignored by P9. */
	isync();
	mtspr(SPR_LPCR, v);
	isync();
	mtspr(SPR_LPIDR, 0);
	isync();
}
