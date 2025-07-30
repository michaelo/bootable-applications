#include "linkedlist.h"
#include <shared/memory.h>

void list_add(List * list, EntityId entity) 
{
    ListNode * node = (ListNode *)malloc(sizeof(ListNode));
    *node = (ListNode)
    {
        .prev = NULL,
        .next = NULL,
        .entity = entity
    };

    if (list->count == 0){
        list->head = list->tail = node;
    }
    else {
        ListNode * currentTail = list->tail;
        currentTail->next = node;
        node->prev = currentTail;
        list->tail = node;
    }
    list->count++;
}

// void list_insert(List * list, int index, EntityId entity) 
// {

// }

int list_remove(List * list, EntityId entity) 
{
    ListNode * current = list->head;
    while (current != NULL)
    {
        if (current->entity == entity)
        {
            ListNode * next = current->next;
            ListNode * prev = current->prev;
            if (prev) prev->next = next;
            if (next) next->prev = prev;
            if (current == list->head) list->head = next;
            if (current == list->tail) list->tail = prev;
            free(current);
            list->count--;
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// int list_removeAt(List * list, int index) 
// {
//     return -1;
// }

// int list_find(List * list, EntityId entity) 
// {
//     return -1;
// }
