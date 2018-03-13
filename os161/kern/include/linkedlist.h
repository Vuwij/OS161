#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

struct node {
    int val;
    struct node * next;
};

void print_list(struct node * head);

void push_end(struct node * head, int val);

void push_begin(struct node ** head, int val);

int pop(struct node ** head);

int remove_last(struct node * head);

int remove_by_index(struct node ** head, int n);

#endif /* LINKEDLIST_H */

