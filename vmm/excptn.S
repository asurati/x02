/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

.macro excp mmu, off, entry
	.org \off
	excp_\mmu\()_\entry:
		b	.
.endm

.section .excp.nommu, "ax"
excp nommu, 0x100, sreset
excp nommu, 0x200, mcheck
excp nommu, 0x300, dstr
excp nommu, 0x380, dseg
excp nommu, 0x400, istr
excp nommu, 0x480, iseg
excp nommu, 0x500, extern
excp nommu, 0x600, align
excp nommu, 0x700, program
excp nommu, 0x800, nofpu
excp nommu, 0x900, dec
excp nommu, 0x980, hdec
excp nommu, 0xa00, dpd
excp nommu, 0xc00, sc
excp nommu, 0xd00, trace
excp nommu, 0xe00, hdstr
excp nommu, 0xe20, histr
excp nommu, 0xe40, hemul
excp nommu, 0xe60, hmntn
excp nommu, 0xe80, dhd
excp nommu, 0xea0, hv
excp nommu, 0xf00, perf
excp nommu, 0xf20, novect
excp nommu, 0xf40, novsx
excp nommu, 0xf60, nofac
excp nommu, 0xf80, nohfac

.section .excp.mmu, "ax"
excp mmu, 0x300, dstr
excp mmu, 0x380, dseg
excp mmu, 0x400, istr
excp mmu, 0x480, iseg
excp mmu, 0x500, extern
excp mmu, 0x600, align
excp mmu, 0x700, program
excp mmu, 0x800, nofpu
excp mmu, 0x900, dec
excp mmu, 0x980, hdec
excp mmu, 0xa00, dpd
excp mmu, 0xc00, sc
excp mmu, 0xd00, trace
excp mmu, 0xe00, hdstr
excp mmu, 0xe20, histr
excp mmu, 0xe40, hemul
excp mmu, 0xe60, hmntn
excp mmu, 0xe80, dhd
excp mmu, 0xea0, hv
excp mmu, 0xf00, perf
excp mmu, 0xf20, novect
excp mmu, 0xf40, novsx
excp mmu, 0xf60, nofac
excp mmu, 0xf80, nohfac
