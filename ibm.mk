#
# Copyright (c) 2019 Amol Surati
# SPDX-License-Identifier: GPL-3.0-or-later
#

CFLAGS += -D__IBM__
P9PATH = ~/p9/opt/ibm/systemsim-p9/run/p9
RUN = $(P9PATH)/power9 -f $(P9PATH)/linux/boot-linux-le-skiboot.tcl
