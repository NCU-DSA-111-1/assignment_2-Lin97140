#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#define FALSE 0
#define TRUE 1

int isempty(const node *stackptr)
{
    if (stackptr == NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void push(node **stackptr, char name, int chess, int original_x, int original_y, int position_x, int position_y, int die)
{
    node *new_node = malloc(sizeof(node));
    new_node->name = name;
    new_node->chess = chess;
    new_node->original_x = original_x;
    new_node->original_y = original_y;
    new_node->position_x = position_x;
    new_node->position_y = position_y;
    new_node->die = die;
    new_node->previous = NULL;
    new_node->next = *stackptr;
    if (*stackptr != NULL)
    {
        (*stackptr)->previous = new_node;
    }
    *stackptr = new_node;
}

void pop(node **stackptr, int *chess, int *position_x, int *position_y, int *original_x, int *original_y, int *die, char *name)
{
    node *temp = *stackptr;
    if (!isempty(*stackptr))
    {
        *stackptr = (*stackptr)->next;
        *chess = temp->chess;
        *position_x = temp->position_x;
        *position_y = temp->position_y;
        *original_x = temp->original_x;
        *original_y = temp->original_y;
        *die = temp->die;
        *name = temp->name;
        free(temp);
    }
}

void show_stack(node *stackptr)
{
    node *temp = stackptr;
    while (temp != NULL)
    {
        printf("player: %c chess = %d o_x = %d o_y = %d p_x = %d p_y = %d die = %d p = %p n = %p\n", temp->name, temp->chess, temp->original_x, temp->original_y, temp->position_x, temp->position_y, temp->die, temp->previous, temp->next);
        temp = temp->next;
    }
}

void free_stack(node **stackptr)
{
    node *temp = NULL;
    while (*stackptr != NULL)
    {
        temp = *stackptr;
        *stackptr = (*stackptr)->next;
        free(temp);
    }
}