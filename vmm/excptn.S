/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

.macro excp_nommu off, entry
	.org \off
	excp_nommu_\entry:
		b	.
.endm

.macro excp_mmu off, entry
	.org \off
	excp_mmu_\entry:
		b	.
.endm

.section .excp.nommu, "ax"
excp_nommu 0x100, sreset
excp_nommu 0x200, mcheck
excp_nommu 0x300, dstr
excp_nommu 0x380, dseg
excp_nommu 0x400, istr
excp_nommu 0x480, iseg
excp_nommu 0x500, extern
excp_nommu 0x600, align
excp_nommu 0x700, program
excp_nommu 0x800, nofpu
excp_nommu 0x900, dec
excp_nommu 0x980, hdec
excp_nommu 0xa00, dpd
excp_nommu 0xc00, sc
excp_nommu 0xd00, trace
excp_nommu 0xe00, hdstr
excp_nommu 0xe20, histr
excp_nommu 0xe40, hemul
excp_nommu 0xe60, hmntn
excp_nommu 0xe80, dhd
excp_nommu 0xea0, hv
excp_nommu 0xf00, perf
excp_nommu 0xf20, novect
excp_nommu 0xf40, novsx
excp_nommu 0xf60, nofac
excp_nommu 0xf80, nohfac

.section .excp.mmu, "ax"
excp_mmu 0x300, dstr
excp_mmu 0x380, dseg
excp_mmu 0x400, istr
excp_mmu 0x480, iseg
excp_mmu 0x500, extern
excp_mmu 0x600, align
excp_mmu 0x700, program
excp_mmu 0x800, nofpu
excp_mmu 0x900, dec
excp_mmu 0x980, hdec
excp_mmu 0xa00, dpd
excp_mmu 0xc00, sc
excp_mmu 0xd00, trace
excp_mmu 0xe00, hdstr
excp_mmu 0xe20, histr
excp_mmu 0xe40, hemul
excp_mmu 0xe60, hmntn
excp_mmu 0xe80, dhd
excp_mmu 0xea0, hv
excp_mmu 0xf00, perf
excp_mmu 0xf20, novect
excp_mmu 0xf40, novsx
excp_mmu 0xf60, nofac
excp_mmu 0xf80, nohfac
