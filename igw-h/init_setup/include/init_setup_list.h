/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _INIT_SETUP_LIST_H_
#define _INIT_SETUP_LIST_H_

#include <stddef.h>

#define jd_container_of(ptr, type, member) ({			\
	typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define container_of_constant(ptr, type, member) ({			\
		typeof( ((type *)0)->member ) *__mptr = (ptr);	\
		(type *)( (const char *)__mptr - offsetof(type,member) );})

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
struct jd_list_head {
	struct jd_list_head *next, *prev;
};

#define JD_LIST_HEAD_INIT(name) { &(name), &(name) }

#define JD_LIST_HEAD(name) \
	struct jd_list_head name = JD_LIST_HEAD_INIT(name)

static inline void JD_INIT_LIST_HEAD(struct jd_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __jd_list_add(struct jd_list_head *new,
			      struct jd_list_head *prev,
			      struct jd_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * jd_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void jd_list_add(struct jd_list_head *new, struct 
jd_list_head *head)
{
	__jd_list_add(new, head, head->next);
}


/**
 * jd_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void jd_list_add_tail(struct jd_list_head *new, struct 
jd_list_head *head)
{
	__jd_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __jd_list_del(struct jd_list_head * prev, struct 
jd_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * jd_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: jd_list_empty() on entry does not return true after this, the entry 
is
 * in an undefined state.
 */
static inline void jd_list_del(struct jd_list_head *entry)
{
	__jd_list_del(entry->prev, entry->next);
}

/**
 * jd_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int jd_list_empty(const struct jd_list_head *head)
{
	return head->next == head;
}

/**
 * jd_list_entry - get the struct for this entry
 * @ptr:	the &struct jd_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the jd_list_struct within the struct.
 */
#define jd_list_entry(ptr, type, member) \
	jd_container_of(ptr, type, member)

/**
 * jd_list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the jd_list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define jd_list_first_entry(ptr, type, member) \
	jd_list_entry((ptr)->next, type, member)

/**
 * jd_list_for_each	-	iterate over a list
 * @pos:	the &struct jd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define jd_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * __jd_list_for_each	-	iterate over a list
 * @pos:	the &struct jd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant doesn't differ from jd_list_for_each() any more.
 * We don't do prefetching in either case.
 */
#define __jd_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * jd_list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct jd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define jd_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * jd_list_for_each_safe - iterate over a list safe against removal of list 
entry
 * @pos:	the &struct jd_list_head to use as a loop cursor.
 * @n:		another &struct jd_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define jd_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#define jd_list_for_each_entry_continue(pos, head, member) 		\
	for (pos = jd_list_entry(pos->member.next, typeof(*pos), member);	\
		&pos->member != (head);	\
		pos = jd_list_entry(pos->member.next, typeof(*pos), member))

/**
 * jd_list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the jd_list_struct within the struct.
 */
#define jd_list_for_each_entry(pos, head, member)				\
	for (pos = jd_list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = jd_list_entry(pos->member.next, typeof(*pos), member))

/**
  * list_for_each_entry_reverse - iterate backwards over list of given type.
  * @pos:	the type * to use as a loop cursor.
  * @head:	the head for your list.
  * @member: the name of the list_struct within the struct.
  */
#define jd_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = jd_list_entry((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = jd_list_entry(pos->member.prev, typeof(*pos), member))

/*
 * @brief iterate over list of given type safe against removal of list entry
 * @param pos	The type * to use as a loop cursor.
 * @param n	Another type * to use as temporary storage
 * @param head	The head for your list.
 * @param Member	The name of the jd_list_struct within the struct.
 */
#define jd_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = jd_list_entry((head)->next, typeof(*pos), member),	\
		n = jd_list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = jd_list_entry(n->member.next, typeof(*n), member))

#define jd_list_for_each_entry_continue_reverse(pos, head, member)		\
			for (pos = jd_list_entry(pos->member.prev, typeof(*pos), member);	\
				 &pos->member != (head);	\
				 pos = jd_list_entry(pos->member.prev, typeof(*pos), member))

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
struct jd_hlist_head {
	struct jd_hlist_node *first;
};

struct jd_hlist_node {
	struct jd_hlist_node *next, **pprev;
};

#define JD_HLIST_HEAD_INIT { .first = NULL }
#define JD_HLIST_HEAD(name) struct jd_hlist_head name = {  .first = NULL }
#define JD_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void JD_INIT_HLIST_NODE(struct jd_hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int jd_hlist_unhashed(const struct jd_hlist_node *h)
{
	return !h->pprev;
}

static inline int jd_hlist_empty(const struct jd_hlist_head *h)
{
	return !h->first;
}

static inline void __jd_hlist_del(struct jd_hlist_node *n)
{
	struct jd_hlist_node *next = n->next;
	struct jd_hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void jd_hlist_del(struct jd_hlist_node *n)
{
	__jd_hlist_del(n);
}

static inline void jd_hlist_add_head(struct jd_hlist_node *n, struct 
jd_hlist_head *h)
{
	struct jd_hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void jd_hlist_add_before(struct jd_hlist_node *n,
					struct jd_hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void jd_hlist_add_after(struct jd_hlist_node *n,
					struct jd_hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

#define jd_hlist_entry(ptr, type, member) jd_container_of(ptr,type,member)

#define jd_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define jd_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

/**
 * jd_hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct jd_hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the jd_hlist_node within the struct.
 */
#define jd_hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos &&							 \
		({ tpos = jd_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
		
#define jd_hlist_for_each_entry_constant(tpos, pos, head, member)		 \
	for (pos = (head)->first;					 \
	     pos &&							 \
		({ tpos = container_of_constant(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * jd_hlist_for_each_entry_safe - iterate over list of given type safe 
against removal of list entry
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct jd_hlist_node to use as a loop cursor.
 * @n:		another &struct jd_hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the jd_hlist_node within the struct.
 */
#define jd_hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) && 				 \
		({ tpos = jd_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)


#endif

