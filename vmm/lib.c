/*
 * Copyright (c) 2018 Amol Surati
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* XXX: For now base is assumed to be 16. */
char *itoa(int val, char *str, int base)
{
	int i, j, shift;
	int w;
	unsigned int v;
	const char *hex = "0123456789abcdef";

	(void)base;

	if (val == 0) {
		str[0] = '0';
		str[1] = 0;
		return str;
	}

	v = val;
	shift = 28;
	for (i = 0, j = 0; i < 8; ++i, shift -= 4) {
		w = (v >> shift) & 0xf;

		/* Skip leading zeroes. */
		if (j == 0 && w == 0)
			continue;

		str[j++] = hex[w];
	}
	str[j] = 0;
	return str;
}

size_t strlen(const char *str)
{
	size_t len;
	for (len = 0; str[len]; ++len)
		;
	return len;
}

enum spec_type {
	ST_CHAR,
	ST_HEX
};

int vsnprintf(char *str, size_t sz, const char *fmt, va_list arg)
{
	size_t ret;
	unsigned int v32;
	static char ostr[8];
	int j, k, o, oflow;
	enum spec_type st;

	k = strlen(fmt);
	if (k == 0)
		return 0;

	--sz;	/* For NULL. */

	oflow = 0;
	for (j = 0, ret = 0; j < k;) {
		st = ST_CHAR;
		ostr[0] = fmt[j++];
		o = 1;

		/* % alone. */
		if (ostr[0] == '%' && j >= k)
			break;

		/* %x, %d, etc. */
		if (ostr[0] == '%' && fmt[j] != '%') {
			if (fmt[j] == 'x')
				st = ST_HEX;
			else
				break;
		}
		++j;

		if (st == ST_HEX)
			o = 8;

		if (oflow || (ret + o) > sz) {
			ret += o;
			oflow = 1;
			continue;
		}

		if (st == ST_HEX) {
			/* Number. */
			v32 = va_arg(arg, unsigned int);
			itoa(v32, ostr, 16);
		}
		memcpy(&str[ret], ostr, o);
		ret += o;
		str[ret] = 0;
	}
	return ret;
}

int snprintf(char *str, size_t sz, const char *fmt, ...)
{
	int ret;
	va_list arg;

	va_start(arg, fmt);
	ret = vsnprintf(str, sz, fmt, arg);
	va_end(arg);
	return ret;
}

void *memset(void *dest, int val, size_t sz)
{
	char *d = dest;

	while (sz > 0) {
		*d = val;
		++d;
		--sz;
	}
	return dest;
}

void *memcpy(void *dest, const void *src, size_t sz)
{
	char *d = dest;
	const char *s = src;

	while (sz > 0) {
		*d = *s;
		++d;
		++s;
		--sz;
	}
	return dest;
}
