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

	v = 0xdeadf00dcafebabc;
	mtspr(SPR_HSRR0, v);
	mfmsr(v);
	v &= bits_off(MSR_HV);
	mtspr(SPR_HSRR1, v);
	hrfid();
	for (;;)
		++x;
	(void)r3;
	(void)r4;
	(void)r5;
	(void)r6;
	(void)r7;
	(void)r8;
	(void)r9;
}

#if 0
#include <stdio.h>
#include <uart.h>
#include <mm.h>
#include <cpu.h>
#include <elf.h>

static struct as kas;
void mm_init(int id);
void euart_init();
void gic_init(int id);
void mmu_init(const struct elf64_hdr *eh, struct as *as);
void mmu_switch(struct tab *pmd);
void pm_init(const struct elf64_hdr *eh);
void vm_init(struct as *as);

void setup(int id)
{
	const struct elf64_hdr *eh;

	if (id == 0) {
		eh = (const struct elf64_hdr *)(SOC_RAM_BASE + _2MB);
		mmu_init(eh, &kas);
	}

	mmu_switch(kas.pmd);

	if (id == 0) {
		eh = (const struct elf64_hdr *)(KVA_BASE + _2MB);
		pm_init(eh);
		vm_init(&kas);
		euart_init();
	}
	euart_str("asdf\n");
	gic_init(id);
	__asm volatile("smc #0xdead");
	for (;;)
		wfi();
}
#endif
