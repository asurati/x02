/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _VMM_H_
#define _VMM_H_

#include <cpu.h>

#define LPCR_AIL_L			39
#define LPCR_AIL_R			40

#define SPR_HSRR0			314
#define SPR_HSRR1			315
#define SPR_LPCR			318
#define SPR_LPIDR			319
#define SPR_PTCR			464

#define hrfid()		__asm volatile("hrfid\n\t" ::: "memory");

struct part_entry {
	uint64_t v[2];
};
void	vmm_part_init();
#endif
