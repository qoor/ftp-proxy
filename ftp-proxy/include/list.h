/*
 * include/common/mini-clist.h
 * Circular list manipulation macros and structures.
 *
 * Copyright (C) 2002-2014 Willy Tarreau - w@1wt.eu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, version 2.1
 * exclusively.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PROXY_INCLUDE_LIST_H_
#define PROXY_INCLUDE_LIST_H_

/* these are circular or bidirectionnal lists only. Each list pointer points to
 * another list pointer in a structure, and not the structure itself. The
 * pointer to the next element MUST be the first one so that the list is easily
 * cast as a single linked list or pointer.
 */
struct list {
    struct list *n;	/* next */
    struct list *p;	/* prev */
};

/* a back-ref is a pointer to a target list entry. It is used to detect when an
 * element being deleted is currently being tracked by another user. The best
 * example is a user dumping the session table. The table does not fit in the
 * output buffer so we have to set a mark on a session and go on later. But if
 * that marked session gets deleted, we don't want the user's pointer to go in
 * the wild. So we can simply link this user's request to the list of this
 * session's users, and put a pointer to the list element in ref, that will be
 * used as the mark for next iteration.
 */
struct bref {
	struct list users;
	struct list *ref; /* pointer to the target's list entry */
};

/* a word list is a generic list with a pointer to a string in each element. */
struct wordlist {
	struct list list;
	char *s;
};

/* this is the same as above with an additional pointer to a condition. */
struct cond_wordlist {
	struct list list;
	void *cond;
	char *s;
};

/* First undefine some macros which happen to also be defined on OpenBSD,
 * in sys/queue.h, used by sys/event.h
 */
#undef LIST_HEAD
#undef LIST_INIT
#undef LIST_NEXT

/* ILH = Initialized List Head : used to prevent gcc from moving an empty
 * list to BSS. Some older version tend to trim all the array and cause
 * corruption.
 */
#define ILH		{ .n = (struct list *)1, .p = (struct list *)2 }

#define LIST_HEAD(a)	((void *)(&(a)))

#define LIST_INIT(l) ((l)->n = (l)->p = (l))

#define LIST_HEAD_INIT(l) { &l, &l }

#define __LIST_STRUCT_PTR_TYPE struct list*

/* adds an element at the beginning of a list ; returns the element */
#define LIST_ADD(lh, el) ({ (el)->n = (lh)->n; (el)->n->p = (lh)->n = (el); (el)->p = (lh); (el); })

/* adds an element at the end of a list ; returns the element */
#define LIST_ADDQ(lh, el) ({ (el)->p = (lh)->p; (el)->p->n = (lh)->p = (el); (el)->n = (lh); (el); })

/* adds the contents of a list <old> at the beginning of another list <new>. The old list head remains untouched. */
#define LIST_SPLICE(new, old) do {				     \
		if (!LIST_ISEMPTY(old)) {			     \
			(old)->p->n = (new)->n; (old)->n->p = (new); \
			(new)->n->p = (old)->p; (new)->n = (old)->n; \
		}						     \
	} while (0)

/* adds the contents of a list whose first element is <old> and last one is
 * <old->prev> at the end of another list <new>. The old list DOES NOT have
 * any head here.
 */
#define LIST_SPLICE_END_DETACHED(new, old, type) do {              \
		type __t;                             \
		(new)->p->n = (old);                         \
		(old)->p->n = (new);                         \
		__t = (old)->p;                              \
		(old)->p = (new)->p;                         \
		(new)->p = __t;                              \
	} while (0)

/* removes an element from a list and returns it */
#define LIST_DEL(el) ({ \
	__LIST_STRUCT_PTR_TYPE __ret = (el); \
	(el)->n->p = (el)->p; \
	(el)->p->n = (el)->n; \
	(__ret); \
})

/* removes an element from a list, initializes it and returns it.
 * This is faster than LIST_DEL+LIST_INIT as we avoid reloading the pointers.
 */
