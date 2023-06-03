/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <stdlib.h>

#include "LinkedList.h"
#include "LinkedList_priv.h"


///////////////////////////////////////////////////////////////////////////////
// LinkedList implementation.

LinkedList* LinkedList_Allocate(void) {

  // TODO: allocate the LinkedList struct and initialize the newly allocated record structure.
  LinkedList* res = malloc(sizeof(LinkedList)) ;
  if (res != NULL) {
    res->num_elements = 0 ;       // the initial num of elements in the Linked List is 0 (no node)
    res->head = NULL ;
    res->tail = NULL ;
  }
  return res;  // you may want to change this
}

void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function) {

  // TODO: sweep through the list and free all of the nodes' payloads
  // (using the payload_free_function supplied as an argument) and
  // the nodes themselves. 
  // free the LinkedListNode
  LinkedListNode *node = list->head ;       // pointer points to head node
  LinkedListNode *next_node = NULL ;

  while (node != NULL) {                    // loop until exceed the linked list
    next_node = node->next ;                // store the address of next node
    payload_free_function(node->payload) ;  // free the current node's payload
    free(node) ;                            // free the current node
    node = next_node ;                      // move to the next node
  }
  
  // free the LinkedList
  free(list);                               
}

int LinkedList_NumElements(LinkedList *list) {
  return list->num_elements;
}

void LinkedList_Push(LinkedList *list, LLPayload_t payload) {
  // TODO: implement LinkedList_Push
  LinkedListNode *new_node = malloc(sizeof(LinkedListNode)) ;
  if (!new_node) {
    return ;
  }
  new_node->payload = payload ;
  new_node->next = list->head ;              // add the new node to the head of the linked list
  new_node->prev = NULL ;
  if (list->head) {
    LinkedListNode *prev_node = list->head ; // record the previous head node
    list->head = new_node ;                  // head of the linked list change to the new node
    prev_node->prev = new_node ;            
  } else {                                   // empty list
    list->head = new_node ;
    list->tail = new_node ;
  }
  list->num_elements += 1 ;                 
}

bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr) {
  // TODO: implement LinkedList_Pop.  Make sure you test for
  // and empty list and fail.  If the list is non-empty, there
  // are two cases to consider: (a) a list with a single element in it
  // and (b) the general case of a list with >=2 elements in it.
  // Be sure to call free() to deallocate the memory that was
  // previously allocated by LinkedList_Push().
  if (list->num_elements == 0) {              // if the list is empty, return false 
    return false ;
  }

  LinkedListNode *first_node = list->head ;   
  *payload_ptr = first_node->payload ;
  if (list->num_elements == 1) {              // if the list has only 1 element
    free(first_node) ;                        // free the heap memory of node
    list->head = NULL ;                       // no element in the list
    list->tail = NULL ;
  } else {
    LinkedListNode *second_node = first_node->next ;  
    free(first_node) ; 
    list->head = second_node ;                // the head of the linked list becomes the second node
    second_node->prev = NULL ;
  }
  list->num_elements -= 1 ;
  return true;  // you may need to change this return value
}

void LinkedList_Append(LinkedList *list, LLPayload_t payload) {
  // TODO: implement LinkedList_Append.  It's kind of like
  // LinkedList_Push, but obviously you need to add to the end
  // instead of the beginning.
  LinkedListNode *new_node = malloc(sizeof(LinkedListNode)) ;
  if (!new_node) {
    return ;
  }
  new_node->payload = payload ;
  new_node->next = NULL ;                     // add the new node to the tail of the linked list
  new_node->prev = list->tail ;
  if (list->tail) {
    LinkedListNode *prev_node = list->tail ; // record the previous tail node
    list->tail = new_node ;                  // tail of the linked list change to the new node
    prev_node->next = new_node ;            
  } else {                                   // empty list
    list->head = new_node ;
    list->tail = new_node ;
  }
  list->num_elements += 1 ; 
}

bool LinkedList_Slice(LinkedList *list, LLPayload_t *payload_ptr) {
  // TODO: implement LinkedList_Slice.
  if (list->num_elements == 0) {              // if the list is empty, return false 
    return false ;
  }

  LinkedListNode *last_node = list->tail ;   
  *payload_ptr = last_node->payload ;
  if (list->num_elements == 1) {              // if the list has only 1 element
    free(last_node) ;                         // free the heap memory of node
    list->head = NULL ;                       // no element in the list
    list->tail = NULL ;
  } else {
    LinkedListNode *second_last_node = last_node->prev ;  
    free(last_node) ; 
    list->tail = second_last_node ;           // the tail of the linked list becomes the second last node
    second_last_node->next = NULL ;
  }
  list->num_elements -= 1 ;
  return true;  // you may need to change this return value
}


