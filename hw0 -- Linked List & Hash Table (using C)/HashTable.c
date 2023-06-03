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
#include <stdint.h>

#include "HashTable.h"
#include "HashTable_priv.h"

///////////////////////////////////////////////////////////////////////////////
// Internal helper functions.
//
#define INVALID_IDX -1

// Grows the hashtable (ie, increase the number of buckets) if its load
// factor has become too high.
static void MaybeResize(HashTable *ht);

// Implemented for you
int HashKeyToBucketNum(HashTable *ht, HTKey_t key) {
  return key % ht->num_buckets;
}

// Deallocation functions that do nothing.  Useful if we want to deallocate
// the structure (eg, the linked list) without deallocating its elements or
// if we know that the structure is empty.
static void LLNoOpFree(LLPayload_t freeme) { }
static void HTNoOpFree(HTValue_t freeme) { }


///////////////////////////////////////////////////////////////////////////////
// HashTable implementation.

// Implemented for you
HTKey_t FNVHash64(unsigned char *buffer, int len) {
  // This code is adapted from code by Landon Curt Noll
  // and Bonelli Nicola:
  //     http://code.google.com/p/nicola-bonelli-repo/
  static const uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
  static const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
  unsigned char *bp = (unsigned char *) buffer;
  unsigned char *be = bp + len;
  uint64_t hval = FNV1_64_INIT;

  // FNV-1a hash each octet of the buffer.
  while (bp < be) {
    // XOR the bottom with the current octet.
    hval ^= (uint64_t) * bp++;
    // Multiply by the 64 bit FNV magic prime mod 2^64.
    hval *= FNV_64_PRIME;
  }
  return hval;
}

// Implemented for you
HashTable* HashTable_Allocate(int num_buckets) {
  HashTable *ht;
  int i;

  // Allocate the hash table record.
  ht = (HashTable *) malloc(sizeof(HashTable));
  if (!ht) {
    return ht;
  }

  // Initialize the record.
  ht->num_buckets = num_buckets;
  ht->num_elements = 0;
  ht->buckets = (LinkedList **) malloc(num_buckets * sizeof(LinkedList *));
  for (i = 0; i < num_buckets; i++) {
    ht->buckets[i] = LinkedList_Allocate();
  }

  return ht;
}

// Implemented for you
void HashTable_Free(HashTable *table,
                    ValueFreeFnPtr value_free_function) {
  int i;

  // Free each bucket's chain.
  for (i = 0; i < table->num_buckets; i++) {
    LinkedList *bucket = table->buckets[i];
    HTKeyValue_t *kv;

    // Pop elements off the chain list one at a time.  We can't do a single
    // call to LinkedList_Free since we need to use the passed-in
    // value_free_function -- which takes a HTValue_t, not an LLPayload_t -- to
    // free the caller's memory.
    while (LinkedList_NumElements(bucket) > 0) {
      LinkedList_Pop(bucket, (LLPayload_t *)&kv);
      value_free_function(kv->value);
      free(kv);
    }
    // The chain is empty, so we can pass in the
    // null free function to LinkedList_Free.
    LinkedList_Free(bucket, LLNoOpFree);
  }

  // Free the bucket array within the table, then free the table record itself.
  free(table->buckets);
  free(table);
}

// Implemented for you
int HashTable_NumElements(HashTable *table) {
  return table->num_elements;
}

// Helper function: check whether the hashtable has the key
// If has key, return true, store the old key value pair in kv
// If not has key, return false
bool Has_Key(LLIterator *iter, HTKey_t key, HTKeyValue_t **kv) {
  // iterate through the bucket
  while(1) {
    if (iter == NULL) {      // invalid iterator
      return false ;
    }
    
    LLIterator_Get(iter, (LLPayload_t *)kv) ;
    // printf("kv_key: %ld\n", kv->key) ;
    // printf("kv_value: %p\n", (void*)kv->value) ;

    if ((*kv)->key == key) {      // the key already exist in the bucket
      return true ;
    }
    bool has_next = LLIterator_Next(iter) ;
    if (has_next == false) {               // already pass the end (i.e. the key doesn't exist in the bucket)
      return false ;
    }
  }
}

