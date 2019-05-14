/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <elf.h>
#include <mmu.h>

void *memset(void *dest, int val, int sz)
{
	char *d = dest;

	while (sz--)
		*d++ = val;
	return dest;
}

void *memcpy(void *dest, const void *src, int sz)
{
	char *d = dest;
	const char *s = src;

	while (sz--)
		*d++ = *s++;
	return dest;
}

static int check_elf(const struct elf64_hdr *eh)
{
	/* Check identity. */
	if (eh->ident[0] != 0x7f || eh->ident[1] != 'E' ||
	    eh->ident[2] != 'L' || eh->ident[3] != 'F')
		return 0;

	/* Check if it is 64bit BE. */
	if (eh->ident[EI_CLASS] != ELFCLASS64 ||
	    eh->ident[EI_DATA] != ELFDATA2MSB)
		return 0;

	/* Check if executable. */
	if (eh->type != ET_EXEC)
		return 0;

	/* Check if ppc64. */
	if (eh->machine != EM_PPC64)
		return 0;

	return 1;
}

static uint64_t _copy_payload(const struct elf64_hdr *eh)
{
	int i;
	const struct elf64_phdr *ph;
	char *dest;
	const char *p, *src;
	uint64_t entry, sz;

	/*
	 * entry is the address of the FD inside opd section. Extract the
	 * actual entry point and the corresponding TOC base, and return.
	 */
	entry = 0;

	/*
	 * Copy the elf segments at their linked location.
	 * Note: This may destroy/overwrite existing exception handlers.
	 * TODO: parse dtb to ensure that opal-reserved regions are kept
	 * intact.
	 */
	p = (const char *)eh;
	ph = (const struct elf64_phdr *)(p + eh->phoff);
	for (i = 0; i < eh->phnum; ++i, ++ph) {
		/* skip if not LOAD. */
		if (ph->type != 1)
			continue;

		src   = p + ph->offset;
		dest  = (char *)ph->paddr;

		/* Copy filesz bytes. */
		memcpy(dest, src, ph->filesz);

		/* Zero the rest, if any. */
		memset(dest + ph->filesz, 0, ph->memsz - ph->filesz);

		/*
		 * If entry point has been found, do not attempt to find
		 * it again.
		 */
		if (entry)
			continue;

		/* Skip if it does not contain the entry point. */
		if (eh->entry < ph->vaddr ||
		    (ph->vaddr + ph->filesz) <= eh->entry)
			continue;

		/* Found the entry point. */
		entry = eh->entry - ph->vaddr + ph->paddr;
	}

	/*
	 * Copy the program header at PHDR_BASE, to allow the vmm
	 * to map its pages. The progam header must be <= 0x100 bytes
	 * in size.
	 */
	sz = eh->phnum * sizeof(*ph);
	if (sz > 0x100)
		return 0;

	p = (const char *)eh;
	ph = (const struct elf64_phdr *)(p + eh->phoff);
	memcpy(PHDRS_BASE, ph, sz);
	return entry;
}

/*
 * Returns the physical address of the function descriptor of the
 * entry point inside vmm.elf payload. Cannot be 0.
 */
uint64_t copy_payload(const char *base)
{
	int i, j;
	const char *p;
	const struct elf64_hdr *eh;
	const struct elf64_shdr *sh;
	const char *str;

	p = base;

	/* Loader itself must be a valid elf. */
	eh = (const struct elf64_hdr *)p;
	if (!check_elf(eh))
		return 0;

	sh = (const struct elf64_shdr *)(p + eh->shoff);
	str = p + sh[eh->shstrndx].offset;

	/* Search for the section by the name .vmm. */
	for (i = 0; i < eh->shnum; ++i, ++sh) {
		j = sh->name;
		if (str[j++] != '.')
			continue;
		if (str[j++] != 'v')
			continue;
		if (str[j++] != 'm')
			continue;
		if (str[j++] != 'm')
			continue;
		if (str[j++] != 0)
			continue;

		/* Found the section. */
		break;
	}

	if (i == eh->shnum)
		return 0;

	eh = (const struct elf64_hdr *)(p + sh->offset);
	if (!check_elf(eh))
		return 0;
	return _copy_payload(eh);
}
