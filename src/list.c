/*****************************************************************************
 * list.c is the source file for a general linked list data structure
 * Oliver Hinds <oph@bu.edu> 2004-02-04
 *
 *
 *
 *****************************************************************************/

#include"list.h"
#include"assert.h"

#define LIST_VERSION_C "$Id: list.c,v 1.12 2007/05/22 19:18:06 oph Exp $"

/**
 * creates a new list
 */
list *newList(int type) {
  /* allocate space for the list and initialize elements */
  list *l = (list*) malloc(sizeof(list));

  l->head = NULL;
  l->tail = NULL;
  l->len = 0;
  l->type = type;

  return l;
}

/**
 * clones a list, shallow copy!
 */
list *cloneList(list *l) {
  list *newL = newList(l->type);
  listNode *ln;

  /* insert copies of all the nodes */
  for(ln = getListNode(l,0); ln; ln = (listNode*) ln->next) {
    enqueue(newL,ln->data);
  }

  return newL;
}

/**
 * appends a list to an existing list
 */
void appendList(list *l1, list *l2) {
  /* validate */
  if(l1 == NULL || l2 == NULL) {
    return;
  }

  /* check for empty list 1 */
  if(listSize(l1) == 0) {
    l1->head = l2->head;
    l1->tail = l2->tail;
    l1->len = l2->len;
  }
  else {
    /* make the current tail point to the new head and update the old tail */
    l1->tail->next = (struct listNode*) l2->head;
    l1->tail = l2->tail;
    l1->len += l2->len;
  }

  checkList(l1);
}

/**
 * splits a list, returns a new list
 */
list *splitList(list *l, int n) {
  list *nl;

  /* validate */
  if(l == NULL || n < 1 || listSize(l) <= n) {
    return NULL;
  }

  /* create a new list, update head, length */
  nl = newList(LIST);
  nl->head = getListNode(l,n);
  nl->tail = l->tail;
  nl->len = listSize(l) - n;

  /* truncate the old list */
  l->tail = getListNode(l,n-1);
  l->tail->next = NULL;
  l->len = n;

  checkList(l);

  return nl;
}

/**
 * reverses the ordering of the nodes in a list
 */
list *reverseList(list *l) {
  listNode *nex, *cur = getListNode(l,0), *prev = NULL;

  /* swap the head and tail node */
  nex = l->tail;
  l->tail = l->head;
  l->head = nex;

  /* swap list nodes that are in oppsite positions */
  while(cur != NULL) {
    nex = (listNode*) cur->next;
    cur->next = (struct listNode*) prev;
    prev = cur;
    cur = nex;
  }

  checkList(l);

  return l;
}

/**
 * take every nth list element
 */
void takeEachNthElement(list *l, int n) {
  /* validate */
  if(l == NULL || n < 1 || listSize(l) < 1) {
    return;
  }

  listNode *ln;
  int counter = 0;

  for(ln = getListNode(l,0); ln; ln = (listNode*) ln->next, counter++) {
    if(counter % n) {
      markForDeletion(ln);
    }    
  }
  deleteMarkedNodes(l);
}

/**
 * frees a list
 */
void freeList(list *l) {
  if(l == NULL) return;

  /* freeing the first node forever */
  while(listSize(l) > 0) {
    free(removeListNode(l,0));
  }

  /* free the list */
  free(l);
} // 0x814a908

/**
 * frees a list and the data pointers
 */
void freeListAndData(list *l) {
  listNode *ln;
  if(l == NULL) return;

  /* freeing the first node forever */
  while(listSize(l) > 0) {
    ln = removeListNode(l,0);
    free(ln->data);
    free(ln);
  }

  /* free the list */
  free(l);
}

/**
 * frees a list and the data pointers, watching for possible repetitions
 */
void freeListAndDataRepetitions(list *l) {
  list *freedData;
  listNode *ln;
  if(l == NULL) return;

  freedData = newList(LIST);

  /* freeing the first node forever */
  while(listSize(l) > 0) {
    ln = removeListNode(l,0);

    if(NULL == findInListLN(freedData,ln->data)) {
      enqueue(freedData,ln->data);
      free(ln->data);
    }
    free(ln);
  }

  /* free the list */
  free(l);

  freeList(freedData);
}

/**
 * gets a pointer to the ith node
 */
