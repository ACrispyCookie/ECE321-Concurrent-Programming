#include "list.h"
#include <stddef.h>
#include <stdlib.h>

/*
    Allocates memory for a new node
    and stores the given data to the corresponding
    field.
*/
static node_t *create_node(void *data);

list_t *list_init() {
    list_t *new_list = (list_t *) malloc(sizeof(list_t));
    if (new_list == NULL)
        return NULL;
    
    node_t *node_to_add = create_node(NULL);
    if (node_to_add == NULL) {
        free(new_list);
        return NULL;
    }

    new_list->size = 0;
    new_list->head = node_to_add;
    node_to_add->next = node_to_add;
    return new_list;
}

int list_add(list_t *list, void *data) {
    if (list == NULL)
        return LIST_ERROR;
    if (list_find(list, data, NULL) != NULL)
        return LIST_ALREADY;
    
    node_t *node_to_add = create_node(data);
    if (node_to_add == NULL)
        return LIST_ERROR;

    node_to_add->next = list->head->next;
    list->head->next = node_to_add;
    list->size++;
    return LIST_SUCCESS;
}

int list_remove(list_t *list, void *data) {
    if (list == NULL)
        return LIST_ERROR;

    node_t *previous_node;
    node_t *found_node = list_find(list, data, &previous_node);
    if (found_node == NULL)
        return LIST_ALREADY;
    
    previous_node->next = found_node->next;
    free(found_node);
    list->size--;
    return LIST_SUCCESS;
}

node_t *list_find(list_t *list, void *data, node_t **previous_return) {
    if (list == NULL)
        return NULL;
    
    node_t *current;
    node_t *previous = list->head;
    list->head->data = data;
    for (current = list->head->next; current->data != data; current = current->next)
        previous = current;
    
    if (previous_return != NULL)
        *previous_return = previous;
    return current == list->head ? NULL : current;
}

void list_destroy(list_t *list) {
    if (list == NULL)
        return;
    node_t *curr_node, *next_node;

    for (curr_node = list->head->next; curr_node != list->head; curr_node = next_node) {
        next_node = curr_node->next;
        free(curr_node);
    }
    free(list->head);
    free(list);
}

static node_t *create_node(void *data) {
    node_t *new_node = (node_t *) malloc(sizeof(node_t));
    if (new_node == NULL)
        return NULL;
    
    new_node->data = data;
    return new_node;
}