bool HashTable_Insert(HashTable *table,
                      HTKeyValue_t newkeyvalue,
                      HTKeyValue_t *oldkeyvalue) {
  int bucket;
  LinkedList *chain;

  MaybeResize(table);

  // Calculate which bucket and chain we're inserting into.
  bucket = HashKeyToBucketNum(table, newkeyvalue.key);
  chain = table->buckets[bucket];

  // STEP 1: finish the implementation of InsertHashTable.
  // This is a fairly complex task, so you might decide you want
  // to define/implement a helper function that helps you find
  // and optionally remove a key within a chain, rather than putting
  // all that logic inside here.  You might also find that your helper
  // can be reused in steps 2 and 3.

  HTKeyValue_t *kv ;

  // manufacture an iterator for the list
  LLIterator* iter = LLIterator_Allocate(chain); 

  // printf("newkeyvalue address: %p\n", (void*)&newkeyvalue) ;

  // store the new key value in the heap
  HTKeyValue_t* newkv = malloc(sizeof(HTKeyValue_t)) ;
  if (!newkv) {
    return false ;
  }
  newkv->key = newkeyvalue.key ;
  newkv->value = newkeyvalue.value ;

  if (Has_Key(iter, newkv->key, &kv)) {       // the key already exists in the bucket

    // printf("key exist") ;
    // printf("head payload: %p\n", chain->head->payload) ;
    // printf("newkv: %p\n", (void*)newkv) ;
    // printf("%ld\n", newkv->key) ;
    // printf("%p\n", (void*)newkv->value) ;

    oldkeyvalue->key = kv->key ;        // the old (key,value) was returned through the oldkeyvalue return parameter
    oldkeyvalue->value = kv->value ;
    // free(kv);                           // free the old payload
    HTNoOpFree(kv->value) ;             // free the value of old value
    kv->value = newkv->value ;
    free(newkv) ;
    LLIterator_Free(iter) ;             // free the iterator
    return true ;
  } 
  else {                                // there was no existing (key,value) with that key
    // free(kv);
    LinkedList_Append(chain, (LLPayload_t)newkv) ;

    // printf("key not exist\n") ;
    // printf("head payload: %p\n", chain->head->payload) ;
    // printf("newkv: %p\n", (void*)newkv) ;
    // printf("%ld\n", newkv->key) ;
    // printf("%p\n", (void*)newkv->value) ;

    LLIterator_Free(iter) ;             // free the iterator
    table->num_elements += 1 ;
    return false ;
  }
}

bool HashTable_Find(HashTable *table,
                    HTKey_t key,
                    HTKeyValue_t *keyvalue) {
  // STEP 2: implement HashTable_Find.
  int bucket;
  LinkedList *chain;
  HTKeyValue_t *kv ;

  MaybeResize(table);

  // Calculate which bucket and chain we're looking through.
  bucket = HashKeyToBucketNum(table, key);
  chain = table->buckets[bucket];

  // manufacture an iterator for the list
  LLIterator* iter = LLIterator_Allocate(chain); 

  if (Has_Key(iter, key, &kv)) {  // the key was found, (key,value) was returned to the caller via the keyvalue return parameter
    keyvalue->key = kv->key ;
    keyvalue->value = kv->value ;
    // free(kv) ;
    LLIterator_Free(iter) ;             // free the iterator
    return true ;
  } 
  else {                                // the key wasn't found in the HashTable
    // free(kv) ;
    LLIterator_Free(iter) ;             // free the iterator
    return false ;
  }
}

bool HashTable_Remove(HashTable *table,
                      HTKey_t key,
                      HTKeyValue_t *keyvalue) {
  // STEP 3: implement HashTable_Remove.
  int bucket;
  LinkedList *chain;
  HTKeyValue_t *kv ;

  MaybeResize(table);

  // Calculate which bucket and chain we're removing from.
  bucket = HashKeyToBucketNum(table, key);
  chain = table->buckets[bucket];

  // manufacture an iterator for the list
  LLIterator* iter = LLIterator_Allocate(chain); 

  if (Has_Key(iter, key, &kv)) {  // the key was found, (key,value) was returned to the caller via the keyvalue return parameter
    keyvalue->key = kv->key ;
    keyvalue->value = kv->value ;
    HTNoOpFree(kv->value) ;             // free the value of old value
    // free(kv) ;
    LLIterator_Remove(iter, free);
    LLIterator_Free(iter) ;             // free the iterator
    table->num_elements -= 1 ;
    return true ;
  } 
  else {                                // the key wasn't found in the HashTable
    // free(kv) ;
    LLIterator_Free(iter) ;             // free the iterator
    return false ;
  }
}


///////////////////////////////////////////////////////////////////////////////
// HTIterator implementation.

// Implemented for you
HTIterator* HTIterator_Allocate(HashTable *table) {
  HTIterator *iter;
  int         i;

  iter = (HTIterator *) malloc(sizeof(HTIterator));

  // If the hash table is empty, the iterator is immediately invalid,
  // since it can't point to anything.
  if (table->num_elements == 0) {
    iter->ht = table;
    iter->bucket_it = NULL;
    iter->bucket_idx = INVALID_IDX;
    return iter;
  }

  // Initialize the iterator.  There is at least one element (bucket) in the
  // table, so find the first element (bucket) and point the iterator at it.
  iter->ht = table;
  for (i = 0; i < table->num_buckets; i++) {
    if (LinkedList_NumElements(table->buckets[i]) > 0) {
      iter->bucket_idx = i;
      break;
    }
  }
  iter->bucket_it = LLIterator_Allocate(table->buckets[iter->bucket_idx]);
  return iter;
}

