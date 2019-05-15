/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cpu.h>
#include <stdlib.h>

#include <assert.h>

void assert_fail(const char *msg, const char *file, int line)
{
	/*
	static char str[5];

	itoa(line, str, 16);
	*/
	for (;;)
		;
	(void)msg;
	(void)file;
	(void)line;
}
