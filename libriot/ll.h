typedef struct ll_elem {
	char *data;
	struct ll_elem *next;
} ll_element;

typedef struct ll_elems {
	ll_element *head;
	ll_element *tail;
} ll_list;

ll_list *ll_create();
void ll_addtohead(ll_list *p, char *data);
void ll_addtotail(ll_list *p, char *data);
void ll_removefromhead(ll_list *p);
char *ll_gethead(ll_list *p);
void ll_print(ll_list *p);
void ll_clear(ll_list *p);
void ll_dumptofile(ll_list *p, char *filename);

