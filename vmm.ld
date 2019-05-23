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
	 * Keep the size of PHDRS <= 0x100, since the loader copies
	 * PHDRS into the first 0x100 bytes of physical space.
	 */
	text PT_LOAD FLAGS(5);		/* r-x */
	rodata PT_LOAD FLAGS(4);	/* r-- */
	data PT_LOAD FLAGS(6);		/* rw- */
}


/*
 * Real [0, 0x100) available for software use.
 * Real [0x100, 0x1000) are exception vectors.
 * Real [0x1000, 0x3000) reserved.
 *
 * At present, the loader copies over the reserved area.
 */
SECTIONS
{
	. = KVA_BASE;

	.excp.nommu : AT(ADDR(.excp.nommu) - KVA_BASE) {
		*(.excp.nommu);
	} :text

	. = ALIGN(0x4000);
	.text : AT(ADDR(.text) - KVA_BASE) {
		*(.excp.mmu);	/* Must be first. */
		. = ALIGN(0x1000);
		*(.text.startup);
		*(.text);
	}

	. = ALIGN(0x10000);	/* Base Page Size. */
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

	. = ALIGN(0x10000);	/* Base Page Size. */
	.data : AT(ADDR(.data) - KVA_BASE) {
		*(.data);
	} :data

	.bss : AT(ADDR(.bss) - KVA_BASE) {
		*(.bss);
	}

	/* NOLOAD sections follow for large allocations. */

	/*
	 * Partition table is naturally aligned. P9's partition table size
	 * is fixed at 64KB.
	 */
	. = ALIGN(0x10000);
	.parttab (NOLOAD) : {
		_parttab_org = .;
		. += 0x10000;
		_parttab_end = .;
	}

	/*
	 * HTABORG must be 2^18-byte aligned. We keep HTABSIZE as 0, forcing the
	 * page table to contain the minimum required 2^11 128-byte PTEGs.
	 * Thus, the table is 2^(11 + 7) = 2^18 bytes in size.
	 */
	. = ALIGN(0x40000);
	.htab (NOLOAD) : {
		_htab_org = .;
		. += 0x40000;
		_htab_end = .;
		_htab1_org = .;
		. += 0x40000;
		_htab1_end = .;
		_ram1_org = .;
	}
	_end = .;
}
