/*
 * Copyright (c) 2019 Amol Surati
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _BITS_H_
#define _BITS_H_

/* a must be a power of 2. */
#define ALIGN_DN(v, a)		((v) & ~((a) - 1))
#define ALIGN_UP(v, a)		(((v) + (a) - 1) & ~((a) - 1))
#define ALIGNED(v, a)		(((v) & ((a) - 1)) == 0)

#define bits_shift(f)		(63 - f##_R)
#define bits_mask(f)		((1ull << (f##_R - f##_L + 1)) - 1)

/* get/set: shifts are applied to the value. */
#define bits_set(f, v)		(((v) & bits_mask(f)) << bits_shift(f))
#define bits_get(v, f)		(((v) >> bits_shift(f)) & bits_mask(f))

/* push/pull: shifts are applied to the mask. */
#define bits_push(f, v)		((v) & bits_on(f))
#define bits_pull(v, f)		bits_push(f, v)

#define bits_on(f)		(bits_mask(f) << bits_shift(f))
#define bits_off(f)		~bits_on(f)
#endif
