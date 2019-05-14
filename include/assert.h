/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <types.h>

#define assert(x)							\
	((x) ? (void)0 : assert_fail(STRINGIFY(x), __FILE__, __LINE__))

void	assert_fail(const char *msg, const char *file, int line);
#endif