#define LIST_DEL_INIT(el, type) ({ \
	__LIST_STRUCT_PTR_TYPE __ret = (el);                        \
	__LIST_STRUCT_PTR_TYPE __n = __ret->n;                \
	__LIST_STRUCT_PTR_TYPE __p = __ret->p;                \
	__n->p = __p; __p->n = __n;                     \
	__ret->n = __ret->p = __ret;                    \
	__ret;                                          \
})

/* returns a pointer of type <pt> to a structure containing a list head called
 * <el> at address <lh>. Note that <lh> can be the result of a function or macro
 * since it's used only once.
 * Example: LIST_ELEM(cur_node->args.next, struct node *, args)
 */
#define LIST_ELEM(lh, pt, el) ((pt)(((void *)(lh)) - ((void *)&((pt)NULL)->el)))

/* checks if the list head <lh> is empty or not */
#define LIST_ISEMPTY(lh) ((lh)->n == (lh))

/* checks if the list element <el> was added to a list or not. This only
 * works when detached elements are reinitialized (using LIST_DEL_INIT)
 */
#define LIST_ADDED(el) ((el)->n != (el))

/* returns a pointer of type <pt> to a structure following the element
 * which contains list head <lh>, which is known as element <el> in
 * struct pt.
 * Example: LIST_NEXT(args, struct node *, list)
 */
#define LIST_NEXT(lh, pt, el) (LIST_ELEM((lh)->n, pt, el))


/* returns a pointer of type <pt> to a structure preceding the element
 * which contains list head <lh>, which is known as element <el> in
 * struct pt.
 */
#undef LIST_PREV
#define LIST_PREV(lh, pt, el) (LIST_ELEM((lh)->p, pt, el))

/*
 * Simpler FOREACH_ITEM macro inspired from Linux sources.
 * Iterates <item> through a list of items of type "typeof(*item)" which are
 * linked via a "struct list" member named <member>. A pointer to the head of
 * the list is passed in <list_head>. No temporary variable is needed. Note
 * that <item> must not be modified during the loop.
 * Example: list_for_each_entry(cur_acl, known_acl, list) { ... };
 */ 
#define list_for_each_entry(item, list_head, member, type)                      \
	for (item = LIST_ELEM((list_head)->n, type, member);     \
	     &item->member != (list_head);                                \
	     item = LIST_ELEM(item->member.n, type, member))

/*
 * Same as list_for_each_entry but starting from current point
 * Iterates <item> through the list starting from <item>
 * It's basically the same macro but without initializing item to the head of
 * the list.
 */
#define list_for_each_entry_from(item, list_head, member, type) \
	for ( ; &item->member != (list_head); \
	     item = LIST_ELEM(item->member.n, type, member))

/*
 * Simpler FOREACH_ITEM_SAFE macro inspired from Linux sources.
 * Iterates <item> through a list of items of type "typeof(*item)" which are
 * linked via a "struct list" member named <member>. A pointer to the head of
 * the list is passed in <list_head>. A temporary variable <back> of same type
 * as <item> is needed so that <item> may safely be deleted if needed.
 * Example: list_for_each_entry_safe(cur_acl, tmp, known_acl, list) { ... };
 */ 
#define list_for_each_entry_safe(item, back, list_head, member, type)           \
	for (item = LIST_ELEM((list_head)->n, type, member),     \
	     back = LIST_ELEM(item->member.n, type, member);     \
	     &item->member != (list_head);                                \
	     item = back, back = LIST_ELEM(back->member.n, type, member))


/*
 * Same as list_for_each_entry_safe but starting from current point
 * Iterates <item> through the list starting from <item>
 * It's basically the same macro but without initializing item to the head of
 * the list.
 */
#define list_for_each_entry_safe_from(item, back, list_head, member, type) \
	for (back = LIST_ELEM(item->member.n, type, member);     \
	     &item->member != (list_head);                                \
	     item = back, back = LIST_ELEM(back->member.n, type, member))

#endif

