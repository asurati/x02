/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <bits.h>
#include <cpu.h>
#include <mmu.h>
#include <vmm.h>

#define EPAPR_MAGIC			0x65504150

static struct as vmm_as;
volatile int x = 0xdeadf00d;
void main(uint64_t r3, uint64_t r4, uint64_t r5, uint64_t r6, uint64_t r7,
	  uint64_t r8, uint64_t r9)
{
	uint64_t v, i, j;

	/* booting-without-of.txt in Linux kernel doc. */
	/* See also ePAPR v1.1, and skiboot's start_kernel source. */

	/* r3 = dtb addr (8 byte aligned)
	 * r4 = 0
	 * r5 = 0
	 * r6 = EPAPR_MAGIC = 0x65504150
	 *
	 * Below are skiboot specific:
	 * r7 = TotalMemory (not valid if "maxmem" not found dtb)
	 * r8 = skiboot base
	 * r9 = skiboot entry
	 */

	/* Return if not a skiboot boot. */
	if (!ALIGNED(r3, 8) || r4 != 0 || r5 != 0 || r6 != EPAPR_MAGIC)
		return;

	/* We need 64bit, HVMode, BigEndian, with IR/DR disabled. */
	mfmsr(v);
	i = 0;
	i |= bits_on(MSR_SF);
	i |= bits_on(MSR_HV);
	j = i;
	i |= bits_on(MSR_PR);
	i |= bits_on(MSR_IR);
	i |= bits_on(MSR_DR);
	i |= bits_on(MSR_LE);
	if ((v & i) != j)
		return;

	vmm_part_init();
	mmu_init(&vmm_as);

	/*
	 * The below switch to a partition cannot be done as it is;
	 * vmm must switch itself to real mode and then launch the
	 * guest.
	 */
	v = 0xdeadf00dcafebabc;
	mtspr(SPR_HSRR0, v);
	mfmsr(v);
	v &= bits_off(MSR_HV);
	mtspr(SPR_HSRR1, v);
	hrfid();
	for (;;)
		++x;
	(void)r7;
	(void)r8;
	(void)r9;
}
