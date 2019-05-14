/*
 * Copyright (c) 2018 Amol Surati
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <list.h>

void init_list_head(struct list_head *head)
{
	head->next = head->prev = head;
}

char list_empty(struct list_head *head)
{
	return head->next == head;
}

static void list_add_between(struct list_head *n,
			     struct list_head *prev,
			     struct list_head *next)
{
	n->next = next;
	n->prev = prev;
	next->prev = n;
	prev->next = n;
}

void list_add(struct list_head *head, struct list_head *n)
{
	list_add_between(n, head, head->next);
}

void list_add_tail(struct list_head *head, struct list_head *n)
{
	list_add_between(n, head->prev, head);
}

void list_del(struct list_head *e)
{
	struct list_head *p, *n;
	n = e->next;
	p = e->prev;
	p->next = n;
	n->prev = p;
}

struct list_head *list_del_head(struct list_head *head)
{
	struct list_head *e;
	e = head->next;
	list_del(e);
	return e;
}
