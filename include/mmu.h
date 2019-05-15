/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _MMU_H_
#define _MMU_H_

#include <cpu.h>
#include <bits.h>

/* Must match with vmm.ld. */
#define KVA_BASE			0xc000000000000000ull

/* Keeping PHDRS_BASE 0/NULL causes undefined behaviour wrt memcpy. */
#define PHDRS_BASE			(void *)8

#define PHDRS_TEXT			1
#define PHDRS_RODATA			2
#define PHDRS_DATA			3

#define EA_TO_RA(v)			((uintptr_t)(v) - KVA_BASE)
#define VA_TO_AVA(v)			((uintptr_t)(v) >> 23)

#define _1KB_BITS			10
#define _1KB				(1ull << _1KB_BITS)
#define _1KB_MASK			(_1KB - 1)

#define PAGE_SIZE_BITS			12
#define PAGE_SIZE			(1ull << PAGE_SIZE_BITS)
#define PAGE_SIZE_MASK			(PAGE_SIZE - 1)

#define _64KB_BITS			16
#define _64KB				(1ull << _64KB_BITS)
#define _64KB_MASK			(_64KB - 1)

#define _1MB_BITS			20
#define _1MB				(1ull << _1MB_BITS)
#define _1MB_MASK			(_1MB - 1)

#define _2MB_BITS			21
#define _2MB				(1ull << _2MB_BITS)
#define _2MB_MASK			(_2MB - 1)

#define _256MB_BITS			28
#define _256MB				(1ull << _256MB_BITS)
#define _256MB_MASK			(_256MB - 1)

#define _1GB_BITS			30
#define _1GB				(1ull << _1GB_BITS)
#define _1GB_MASK			(_1GB - 1)

#define BASE_PGSZ			_64KB
#define BASE_PGSZ_BITS			_64KB_BITS
#define BASE_PGSZ_MASK			_64KB_MASK

#define HPTE_VA_HI_L			0
#define HPTE_VA_HI_R			35
#define HPTE_VA_LO_L			36
#define HPTE_VA_LO_R			63

#define HPTE_HASH_L			53
#define HPTE_HASH_R			63

#define PTCR_PATB_L			4
#define PTCR_PATB_R			51
#define PTCR_PATS_L			59
#define PTCR_PATS_R			63

#define HPARTE_HR_L			0
#define HPARTE_HR_R			0
#define HPARTE_HTABORG_L		4
#define HPARTE_HTABORG_R		45
#define HPARTE_VRMA_PS_L		56
#define HPARTE_VRMA_PS_R		58
#define HPARTE_HTABSZ_L			59
#define HPARTE_HTABSZ_R			63

/* In the format expected by slbmte and peers. */
#define SLB_B_L				0
#define SLB_B_R				1
#define SLB_VSID_L			2
#define SLB_VSID_R			51
#define SLB_KS_L			52
#define SLB_KS_R			52
#define SLB_KP_L			53
#define SLB_KP_R			53
#define SLB_N_L				54
#define SLB_N_R				54
#define SLB_L_L				55
#define SLB_L_R				55
#define SLB_C_L				56
#define SLB_C_R				56
#define SLB_LP_L			58
#define SLB_LP_R			59

#define SLB_ESID_L			0
#define SLB_ESID_R			35
#define SLB_V_L				36
#define SLB_V_R				36
#define SLB_IX_L			52
#define SLB_IX_R			63

#define SLB_B_256MB			0
#define SLB_LP_64KB			1

#define HPTE_AVA_L			12
#define HPTE_AVA_R			56
#define HPTE_L_L			61
#define HPTE_L_R			61
#define HPTE_H_L			62
#define HPTE_H_R			62
#define HPTE_V_L			63
#define HPTE_V_R			63

#define HPTE_PP0_L			0
#define HPTE_PP0_R			0
#define HPTE_B_L			4
#define HPTE_B_R			5
#define HPTE_ARPN_L			7
#define HPTE_ARPN_R			43
#define HPTE_LP_L			44
#define HPTE_LP_R			51
#define HPTE_R_L			55
#define HPTE_R_R			55
#define HPTE_C_L			56
#define HPTE_C_R			56
#define HPTE_W_L			57
#define HPTE_W_R			57
#define HPTE_I_L			58
#define HPTE_I_R			58
#define HPTE_M_L			59
#define HPTE_M_R			59
#define HPTE_G_L			60
#define HPTE_G_R			60
#define HPTE_N_L			61
#define HPTE_N_R			61
#define HPTE_PP1_L			62
#define HPTE_PP1_R			63

enum {
	UNIT_4KB,
	UNIT_PAGE = UNIT_4KB,
	UNIT_8KB,
	UNIT_16KB,
	UNIT_32KB,
	UNIT_64KB,
	UNIT_128KB,
	UNIT_256KB,
	UNIT_512KB,
	UNIT_1MB,
	UNIT_2MB,
	UNIT_4MB,
	UNIT_8MB,
	UNIT_16MB,	/* BUDDY_NLEVELS stops here. */
	UNIT_SUP,

	UNIT_32MB = UNIT_SUP,
	UNIT_64MB,
	UNIT_128MB,
	UNIT_256MB,
	UNIT_512MB,
	UNIT_1GB,
	UNIT_MAX
};

struct slbe {
	uint64_t v[2];
};

struct as {
	struct slbe slbe;
};

struct pte {
	uint64_t v[2];
};

struct pteg {
	struct pte pte[8];
};

void	mmu_init(struct as *as);
#endif
