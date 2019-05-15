/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/* highest is the highest half-word, etc. */
#define load_imm64(r, i)						\
	lis	r, (i)@highest;						\
	ori	r, r, (i)@higher;					\
	sldi	r, r, 32;						\
	oris	r, r, (i)@h;						\
	ori	r, r, (i)@l;

/* Entry point function descriptor. */
.globl entry
.section .opd, "aw"
.align 3
entry:
	.quad .entry, .TOC.@tocbase, 0
.previous


/* Entry point */
/* Assumption: 64bit, BE, HV mode, with IR/DR disabled. */
/* r3 to r9 from skiboot's start_kernel. */
.globl .entry
.entry:
	/*addi	%r1, %r2, stack@toc */	/* Set stack. */

	/*
	 * r2 is set by the loader after parsing .entry's function
	 * descriptor.
	 */
	ld %r1, stack@got(%r2)

	/*
	 * %r1 has the address of stack, as required. But stack@toc can only
	 * access offsets of size 16-bits from the TOC. For anything larger,
	 * the address of the stack can be placed in GOT, and the address of
	 * that GOT entry can be accessed through TOC. stack@got
	 * addi %r1, %r2, stack@got, or to load stack directly
	 * ld %r1, stack@got(%r2)
	 */

	/*
	 * main is passsed r3 to r9. Allocate parameter save area for them.
	 * And build the stack frame for .entry
	 */

	/* The view of the stack for this function: 
	 *
	 * TOS:
	 * +0x00: SP save area or backchain (unused by .entry)
	 * +0x08: CR save area
	 * +0x10: LR save area
	 * +0x18: CC dword
	 * +0x20: LD dword
	 * +0x28: TOC save area
	 *
	 * +0x30: Parameter save area for r3
	 * +0x38: Parameter save area for r4
	 * +0x40: Parameter save area for r5
	 * +0x48: Parameter save area for r6
	 * +0x50: Parameter save area for r7
	 * +0x58: Parameter save area for r8
	 * +0x60: Parameter save area for r9
	 * +0x68: Padding for qword alignment
	 * +0x70: OutOfStack
	 */

	subi	%r1, %r1, 0x70
	bl	main
sink:
	b	sink

/* Another way to set a got entry for stack. Usage: ld %r1, STACK@toc(%r2)
.section .toc, "aw"
STACK:
	.tc stack[TC],stack
.previous
*/


.section .bss
.globl htaborg
.globl parttaborg

/*
 * HTABORG must be 2^18-byte aligned. We keep HTABSIZE as 0, forcing the
 * page table to contain the minimum required 2^11 128-byte PTEGs.
 * Thus, the table is 2^(11 + 7) = 2^18 bytes in size.
 */
.align 18
htaborg:
.fill 32768, 8

/* Partition Table is aligned to its size, with the minimum size being 4KB. */
.align 12
parttaborg:
.fill 4096

.align 4
.fill 4096
stack:

