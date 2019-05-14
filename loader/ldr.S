/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

.globl entry
.section .opd, "aw"
.align 3
entry:
	.quad .entry, .TOC.@tocbase, 0
	.previous

/* Assumption: 64bit, BE, HV mode, with IR/DR disabled. */
.section .text.startup
.globl .entry
.entry:
	/* Setup stack. No TOC setup. */
	lis	%r1, stack@h;
	ori	%r1, %r1, stack@l;

	/* Allocate the stack frame for .entry.
	 * This function only ever calls a single routine copy_payload(base).
	 * So space needs to be reserved for r3 in the parameter save area of
	 * the frame.
	 */

	/* The view of the stack frame for this function: 
	 *
	 * TOS:
	 * +0x00: SP save area or backchain (unused by .entry)
	 * +0x08: CR save area
	 * +0x10: LR save area
	 * +0x18: CC dword
	 * +0x20: LD dword
	 * +0x28: TOC save area
	 * +0x30: Parameter save area for r3
	 *
	 * +0x38: GPR save area for r3 (params passed by skiboot)
	 * +0x40: GPR save area for r4
	 * +0x48: GPR save area for r5
	 * +0x50: GPR save area for r6
	 * +0x58: GPR save area for r7
	 * +0x60: GPR save area for r8
	 * +0x68: GPR save area for r9
	 * +0x70: OutOfStack
	 */

	subi	%r1, %r1, 0x70;

	/* Skiboot passes r3 to r9. Save them. */
	std	%r3, 0x38(%r1)
	std	%r4, 0x40(%r1)
	std	%r5, 0x48(%r1)
	std	%r6, 0x50(%r1)
	std	%r7, 0x58(%r1)
	std	%r8, 0x60(%r1)
	std	%r9, 0x68(%r1)

	/* Assuming that .entry is within 64k */
	li	%r3, 0
	lis	%r3, .entry@h
	bl	copy_payload
	mr	%r10, %r3
	cmpdi	%r10, 0	/* 0 == null == failed. */
	beq	sink

	/*
	 * Restore the parameters passed by skiboot, and pass them to
	 * vmm.
	 */
	ld	%r3, 0x38(%r1)
	ld	%r4, 0x40(%r1)
	ld	%r5, 0x48(%r1)
	ld	%r6, 0x50(%r1)
	ld	%r7, 0x58(%r1)
	ld	%r8, 0x60(%r1)
	ld	%r9, 0x68(%r1)

	/* Free the stack. */
	addi	%r1, %r1, 0x70;

	/*
	 * r10 has the address of the func descr. Load the entry point
	 * and the TOC, and jump.
	 */

	/*
	 * Warning: This overwrites the TOC set by skiboot firmware.
	 * The exception dump that skiboot prints becomes unavailable.
	 */
	ld	%r2, 8(%r10)	/* TOC */
	ld	%r10, 0(%r10)	/* Entry point */

	mtlr	%r10
	blr
sink:
	b	sink

/* Stack is quadward (16 bytes) aligned. */
.align 4
.fill 1024, 1
stack:

/* Section to receive the vmm binary. */
.section .vmm, "ad"
.fill 8, 1
