#include <types.h>
#include <lib.h>
#include <linkedlist.h>

void print_list(struct node * head) {
    struct node * current = head;

    while (current != NULL) {
        kprintf("%d\n", current->val);
        current = current->next;
    }
}

void push_end(struct node ** head, int val) {
    if(*head == NULL) {
        push_begin(head, val);
        return;
    }
    
    struct node * current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = kmalloc(sizeof(struct node));
    current->next->val = val;
    current->next->next = NULL;
}

void push_begin(struct node ** head, int val) {
    struct node * new_node;
    new_node = kmalloc(sizeof(struct node));

    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

int exists(struct node * head, int val) {
    struct node * current = head;

    while (current != NULL) {
        if(current->val == val) return 1;
        current = current->next;
    }
    
    return 0;
}

struct node * swapexists(struct node * head, int val) {
    struct node * current = head;

    while (current != NULL) {
        if(current->val >> 16 == val >> 16) return current;
        current = current->next;
    }
    
    return 0;
}

int remove_val(struct node ** head, int val) {
    struct node * current = *head;
    // If the first is in linked list
    if(current->val == val) {
        *head = current->next;
        kfree(current);
        return 1;
    }
    
    while (current->next != NULL) {
        if(current->next->val == val) {
            struct node *next = current->next;
            current->next = next->next;
            kfree(next);
            return 1;
        }
        current = current->next;
    }
    
    return 0;
}

int pop(struct node ** head) {
    int retval = -1;
    struct node * next_node = 0;

    if (*head == 0) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    kfree(*head);
    *head = next_node;

    return retval;
}

int ll_destroy(struct node** head) {
    while(pop(head) != -1);
}

int remove_last(struct node * head) {
    int retval = 0;
    /* if there is only one item in the list, remove it */
    if (head->next == 0) {
        retval = head->val;
        kfree(head);
        return retval;
    }

    /* get to the second to last node in the list */
    struct node * current = head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->val;
    kfree(current->next);
    current->next = NULL;
    return retval;

}

int remove_by_index(struct node ** head, int n) {
    int i = 0;
    int retval = -1;
    struct node * current = *head;
    struct node * temp_node = NULL;

    if (n == 0) {
        return pop(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->val;
    current->next = temp_node->next;
    kfree(temp_node);

    return retval;
}

int ll_count(struct node ** head) {
    struct node * current = head;
    int count = 0;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    return count;
}
