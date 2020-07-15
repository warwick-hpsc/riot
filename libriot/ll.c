#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

// set this to 1 if things seem to be going wrong?!
int debug = 0;

ll_list *ll_create() {
	// create a new linked list and return it.
	ll_list *newll = malloc(sizeof(ll_list));
	newll->head = NULL;
	newll->tail = NULL;
	return newll;
}

void ll_addtohead(ll_list *p, char *data) {
	// set up element
	ll_element *n = (ll_element *) malloc(sizeof(ll_element));
	n->data = malloc((strlen(data) + 1) * sizeof(char));
	strcpy(n->data, data);
	n->next = NULL;

	// failed to allocate the memory!
	if (n == NULL) return;

	// if the list is empty... set up both!
	if ((p->head == NULL) && (p->tail == NULL)) { 
		if (debug == 1) printf("List empty, adding element as head and tail\n");
		p->head = n;
		p->tail = n;
	} else {
		// set element to head and next to old head
		n->next = p->head;
		p->head = n;
	}
}

void ll_addtotail(ll_list *p, char *data) {
	// set up element
	ll_element *n = (ll_element *) malloc(sizeof(ll_element));
	n->data = malloc((strlen(data) + 1) * sizeof(char));
	strcpy(n->data, data);
	n->next = NULL;

	// failed to allocate memory!
	if (n == NULL) return;
	
	// if list is empty set up both
	if ((p->head == NULL) && (p->tail == NULL)) { 
		if (debug == 1) printf("List empty, adding element as head and tail\n");
		p->head = n;
		p->tail = n;
	} else {
		// set old tail's next to be new tail, set tail to n
		p->tail->next = n;
		p->tail = n;
	}
}

void ll_removefromhead(ll_list *p) {
	// if list isn't empty
	if (p->head != NULL) {
		// store element to free it
		ll_element *n = p->head;

		// if next is empty, make sure we reset both head and tail (empty list)
		if (p->head->next == NULL) {
			p->head = NULL;
			p->tail = NULL;
		} else {
			// otherwise set head to be the next
			p->head = p->head->next;
		}
		// free data and pointer
		free(n->data);
		free(n);
	}
}

char *ll_gethead(ll_list *p) {
	// if head isn't null, return data otherwise return NULL.
	if (p->head != NULL)
		return p->head->data;
	else
		return NULL;
}

void ll_print(ll_list *p) {
	ll_element *tmp = p->head;

	// if tmp is NULL, list is empty.
	if (tmp == NULL) {
		if (debug == 1) printf("NULL!\n");
		return;
	}

	// iterate over list printing out the data at each stage.
	while (tmp != NULL) {
		printf("%s\n", tmp->data);
		tmp = tmp->next;
	}
}

void ll_clear(ll_list *p) {
	// if head is not null iterate over it freeing up pointers until its empty.
	while (p->head != NULL) {
		ll_element *n = p->head;
		
		p->head = p->head->next;
		
		free(n->data);
		free(n);
	}

	// set head and tail null (empty list)
	p->head = NULL;
	p->tail = NULL;
}

void ll_dumptofile(ll_list *p, char *filename) {
	FILE *fp;
	if (filename != NULL) {
		fp = fopen(filename, "w+");
		
		if (fp == NULL) return;

		// if head is not null iterate over it freeing up pointers until its empty.
		while (p->head != NULL) {
			ll_element *n = p->head;

			p->head = p->head->next;

			// before freeing pointers, print out data
			//fprintf(fp, "%s\n", n->data);

			fputs(n->data, fp);
			fputs("\n", fp);
			fflush(fp);

			free(n->data);
			free(n);
		}

		// set head and tail null (empty list)
		p->head = NULL;
		p->tail = NULL;

		fclose(fp);
	}
}

