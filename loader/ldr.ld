/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/* Address of the function descriptor */
ENTRY(entry)

/* All in one place; expected by skiboot. */
PHDRS
{
	data PT_LOAD FILEHDR PHDRS;
}

SECTIONS
{
	. = 0x20000000 + SIZEOF_HEADERS;

	.text : {
		*(.text.startup);
		*(.text);
		*(.rodata);
		*(.rodata.*);
	} :data

	. = ALIGN(0x10);
	.opd : {
		*(.opd);
	}

	. = ALIGN(0x10);
	.got : {
		*(.toc);
		*(.got);
	}

	. = ALIGN(0x10);
	.data : {
		*(.data);
	}

	/* Keep at the end of this list. */
	. = ALIGN(0x10);
	.vmm : {
		*(.vmm);
	}
}
