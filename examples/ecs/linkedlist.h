#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "ecs_base.h"

typedef struct ListNode ListNode;

typedef struct ListNode { 
    ListNode * prev;
    ListNode * next;
    EntityId entity;
} ListNode;

typedef struct List {
    ListNode * head;
    ListNode * tail;
    size_t count;
} List;

void list_add(List * list, EntityId entity);
// void list_insert(List * list, int index, EntityId entity);
int list_remove(List * list, EntityId entity);
// int list_removeAt(List * list, int index);
// int list_find(List * list, EntityId entity);

#endif