// Implemented for you
void HTIterator_Free(HTIterator *iter) {
  if (iter->bucket_it != NULL) {
    LLIterator_Free(iter->bucket_it);
    iter->bucket_it = NULL;
  }
  free(iter);
}

bool HTIterator_IsValid(HTIterator *iter) {
  // STEP 4: implement HTIterator_IsValid.
  if (iter->bucket_it != NULL && LLIterator_IsValid(iter->bucket_it) == true) {    // if iter is not at the end of the table, return true
    return true ;
  }
  return false;  // you may need to change this return value
}

bool HTIterator_Next(HTIterator *iter) {
  int i ;

  // STEP 5: implement HTIterator_Next.
  if (HTIterator_IsValid(iter) == false) {      // if the iterator is now invalid, return false directly
    return false ;
  }
  HashTable* table = iter->ht ;
  // if there is element in the current bucket, advance the LLIterator, the HTIterator remains the same
  if (LLIterator_Next(iter->bucket_it) == true) {     // call LLIterator_Next() to advance the LLIterator
    return true ;           
  }
  // iterate through the buckets to find next element
  for (i = iter->bucket_idx+1; i < table->num_buckets; i++) {
    LinkedList* list_ptr = (table->buckets)[i] ;
    if (LinkedList_NumElements(list_ptr) == 0) {       // if there is no element in the current bucket, move to the next bucket
      continue ;
    }
    LLIterator_Free(iter->bucket_it) ;       // free the current LLIterator 
    iter->bucket_it = LLIterator_Allocate(table->buckets[i]) ;    // assign the bucket iterator to point to the new bucket_it
    iter->bucket_idx = i ;
    break ;           // exit the loop
  }
  // there is no element remaining in the table(i = table->num_buckets, exceeds the num of buckets)
  if (i == table->num_buckets) { return false ; }
  // check whether the HTIterator is valid after advance the iterator
  if (HTIterator_IsValid(iter) == false) {
    return false ;
  } else {
    return true ;
  }

  return true ;  // you may need to change this return value
}

bool HTIterator_Get(HTIterator *iter, HTKeyValue_t *keyvalue) {
  // STEP 6: implement HTIterator_Get.
  if (HTIterator_IsValid(iter) == true) {
    HTKeyValue_t *kv ;
    LLIterator_Get(iter->bucket_it, (LLPayload_t *)&kv);
    keyvalue->key = kv->key ;
    keyvalue->value = kv->value ;
    return true ;
  }

  return false;  // if the iterator is not valid or the table is empty
}

// Implemented for you
bool HTIterator_Remove(HTIterator *iter, HTKeyValue_t *keyvalue) {
  HTKeyValue_t kv;

  // Try to get what the iterator is pointing to.
  if (!HTIterator_Get(iter, &kv)) {
    return false;
  }

  // Advance the iterator.  Thanks to the above call to
  // HTIterator_Get, we know that this iterator is valid (though it
  // may not be valid after this call to HTIterator_Next).
  HTIterator_Next(iter);

  // Lastly, remove the element.  Again, we know this call will succeed
  // due to the successful HTIterator_Get above.
  HashTable_Remove(iter->ht, kv.key, keyvalue);

  return true;
}

// Implemented for you
static void MaybeResize(HashTable *ht) {
  HashTable *newht;
  HashTable tmp;
  HTIterator *it;

  // Resize if the load factor is > 3.
  if (ht->num_elements < 3 * ht->num_buckets)
    return;

  // This is the resize case.  Allocate a new hashtable,
  // iterate over the old hashtable, do the surgery on
  // the old hashtable record and free up the new hashtable
  // record.
  newht = HashTable_Allocate(ht->num_buckets * 9);

  // Loop through the old ht copying its elements over into the new one.
  for (it = HTIterator_Allocate(ht);
       HTIterator_IsValid(it);
       HTIterator_Next(it)) {
    HTKeyValue_t item, unused;

    HTIterator_Get(it, &item);
    HashTable_Insert(newht, item, &unused);
  }

  // Swap the new table onto the old, then free the old table (tricky!).  We
  // use the "no-op free" because we don't actually want to free the elements;
  // they're owned by the new table.
  tmp = *ht;
  *ht = *newht;
  *newht = tmp;

  // Done!  Clean up our iterator and temporary table.
  HTIterator_Free(it);
  HashTable_Free(newht, &HTNoOpFree);
}
