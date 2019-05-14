#
# Copyright (c) 2019 Amol Surati
# SPDX-License-Identifier: GPL-3.0-or-later
#

# GNU Make

LDS = ldr.ld
ELF = ldr.elf
INC = ../include

SRC = ldr.S ldr.c

OBJ = $(addsuffix .o,$(SRC))
DEP = $(addsuffix .d,$(SRC))

all: $(ELF)

include ../Makefile.build

c:
	rm -f $(ELF) $(OBJ) $(DEP) d.txt
