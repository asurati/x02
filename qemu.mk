#
# Copyright (c) 2019 Amol Surati
# SPDX-License-Identifier: GPL-3.0-or-later
#

QPATH = ~/tools/qemu
QEMU = $(QPATH)/bin/qemu-system-ppc64

#-monitor tcp::4444,server,nowait
QFLAGS = -m 1g -serial mon:stdio -nographic -nodefaults
# when initrd is supplied during runtime, increase mem to 2g

QFLAGS += -d int,guest_errors,unimp,mmu
QFLAGS += -M powernv -bios skiboot.lid
QFLAGS += -cpu power9 -smp cores=4

CFLAGS += -D__QEMU__

# Qemu's KERNEL_LOAD_ADDR == 0x20000000 == skiboot's KERNEL_LOAD_BASE
RUN = $(QEMU) $(QFLAGS) -kernel $(PKG)
RUND = $(RUN) -s -S
