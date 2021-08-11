#include "stack.h"
#include<stdlib.h>
stack* push(stack* s, int id)
{
    if(s==NULL)
    {
        s = (stack*)malloc(sizeof(stack));
        s->std_id = id;
        s->next = NULL;
        return s;
    }
    stack* new = (stack*)malloc(sizeof(stack));
    new->std_id = id;
    new->next = s;
    return new;
}
stack* pop(stack* s)
{
    stack* temp = s;
    if(temp==NULL)
        return NULL;
    s=s->next;
    free(temp);
    temp = NULL;
    return s;
}
int top(stack* s)
{
    if(s==NULL)
        return -1;
    return s->std_id;
}
stack* deleteID(stack* s, int id)
{
    stack* temp = s;
    if(temp==NULL)
        return NULL;
    if(temp->std_id==id)
    {
        s = s->next;
        free(temp);
        return s;
    }
    stack* next = temp->next;
    while(next!=NULL && next->std_id!=id)
        temp = next, next = next->next;
    if(next!=NULL)
    {
        temp->next = next->next;
        free(next);
        return s;
    }
    return s;
}