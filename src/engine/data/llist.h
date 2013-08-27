/*
 * llist.h
 *
 *	General purpose Linked List implementation in C
 *
 *  Created on: Apr 26, 2013
 *      Author: Mathias H. Wilhelmsen
 */

#ifndef LLIST_H_
#define LLIST_H_

typedef void* LList;

LList llist_create(void); /* lager en ny lenket liste og returnerer et id-nummer til denne */

int llist_set_remove_callback(LList list, void (*remove_callback)(void *)); /* funksjonstilbakekall n�r peker fjernes */

int llist_add(LList list, void *p); /* legger til angitt peker i list */
int llist_remove(LList list, void *p); /* fjerner angitt peker fra list */

int llist_contains(LList id, void *p);

int llist_size(LList list);    /* returnerer antall pekere i listen */

void* llist_first(LList list); /* returnerer f�rste peker i listen */
void* llist_last(LList list);  /* returnerer siste peker i listen */
void* llist_at_index(LList list, int index); /* returnerer pekeren  p� angitt index */

void llist_iterate_func(LList list, void (*func)(void *)); /* kaller p� func for hver peker */

int llist_begin_loop(LList list);   /* push-er ny iterasjons-peker */
int llist_hasnext(LList list); /* returnerer 1 om n�v�rende iterasjonspeker ikke er NULL */
void* llist_next(LList list);  /* returnerer neste peker i list, etter siste peker returneres null */
int llist_end_loop(LList list);     /* pop-er n�v�rende iterasjons-peker */

void llist_clear(LList list);  /* fjerner alle pekere i listen */

int llist_destroy(LList list);    /* frigj�r listen fra minnet */

void llist_free_nodes(void);       /* frigj�r alle ubrukte noder fra minnet */

#endif /* LLIST_H_ */
