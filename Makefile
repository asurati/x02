#
# Copyright (c) 2019 Amol Surati
# SPDX-License-Identifier: GPL-3.0-or-later
#

# GNU Make

MAKEFLAGS += -rR
MACH ?= qemu

TGT = ppc64-linux-gnu
CC = $(TGT)-gcc
LD = $(TGT)-ld
OC = $(TGT)-objcopy
OD = $(TGT)-objdump

INC = ./include
LDFLAGS = -n -nostdlib -nopie
AFLAGS = -mbig
CFLAGS = -c -MMD -MP -O0 -ansi -pedantic -pedantic-errors		\
	 -ffreestanding -fno-builtin -fomit-frame-pointer		\
	 -fno-exceptions -fno-unwind-tables				\
	 -fno-asynchronous-unwind-tables -fno-pie -fno-common		\
	 -Wall -Wextra -Werror -Wshadow -Wfatal-errors -Wno-long-long	\
	 -mcpu=power9 -mbig -mno-vsx -mno-altivec

export CC LD OC LDFLAGS AFLAGS CFLAGS

LDS = vmm.ld
ELF = vmm.elf
LDR = loader
PKG = pkg

# Subdirectories to build
#SRC = $(shell find vmm -name '*.[cS]')
SRC += vmm/head.S
SRC += vmm/excptn.S

SRC += vmm/main.c
SRC += vmm/mmu.c

OBJ = $(addsuffix .o,$(SRC))
DEP = $(addsuffix .d,$(SRC))

all: $(PKG)

# Embed vmm inside the ldr binary
OCFLAGS  = --update-section .vmm=$(ELF)

$(PKG): $(LDR) $(ELF)
	$(info pkg)
	@$(OC) $(OCFLAGS) $(LDR)/ldr.elf $(PKG)

$(LDR):
	$(MAKE) -C $(LDR)

include Makefile.build
include $(MACH).mk

r:
	$(RUN)
rd:
	$(RUND)
d:
	$(OD) -d $(ELF) > d.txt
x:
	$(OD) -x $(ELF)
c:
	$(MAKE) -C $(LDR) c
	rm -f $(ELF) $(OBJ) $(DEP) $(PKG) d.txt

.PHONY: r rd d x $(LDR)
