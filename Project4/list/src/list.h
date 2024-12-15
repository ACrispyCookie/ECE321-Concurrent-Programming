#ifndef _COROUTINE_LIST_H_
#define _COROUTINE_LIST_H_
#define LIST_SUCCESS 1
#define LIST_ALREADY 0 
#define LIST_ERROR -1

    /*
        Struct describing a node
        of a circular list 
    */
    typedef struct node {
        void *data;
        struct node *next; 
    } node_t;

    /*
        Struct describing a circular list 
    */
    typedef struct list {
        unsigned int size;
        node_t *head; 
    } list_t;

    /*
        Initializes a coroutine list and
        returns a list_t describing the list.
    */
    list_t *list_init();

    /*
        Adds the given struct to the end of the list.

        Parameters:
        list_t *list - The list to add the new node.
        void *data - The data of the new node.

        Returns:
        1 if the new node was added successfully.
        0 if the new node already existed in the list.
        -1 if an error occurred.
    */
    int list_add(list_t *list, void *data);

    /*
        Removes the given struct from the list.

        Parameters:
        list_t *list - The list to remove the node from.
        void *data - The data of the node.

        Returns:
        1 if the node was removed successfully.
        0 if the node didn't exist in the list.
        -1 if an error occurred.
    */
    int list_remove(list_t *list, void *data);

    /*
        Removes the node in the given index from the list.

        Parameters:
        list_t *list - The list to remove the node from.
        unsigned int index - The index of the node to remove.

        Returns:
        a pointer to the node of the list found or
        NULL if the node was not found or if an error occurred.
    */
    void *list_remove_index(list_t *list, unsigned int index);

    /*
        Find the given struct in the list.

        Parameters:
        list_t *list - The list to search in.
        void *data - The data to find.
        node_t **previous - Optional parameter to store the previous node of 
                            the one we are looking for.

        Returns:
        a pointer to the node of the list containing the given data or
        NULL if the data was not found or if an error occurred.
    */
    node_t *list_find(list_t *list, void *data, node_t **previous);

    /*
        Destroys the given list freeing
        any related memory.
    */
    void list_destroy(list_t *list);

#endif