listNode *getListNode(list *l, int n) {
  int i;
  listNode *a;

  /* make sure the list is big enough */
  if(l == NULL || n < 0 || listSize(l) <= n) {
    return NULL;
  }

  a = l->head;

  /* find the nth node and return a pointer to it */
  for(i=0; i < n; i++) a = (listNode*) a->next;

  checkList(l);

  return a;
}

/**
 * gets the top of a stack
 */
listNode *pop(list *s) {
  listNode *a = s->head;

  /* make sure the heap is not empty */
  if(s == NULL || listSize(s) < 1) {
    return NULL;
  }

  /* return the top element and remove it */
  s->head = (listNode*) a->next;
  s->len--;

  /* special tail case */
  if(a == s->tail) {
    s->tail = NULL;
  }

  return a;
}

/**
 * gets the top node in a heap
 */
listNode *dequeue(list *q) {
  return pop(q);
}

/**
 * gets the top node in a heap
 */
listNode *getHeapTop(list *h) {
  return pop(h);
}

/**
 * deletes a node an returns a pointer to it
 */
listNode *removeListNode(list *l, int n) {
  /* test for in list */
  if(l == NULL || listSize(l) <= n) {
    return NULL;
  }

  listNode *cur = l->head, *prev = NULL;
  int i;

  /* special case - element to be deleted is first */
  if(n == 0) {
    l->head = (listNode *) cur->next;
  }
  else {
    /* find the prev element and remove the current one */
    for(i = 0; i < n; i++) {
      prev = cur;
      cur = (listNode *) cur->next;
    }
    if(prev == NULL) return NULL;
    prev->next = cur->next;
  }

  /* special case - element to be deleted is last */
  if(cur == l->tail) {
    l->tail = prev;
  }

  l->len--;

  checkList(l);

  return cur;
}

/**
 * mark a list node for deletion
 */
void markForDeletion(listNode* ln) {
  /* validate */
  if(ln == NULL) return;

  ln->delete = TRUE;
}

/**
 * delete all list nodes in a list that are marked for deletion
 */
void deleteMarkedNodes(list* l) {
  listNode *cur, *prev;

  /* validate */
  if(l == NULL) return;

  cur = l->head;
  prev = NULL;

  /* search the list for deletable nodes */
  for(cur = getListNode(l,0); cur; cur = (listNode*) cur->next) {
    if(cur->delete == TRUE) {
      if(cur == l->head) { /* head is to be deleted */
	l->head = (listNode*) cur->next;
      }
      else {
	prev->next = cur->next;
      }
      if(cur == l->tail) {
	l->tail = prev;
      }

      if(cur->data != NULL) {
	free(cur->data);
      }
      free(cur);

      l->len--;
      checkList(l);
    }
    else {
      prev = cur;
    }
  }
}

/**
 * sets the ith list node
 */
void setListNode(list *l, int n, void *data) {
  /* test for in list */
  if(l == NULL || listSize(l) <= n) {
    return;
  }

  /* find the node to replace */
  getListNode(l,n)->data = data;

  checkList(l);
}

/**
 * sets the data of a list node
 */
void setListNodeData(listNode *ln, void *data) {
  if(ln == NULL) {
    return;
  }

  ln->data = data;
}

/**
 * insert a node into the list behind a specified node, n should be -1
 * to insert at the head
 */
listNode *insertListNode(list *l, int n, void *data) {
  /* get the node to insert behind */
  listNode *prev, *ln;

  /* test for in list */
  if(l == NULL || listSize(l) <= n) {
    return NULL;
  }

  /* check for head insertion */
  if(n == -1) {
    return push(l,data);
  }

  /* find the node to insert in front of */
  prev = getListNode(l,n);

  if(prev == NULL) return NULL;

  /* allocate space for the new node */
  ln = (listNode*) malloc(sizeof(listNode));
  ln->data = data;
  ln->delete = FALSE;
  ln->next = prev->next;

  prev->next = (struct listNode *) ln;
  l->len++;

  checkList(l);

  return ln;
}

/**
 * push a node into a stack
 */
