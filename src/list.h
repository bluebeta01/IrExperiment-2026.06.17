#ifndef LIST_H
#define LIST_H
//Used when mallocing a new vector
#include <stdlib.h>
#include <stddef.h>

typedef struct
{
	void *data;
	int size;
	int capacity;
} Vector;

void vector_push_back(Vector *vector, void *data, int data_size);

#define vec_at(type, vector, index) ((type*)(vector)->data)[index]
#define vec_last(type, vector) ((type*)(vector)->data)[(vector)->size - 1]
#define vec_new(type, initial_capacity) (Vector){ .data = malloc(sizeof(type) * initial_capacity), .capacity = initial_capacity }
#define vec_push(type, vector, value) vector_push_back(vector, value, sizeof(type))
#define vec_free(vector) free((vector)->data);

struct ListNode
{
	struct ListNode *next;
	struct ListNode *prev;
};
typedef struct ListNode ListNode;

typedef struct LinkedList
{
	ListNode *first;
	ListNode *last;
} LinkedList;

#define list_value(type, member, nodeptr)  ((type*)((char*)(nodeptr) - offsetof(type, member)))
#define list_append(list, nodeptr) \
	if ((list)->first == NULL) { (list)->first = nodeptr; (list)->last = nodeptr; } \
	else { (list)->last->next = nodeptr; (nodeptr)->prev = (list)->last; (list)->last = nodeptr; }


#endif