/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <types.h>

/*
 * HPTMode implementation constants for power9.
 * Note that power9 implements 68 bits of VA space. Here, we limit it to
 * to 64 bits, for the convenience of using a single uint64_t.
 */
#define EA_BITS				64
#define VA_BITS				64
#define RA_BITS				51

/* MSR bits. */
#define MSR_SF_L			0
#define MSR_SF_R			0
#define MSR_HV_L			3
#define MSR_HV_R			3
#define MSR_PR_L			49
#define MSR_PR_R			49
#define MSR_IR_L			58
#define MSR_IR_R			58
#define MSR_DR_L			59
#define MSR_DR_R			59
#define MSR_LE_L			63
#define MSR_LE_R			63

#define slbia(ih)	__asm volatile("slbia " #ih ::: "memory")
#define isync()		__asm volatile("isync" ::: "memory")
#define slbmte(rs, rb)							\
	__asm volatile("slbmte	%0, %1" :: "r" (rs), "r" (rb) : "memory")



#define SPR_HSRR0			314
#define SPR_HSRR1			315
#define SPR_PTCR			464

#define hrfid()		__asm volatile("hrfid\n\t" ::: "memory");

#define mtspr(spr, v)							\
	__asm volatile("mtspr	%0, %1" :: "i" (spr), "r" (v) : "memory")

#define mfspr(v, spr)							\
	__asm volatile("mfspr	%0, %1" : "=r" (v) : "i" (spr) : "memory")

#define mfmsr(v)	__asm volatile("mfmsr	%0" : "=r" (v) :: "memory")

/* Move paritition items into vmm.h */
struct hpart_entry {
	uint64_t v[2];
};

#endif
