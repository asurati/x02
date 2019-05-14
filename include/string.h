/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

size_t	strlen(const char *str);
void	*memcpy(void *dest, const void *src, size_t n);
void	*memset(void *dest, int val, size_t n);
#endif
