/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stddef.h>

typedef uint32_t atomic_t;
typedef int64_t off_t;

#define ARRAY_SIZE(x)				(sizeof(x)/sizeof(x[0]))
#define STRINGIFY(x)				#x

#ifndef NULL
#define NULL					(void *)0
#endif

#define container_of(p, t, m)		(t *)((char *)p - offsetof(t, m))

static __inline__ void writell(void *p, uint64_t v)
{
	*(volatile uint64_t *)p = v;
}

static __inline__ uint64_t readll(void *p)
{
	return *(volatile uint64_t *)p;
}

static __inline__ void writel(void *p, uint32_t v)
{
	*(volatile uint32_t *)p = v;
}

static __inline__ uint32_t readl(void *p)
{
	return *(volatile uint32_t *)p;
}

extern void *memset(void *dest, int val, int sz);
extern void *memcpy(void *dest, const void *src, int sz);
#endif
