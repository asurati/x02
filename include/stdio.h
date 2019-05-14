/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stddef.h>
#include <stdarg.h>

int	snprintf(char *str, size_t sz, const char *fmt, ...);
int	vsnprintf(char *str, size_t sz, const char *fmt, va_list arg);
#endif
