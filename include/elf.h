/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _ELF_H_
#define _ELF_H_

#include <types.h>

struct elf64_hdr {
	unsigned char ident[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

struct elf64_phdr {
	uint32_t type;
	uint32_t flags;
	uint64_t offset;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesz;
	uint64_t memsz;
	uint64_t align;
};

struct elf64_shdr {
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	uint64_t vaddr;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t align;
	uint64_t entsz;
};


#define EI_CLASS						4
#define ELFCLASS64						2

#define EI_DATA							5
#define ELFDATA2MSB						2

#define ET_EXEC							2

#define EM_PPC64						21
#endif
