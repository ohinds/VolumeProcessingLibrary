/*****************************************************************************
 * list.h is the header file for a list
 * Oliver Hinds <oph@bu.edu> 2004-06-07
 *
 *
 *
 *****************************************************************************/

#ifndef LIST_H
#define LIST_H

#define LIST_VERSION_H "$Id: list.h,v 1.8 2007/05/22 19:18:06 oph Exp $"

#include<stdlib.h>
#include<stdio.h>

#include"libvpTypes.h"
#include"libvpUtil.h"

/* list actions */

/**
 * creates a new list
 */
list *newList(int type);

/**
 * clones a list, shallow copy!
 */
list *cloneList(list *l);

/**
 * appends a list to an existing list
 */
void appendList(list *l1, list *l2);

/**
 * splits a list, returns a new list
 */
list *splitList(list *l, int n);

/**
 * reverses the ordering of the nodes in a list
 */
list *reverseList(list *l);

/**
 * discards every nth list element
 */
void takeEachNthElement(list *l, int n);

/**
 * frees a list
 */
void freeList(list *l);

/**
 * frees a list and the data pointers
 */
void freeListAndData(list *l);

/**
 * frees a list and the data pointers, watching for possible repetitions
 */
void freeListAndDataRepetitions(list *l);

/**
 * gets a pointer to the ith node
 */
listNode *getListNode(list *l, int n);

/**
 * gets the top of a stack
 */
listNode *pop(list *s);

/**
 * gets the front of a queue
 */
listNode *dequeue(list *q);

/**
 * gets the top node in a heap
 */
listNode *getHeapTop(list *h);

/**
 * deletes a node an returns a pointer to it
 */
listNode *removeListNode(list *l, int n);

/**
 * mark a list node for deletion 
 */
void markForDeletion(listNode* ln);

/**
 * delete all list nodes in a list that are marked for deletion
 */
void deleteMarkedNodes(list* l);

/**
 * sets the ith list node
 */
void setListNode(list *l, int n, void *data);

/**
 * sets the data of a list node
 */
void setListNodeData(listNode *ln, void *data);

/**
 * insert a node into the list behind a specified node
 */
listNode *insertListNode(list *l, int n, void *data);

/**
 * push a node into a stack
 */
listNode *push(list *l, void *data);

/**
 * enqueue a node
 */
listNode *enqueue(list *l, void *data);

/**
 * insert a node into the heap
 */
listNode *insertHeapNode(list *h, double value, void *data);

/**
 * find a node in a list
 * return the index of the node if found, FAILURE if not found
 */
int findInListI(list *l, void *data);

/**
 * find a node in a list
 * return a pointer to the listNode if found, NULL, otherwise
 */
listNode *findInListLN(list *l, void *data);

/**
 * test if a list contains a node
 * returns TRUE if found, FALSE if not
 */
int listContains(list *l, void *data);

/**
 * get the size of the list
 */
int listSize(list *l);

/**
 * prints the contents of one node to an io stream
 */
void printListNode(listNode *a, FILE* str);

/**
 * dumps the contents of a list to stdout
 */
void dumpList(list* l);

/**
 * checks that the list is valid, prints message if not
 */
void checkList(list *l);

#endif

/********************************************************************
 * $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/src/list.h,v $
 * Local Variables:
 * mode: C
 * fill-column: 76
 * comment-column: 0
 * End:
 ********************************************************************/
