#ifndef __STACK_H
#define __STACK_H
typedef struct node
{
    int std_id;
    struct node* next;
}stack;
int top(stack*);
stack* push(stack*, int);
stack* pop(stack*);
stack* deleteID(stack*, int);
#endif