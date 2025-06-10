#include "linked_list.h"

// Function pointers for custom memory allocation
static void *(*malloc_fptr)(size_t) = NULL;
static void (*free_fptr)(void *) = NULL;

struct linked_list *linked_list_create(void)
{
    struct linked_list *ll = malloc_fptr(sizeof *ll);
    if (!ll)
        return NULL;
    ll->head = NULL;
    ll->size = 0;
    return ll;
}

bool linked_list_delete(struct linked_list *ll)
{
    if (!ll)
        return false;

    struct node *current = ll->head;
    while (current)
    {
        struct node *next = current->next;
        free_fptr(current);
        current = next;
    }

    free_fptr(ll);
    return true;
}

size_t linked_list_size(struct linked_list *ll)
{
    if (!ll)
        return SIZE_MAX;

    return ll->size;
}

bool linked_list_insert_end(struct linked_list *ll, unsigned int data)
{
    if (!ll)
        return false;

    struct node *new_node = malloc_fptr(sizeof *new_node);
    if (!new_node)
        return false;

    new_node->data = data;
    new_node->next = NULL;

    if (!ll->head)
    {
        ll->head = new_node;
        return true;
    }

    struct node *tail = ll->head;
    while (tail->next)
        tail = tail->next;
    tail->next = new_node;

    ll->size++;
    return true;
}

bool linked_list_insert_front(struct linked_list *ll, unsigned int data)
{
    if (!ll)
        return false;

    struct node *new_node = malloc_fptr(sizeof *new_node);
    if (!new_node)
        return false;

    new_node->data = data;
    new_node->next = ll->head;
    ll->head = new_node;

    ll->size++;
    return true;
}

bool linked_list_insert(struct linked_list *ll, size_t index, unsigned int data)
{
    if (!ll)
        return false;

    if (!index)
        return linked_list_insert_front(ll, data);

    struct node *current = ll->head;
    while (index-- > 1 && current)
        current = current->next;

    if (!current)
        return false;

    struct node *new_node = malloc_fptr(sizeof *new_node);
    if (!new_node)
        return false;

    new_node->data = data;
    new_node->next = current->next;
    current->next = new_node;

    ll->size++;
    return true;
}

size_t linked_list_find(struct linked_list *ll, unsigned int data)
{
    if (!ll)
        return SIZE_MAX;

    size_t index = 0;
    struct node *current = ll->head;
    while (current)
    {
        if (current->data == data)
            return index;
        current = current->next;
        index++;
    }

    return SIZE_MAX;
}

bool linked_list_remove(struct linked_list *ll, size_t index)
{
    if (!ll || !ll->head)
        return false;

    if (!index)
    {
        struct node *to_delete = ll->head;
        ll->head = to_delete->next;
        free_fptr(to_delete);
        return true;
    }

    struct node *current = ll->head;
    while (index-- > 1 && current)
        current = current->next;

    if (!current || !current->next)
        return false;

    struct node *to_delete = current->next;
    current->next = to_delete->next;
    free_fptr(to_delete);

    ll->size--;
    return true;
}

struct iterator *linked_list_create_iterator(struct linked_list *ll, size_t index)
{
    if (!ll)
        return NULL;

    struct iterator *it = malloc_fptr(sizeof *it);
    if (!it)
        return NULL;

    it->ll = ll;
    it->current_node = ll->head;
    it->current_index = 0;

    while (it->current_node && it->current_index < index)
    {
        it->current_node = it->current_node->next;
        it->current_index++;
    }

    if (it->current_index < index || !it->current_node)
    {
        free_fptr(it);
        return NULL;
    }

    it->data = it->current_node->data;
    return it;
}

bool linked_list_delete_iterator(struct iterator *iter)
{
    if (!iter)
        return false;
    free_fptr(iter);
    return true;
}

bool linked_list_iterate(struct iterator *iter)
{
    if (!iter || !iter->current_node)
        return false;

    iter->current_node = iter->current_node->next;
    iter->current_index++;

    if (iter->current_node)
    {
        iter->data = iter->current_node->data;
        return true;
    }

    return false;
}

bool linked_list_register_malloc(void *(*malloc)(size_t))
{
    if (!malloc)
        return false;
    malloc_fptr = malloc;
    return true;
}

bool linked_list_register_free(void (*free)(void *))
{
    if (!free)
        return false;
    free_fptr = free;
    return true;
}