listNode *push(list *l, void *data) {
  /* allocate space for the node */
  listNode *a = (listNode*) malloc(sizeof(listNode));

  /* test for in list */
  if(l == NULL) {
    return NULL;
  }

  if(a) { /* successful allocation */
    a->data = data;
    a->delete = FALSE;

    /* see if this is the first element added */
    if(listSize(l) == 0) {
      l->head = l->tail = a;
      l->tail->next = NULL;
    }
    else {
      a->next = (struct listNode*) l->head;
      l->head = a;
    }
  }

  l->len++;

  return a;
}


/**
 * enqueue a node
 */
listNode *enqueue(list *l, void *data) {
  /* test for in list */
  if(l == NULL) {
    return NULL;
  }

  /* allocate space for the node */
  listNode *a = (listNode*) malloc(sizeof(listNode));

  if(a) { /* successful allocation */
    a->data = data;
    a->delete = FALSE;

    /* see if this is the first element added */
    if(listSize(l) == 0) {
      l->head = l->tail = a;
      a->next = NULL;
    }
    else {
      l->tail->next = (struct listNode*) a;
      l->tail = a;
      a->next = NULL;
    }
  }

  l->len++;

  checkList(l);

  return a;
}

/**
 * add a heap node
 */
listNode *insertHeapNode(list *h, double value, void *data) {
  /* test for null list */
  if(h == NULL) {
    return NULL;
  }

  /* allocate space for the node */
  listNode *a = (listNode*) malloc(sizeof(listNode));
  listNode *cur, *prev = NULL;
  
  if(a) { /* successful allocation */
    /* assign members */
    a->data = data;
    a->delete = FALSE;
    a->value = value;

    /* see if this is the first element added */
    if(listSize(h) == 0) {
      h->head = h->tail = a;
    }
    else {
      /* find where to insert the node */
      cur = h->head;
      while(cur != NULL && (h->type == MAXHEAP
	    ? cur->value >= a->value : cur->value <= a->value)) {
	prev = cur;
	cur = (listNode*) cur->next;
      }

      /* special head case */
      if(cur == h->head) {
	h->head = (listNode*) a;
      }
      else {
	prev->next = (struct listNode*) a;
      }

      /* special tail case */
      if(prev == h->tail) {
	h->tail = (listNode*) a;
      }

      a->next = (struct listNode*) cur;
    }

    /* set the end to point to null and increase the size */
    h->tail->next = NULL;
    h->len++;
  }

  return a;
}

/**
 * test if a list contains a node
 * returns TRUE if found, FAILURE if not
 */
int listContains(list *l, void *data) {
  return (findInListI(l,data) == VP_FAILURE) ? FALSE : TRUE;
}

/**
 * find a node in a list
 * return the index of the node if found, FAILURE if not found
 */
int findInListI(list *l, void *data) {
  listNode *i;
  int ind;

  /* test for null list */
  if(l == NULL) {
    return VP_FAILURE;
  }

  for(i = getListNode(l,0), ind=0; i; i = (listNode*) i->next, ind++) {
    if(data == i->data) return ind;
  }

  return VP_FAILURE;
}

/**
 * find a node in a list
 * return a pointer to the listNode if found, NULL otherwise
 */
listNode *findInListLN(list *l, void *data) {
  listNode *i;

  /* test for null list */
  if(l == NULL) {
    return NULL;
  }

  for(i = getListNode(l,0); i; i = (listNode*) i->next) {
    if(data == i->data) return i;
  }

  return NULL;
}

/**
 * get the size of the list
 */
int listSize(list *l) {
  /* test for null list */
  if(l == NULL) {
    return 0;
  }

  return l->len;
}

/**
 * prints the contents of one node to an io stream
 */
void printListNode(listNode *a, FILE* str) {
  fprintf(str, "data = %p\n" , a->data);
}

/**
 * dumps the contents of a list to stdout
 */
void dumpList(list* l) {
  listNode *i;

  /* test for null list */
  if(l == NULL) {
    return;
  }

  /* print size */
  fprintf(stdout,"dumping list with %d nodes\n", l->len);
  for(i = getListNode(l,0); i; i = (listNode*) i->next) {
    printListNode(i,stdout);
  }
}

/**
 * checks that the list is valid, prints message if not
 */
void checkList(list *l) {
  if(l->len > 0) {
    if(l->tail->next != NULL) {
      fprintf(stderr,"error: list %p has tail->next %p != NULL\n",
	      l, l->tail->next);
    }
    assert(l->tail->next == NULL);
  }
}

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/list.c,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
