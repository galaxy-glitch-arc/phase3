#ifndef LIST_H
#define LIST_H

typedef struct _list {
    int *data;
    int length;
    int capacity;
} list;

void list_init(list *l);
int list_at(list *l, int index);
void list_push_front(list *l, int value);
int list_pop_front(list *l);
void list_push_back(list *l, int value);
int list_pop_back(list *l);
void list_delete(list *l, int index);
int list_exists(list *l, int index);
void list_free(list *l);
void list_clear(list *l);
void list_print(char* message, list *l);
void list_set_push(list *l, int value);
void list_remove(list *l, int value, int all);
int list_is_first(list *l, int value);


#endif /* LIST_H */
