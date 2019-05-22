/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <types.h>

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
#define hwsync()	__asm volatile("hwsync" ::: "memory")
#define eieio()		__asm volatile("eieio" ::: "memory")
#define ptesync()	__asm volatile("ptesync" ::: "memory")
#define tlbsync()	__asm volatile("tlbsync" ::: "memory")
#define slbmte(rs, rb)							\
	__asm volatile("isync\n\t"					\
		       "slbmte	%0, %1\n\t"				\
		       "isync\n\t" :: "r" (rs), "r" (rb) : "memory")

#define mtspr(spr, v)							\
	__asm volatile("mtspr	%0, %1" :: "i" (spr), "r" (v) : "memory")

#define mfspr(v, spr)							\
	__asm volatile("mfspr	%0, %1" : "=r" (v) : "i" (spr) : "memory")

#define mfmsr(v)	__asm volatile("mfmsr	%0" : "=r" (v) :: "memory")
#define mtmsr(v)	__asm volatile("mtmsr	%0" :: "r" (v) : "memory")
#endif