// this function is completed for you
void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function) {
  if (list->num_elements < 2) {
    // No sorting needed.
    return;
  }

  // We'll implement bubblesort! Nnice and easy, and nice and slow :)
  int swapped;
  do {
    LinkedListNode *curnode;

    swapped = 0;
    curnode = list->head;
    while (curnode->next != NULL) {
      int compare_result = comparator_function(curnode->payload,
                                               curnode->next->payload);
      if (ascending) {
        compare_result *= -1;
      }
      if (compare_result < 0) {
        // Bubble-swap the payloads.
        LLPayload_t tmp;
        tmp = curnode->payload;
        curnode->payload = curnode->next->payload;
        curnode->next->payload = tmp;
        swapped = 1;
      }
      curnode = curnode->next;
    }
  } while (swapped);
}


///////////////////////////////////////////////////////////////////////////////
// LLIterator implementation.

LLIterator* LLIterator_Allocate(LinkedList *list) {
  // TODO: implement
  if (list->num_elements == 0) {             // the linked list is empty
    return NULL ;
  }

  LLIterator* LLIterator = malloc(sizeof(LLIterator)) ;
  if (LLIterator != NULL) {
    LLIterator->list = list ;                
    LLIterator->node = list->head ;          // the initial node iterator points to is the head node of linked list
  }
  return LLIterator;  // you may want to change this
}

// implemented for you
void LLIterator_Free(LLIterator *iter) {
  free(iter);
}

bool LLIterator_IsValid(LLIterator *iter) {
  // TODO: implement
  if (iter->node == NULL) {        // if iter is past the end of the list, return false
    return false ;
  } else {
    return true;  // you may want to change this
  }
}

bool LLIterator_Next(LLIterator *iter) {
  // TODO: try to advance iterator to the next node and return true if
  // you succeed and are now on a new node, false otherwise
  if (LLIterator_IsValid(iter) == false) {      // if the iterator is now invalid, return false directly
    return false ;
  }
  iter->node = iter->node->next ;               // advance the iterator
  if (LLIterator_IsValid(iter) == false) {      // if the iterator is invalid after advance, return false
    return false ;
  }
  return true;  // you may need to change this return value
}

void LLIterator_Get(LLIterator *iter, LLPayload_t *payload) {
  // TODO: implement
  if (LLIterator_IsValid(iter) != false) {           // if the iterator is now valid
    *payload = iter->node->payload ;
  }
}

bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function) {

  // TODO: implement LLIterator_Remove.  This is the most
  // complex function you'll build.  There are several cases
  // to consider:
  // - degenerate case: the list becomes empty after deleting.
  // - degenerate case: iter points at head
  // - degenerate case: iter points at tail
  // - fully general case: iter points in the middle of a list,
  //                       and you have to "splice".
  //
  // Be sure to call the payload_free_function to free the payload
  // the iterator is pointing to, and also free any LinkedList
  // data structure element as appropriate.
  if (LLIterator_IsValid(iter) == false) {        // if the iterator is now invalid, return false directly
    return false ;
  }
  payload_free_function(iter->node->payload) ;    // free the removed node's payload
  iter->list->num_elements -= 1 ;
  if (iter->list->num_elements == 0) {            // degenerate case: the list becomes empty after deleting
    free(iter->node) ;                            // free the removed node                
    iter->list->head = NULL ;                     // the list becomes empty
    iter->list->tail = NULL ;
    // LLIterator_Free(iter) ;                       // free the now-invalid iterator
    iter->node = NULL ;
    return false ;                                // return false if the deletion succeeded, but the list is now empty
  } 
  if (iter->node == iter->list->head) {           // degenerate case: iter points at head
    iter->list->head = iter->node->next ;         // change the head of the linked list to the second node
    free(iter->node) ;                            // free the removed node 
    iter->node = iter->list->head ;               // change the node iterator points to to the second node
    iter->list->head->prev = NULL ;               // the prev of second node is changed to be NULL
  }
  else if (iter->node == iter->list->tail) {      // degenerate case: iter points at tail
    iter->list->tail = iter->node->prev ;         // change the tail of the linked list to the second last node
    free(iter->node) ;                            // free the removed node 
    iter->node = iter->list->tail ;               // change the node iterator points to to the second last node
    iter->list->tail->next = NULL ;               // the next of second last node is changed to be NULL
  }
  else {                                          // fully general case: iter points in the middle of a list
    LinkedListNode *prev_node = iter->node->prev ;    // get the prev node and next node
    LinkedListNode *next_node = iter->node->next ; 
    free(iter->node) ;                            // free the removed node 
    prev_node->next = next_node ;                 // link the prev node and next node
    next_node->prev = prev_node ;
    iter->node = next_node ;                      // the iterator is now pointing at the successor
  }
  return true;  // you may need to change this return value
}


// Implemented for you
void LLIterator_Rewind(LLIterator *iter) {
  iter->node = iter->list->head;
}
