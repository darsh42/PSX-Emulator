#include "../../include/linkedlist.h"

void ll_prepend(ll_node_t **head, uint32_t value);
void ll_append(ll_node_t **head, uint32_t value);
void ll_insert(ll_node_t **head, uint32_t value, int index);
void ll_remove_first(ll_node_t **head);
void ll_remove_last(ll_node_t **head);
void ll_remove_index(ll_node_t **head, int index);
void ll_remove_find(ll_node_t **head, uint32_t value);
int  ll_find(ll_node_t *head, uint32_t value);
void ll_print(ll_node_t *head);
void ll_destroy(ll_node_t *head);

void ll_prepend(ll_node_t **head, uint32_t value) {
    ll_node_t *new = malloc(sizeof(ll_node_t));
    
    new->value = value;
    new->next  = *head;
    *head = new;
}

void ll_append(ll_node_t **head, uint32_t value) {
    if (*head == NULL) {
        return;
    }

    ll_node_t *current = *head;
    ll_node_t *new = malloc(sizeof(ll_node_t));

    new->value = value;
    new->next  = NULL;
    
    for (; current->next; current = current->next)
        ;
    current->next = new;
}

void ll_insert(ll_node_t **head, uint32_t value, int index) {
    if (*head == NULL) {
        return;
    }

    if (!index) {
        ll_prepend(head, value);
        return;
    }
    
    int i = 0;
    ll_node_t *new, *current = *head;
    for(;current->next && i != index - 1; current = current->next, i++)
        ;
    new = malloc(sizeof(ll_node_t));
    new->value = value;
    new->next  = current->next;
    current->next = new;
}

void ll_remove_first(ll_node_t **head) {
    ll_node_t *remove = *head;
    *head = remove->next;
    free(remove);
}

void ll_remove_last(ll_node_t **head) {
    if (head == NULL) {
        return;
    }
        
    int i;
    ll_node_t *previous, *current = *head;
    for (i = 0; current->next; previous = current, current = current->next, i++)
        ;

    if (i == 0) {
        ll_remove_first(head);
        return;
    }
    previous->next = NULL;
    free(current);
}

void ll_remove_index(ll_node_t **head, int index) {
    if (*head == NULL) {
        return;
    }

    if (!index) {
        ll_remove_first(head);
        return;
    }
    
    int i = 0;
    ll_node_t *remove, *current = *head;
    for (; current->next && i < index - 1; current = current->next, i++)
        ;
    remove        = current->next;
    current->next = remove->next;
    free(remove);
}

void ll_remove_find(ll_node_t **head, uint32_t value) {
    if (head == NULL) {
        return;
    }
        
    int index;

    if ((index = ll_find(*head, value)) < 0) {
        return;
    }

    ll_remove_index(head, index);
}

int ll_find(ll_node_t *head, uint32_t value) {
    if (head == NULL) {
        return -1;
    }
    
    bool found = false; int index = 0;
    for (; head && !(found = (head->value == value)); head = head->next, index++)
        ;
    return (found) ? index: -1;
}

void ll_destroy(ll_node_t *head) {
    if (head == NULL) {
        return;
    }

    if (!head->next) {
        free(head);
        return;
    }

    ll_node_t *previous = head;
    for (head = head->next; head->next; previous = head, head = head->next)
        free(previous);
    free(previous);
}

void ll_print(ll_node_t *head) {
    for(; head; head = head->next)
        printf("%x ", head->value);
    printf("\n");
}

// int main(void) {
//     ll_node_t *list = NULL;
//     
//     // expected: 1 2 3
//     ll_prepend(&list, 0XBFC00000);
//     ll_append(&list, 3);
//     ll_insert(&list, 2, 1);
//     ll_print(list);
//     
//     // expected: 1
//     printf("%d\n", ll_find(list, 4));
// 
//     // expected: 
//     ll_remove_find(&list, 2);
//     ll_remove_first(&list);
//     ll_remove_last(list);
//     ll_remove_index(&list, 0);
//     ll_print(list);
// 
//     // expected: list is inaccessable
//     ll_destroy(list);
//     return 0;
// }
