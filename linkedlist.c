#include "linkedlist.h"

#include <stdlib.h>

struct llnode {
    void *val;
    struct llnode *next;
};
typedef struct llnode* LLnode;

struct linkedlist {
    LLnode head;
    LLnode tail;
};

struct ll_iterator {
    LLnode curr;
};


LList makeLL() {
    LList list = (LList) malloc(sizeof(struct linkedlist));
    list->head = NULL;
    return list;
}

LList cloneLL(LList l) {
    LList clone = makeLL();
    
    if (l) {
        LLnode curr = l->head;
        while (curr) {
            appendToLL(clone, curr->val);
            curr = curr->next;
        }
    }

    return clone;

}


int sizeOfLL(LList l) {
    int len = 0;
    LLnode curr;

    if (!l) return 0; /* Empty case */

    curr = l->head;
    while (curr) {
        curr = curr->next;
        len++;
    }

    return len;
}

int isEmptyLL(LList l) {
    return !l || !(l->head);
}

void* getFromLL(LList l, int idx) {
    LLnode curr;
    
    /* Empty and below bounds cases */
    if (!l || idx < 0)
        return NULL;
    
    curr = l->head;
    while (curr) {
        if (idx == 0)
            return curr->val;

        curr = curr->next;
        idx--;
    }

    return NULL;
}

int indexOfLL(LList l, void *val) {
    
    LLnode curr;
    int i;

    if (!l)
        return -1;

    curr = l->head;
    for (i = 0; curr; i++, curr = curr->next) {
        if (curr->val == val)
            return i;
    }

    return -1;
}

void appendToLL(LList l, void *val) {
    if (!l) return;
    
    if (l->head == NULL) {
        /* List is empty */
        l->head = (LLnode) malloc(sizeof(struct llnode));
        l->head->val = val;
        l->head->next = NULL;
        l->tail = l->head;
    } else {
        /* List is non-empty */
        l->tail->next = (LLnode) malloc(sizeof(struct llnode));
        l->tail = l->tail->next;

        l->tail->val = val;
        l->tail->next = NULL;
    }
}

void addToLL(LList l, int idx, void *val) {
    LLnode node;
    
    /* Empty cases */
    if (!l) return;
    else if (l->head == NULL) {
        appendToLL(l, val);
        return;
    }

    node = (LLnode) malloc(sizeof(struct llnode));
    node->val = val;

    if (idx <= 0) {
        /* Front of list case */
        node->next = l->head;
        l->head = node;
    } else {
        LLnode curr = l->head;
        while (curr->next && idx > 1) {
            curr = curr->next; 
            idx--;
        }
        
        /* Set new tail if necessary */
        if (curr->next == NULL)
            l->tail = node;

        node->next = curr->next;
        curr->next = node;
    }
}

void* remFromLL(LList l, int idx) {
    if (!l || l->head == NULL)
        /* Empty list */
        return NULL;
    else if (l->head->next == NULL) {
        /* Singleton list */
        void *res = l->head->val;

        l->head->val = NULL;
        free(l->head);
        l->head = l->tail = NULL;

        return res;
    } else {
        /* Plural items */
        LLnode curr = l->head;
        LLnode rem;

        void *res = NULL;

        while (idx > 1 && curr->next->next) {
            curr = curr->next;
            idx--;
        }
        
        if (idx > 0) {
            /* Item is at front of list */
            rem = curr->next;
            res = curr->next->val;

            curr->next = curr->next->next;
        } else {
            rem = curr;
            res = curr->val;
            l->head = rem->next;

            if (rem->next == NULL)
                l->tail = curr;
        }

        rem->val = NULL;
        rem->next = NULL;
        free(rem);

        return res;
    }
    
}

LLiter makeLLiter(LList list) {
    LLiter iter = (LLiter) malloc(sizeof(struct ll_iterator));

    iter->curr = list ? list->head : NULL;

    return iter;

}

int iterHasNextLL(LLiter iter) {
    return iter && iter->curr != NULL;
}

void *iterNextLL(LLiter iter) {
    if (iter && iter->curr) {
        void *res = iter->curr->val;
        iter->curr = iter->curr->next;
        return res;
    } else
        return NULL;
}

void disposeIterLL(LLiter iter) {
    if (iter) {
        iter->curr = NULL;
        free(iter);
    }
}


