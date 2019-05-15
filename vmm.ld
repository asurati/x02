/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Standard entry point. Address of the function descriptor
 * of the entry point.
 */
ENTRY(entry)

/*
 * Set EA:01 = 0b11. To allow us to run in hypervisor real mode, and
 * to take advantage of the 0xc000...4000 exception vectors.
 *
 * Must match with mmu.h
 */
KVA_BASE = 0xc000000000000000;

PHDRS
{
	/*
	 * Keep the size of phdr <= 0x100, since the loader copies
	 * PHDRS into the first 0x100 bytes of physical space.
	 */
	nommu PT_LOAD;
	text PT_LOAD FLAGS(5);		/* r-x */
	rodata PT_LOAD FLAGS(4);	/* r-- */
	data PT_LOAD FLAGS(6);		/* rw- */
}


/*
 * Real [0, 0x100) available for software use.
 * Real [0x100, 0x1000) are exception vectors.
 * Real [0x1000, 0x3000) reserved.
 */
SECTIONS
{
	/* We depend on SIZEOF_HEADERS to remain < 0x100 */
	. = 0;

	.excp.nommu : {
		*(.excp.nommu);
	} :nommu

	. += KVA_BASE;
	. = ALIGN(0x4000);

	.excp.mmu : AT(ADDR(.excp.mmu) - KVA_BASE) {
		*(.excp.mmu);
	} :text

	. = ALIGN(0x1000);
	.text : AT(ADDR(.text) - KVA_BASE) {
		*(.text.startup);
		*(.text);
	}




	. = ALIGN(0x10000);
	.rodata : AT(ADDR(.rodata) - KVA_BASE) {
		*(.rodata);
		*(.rodata.*);
	} :rodata

	.opd : AT(ADDR(.opd) - KVA_BASE) {
		*(.opd);
	}

	.got : AT(ADDR(.got) - KVA_BASE) {
		*(.toc);
		*(.got);
	}





	. = ALIGN(0x10000);
	.data : AT(ADDR(.data) - KVA_BASE) {
		*(.data);
	} :data

	.bss : AT(ADDR(.bss) - KVA_BASE) {
		*(.bss);
	}
	_end = .;
}
