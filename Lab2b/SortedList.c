#define _GNU_SOURCE

#include "SortedList.h"
#include <string.h>
#include <pthread.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  SortedListElement_t *prev = list;
  SortedListElement_t *next = list->next;  // We have a circular list with a dummy node so we start at one after the dummy node
  while(list != next){
    if( strcmp(next->key, element->key) <= 0 )
      break;
    prev=next;
    next=next->next;
  }

  if(opt_yield & INSERT_YIELD)
    pthread_yield();
  
  // Rearrange the pointers so element is now in the list
  prev->next = element;
  element->prev = prev;
  next->prev = element;
  element->next = next;
}

int SortedList_delete(SortedListElement_t *element){
  if(!element)
    return 1;
  if(element->prev && (element->prev->next != element) )
    return 1;
  if(element->next && (element->next->prev != element) )
    return 1;
  if(element->prev)  // If it is not the head
    element->prev->next=element->next;
  if(opt_yield & DELETE_YIELD)
    pthread_yield();
  if(element->next)  // If it is not the last element
    element->next->prev=element->prev;
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  if(!list)
    return NULL;
  SortedListElement_t* cur = list->next;  // We have a circular list with a dummy node so we start at one after the dummy node
  while(cur != list){
    int compare = strcmp(cur->key, key);   // Compares the key at cur and target key. Returns 0 is equal
    if( compare == 0 )     // If we found the element, return it
      return (SortedListElement_t*)cur;
    else if( compare > 0 ){   // If the key at the current element is less than the key we have, we keep traversing
      cur=cur->next;
      if(opt_yield & SEARCH_YIELD)
	pthread_yield();
    }
    else{                          // If the key is greater, since the list is sorted, we are done. We didn't find it
      return NULL;
    }
  }
  return NULL;
}

int SortedList_length(SortedList_t *list){
  int length = 0;
  SortedListElement_t* cur = list->next;  // We have a circular list with a dummy node so we start at one after the dummy node
  while(cur != list){
    if(cur->next && (cur->next->prev != cur) )
      return -1;
    if(cur->prev && (cur->prev->next != cur) )
      return -1;
    length++;
    cur=cur->next;
  }
  return length;
}
