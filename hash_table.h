#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "crc64.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h> // Needed for open
#include <unistd.h> // Needed for close
#include <errno.h>

typedef struct Node {
    void* key;
    void* val;
    struct Node* next;
} Node;

typedef struct hashTable {
    int buckets;
    int elements;
    Node** table;

    // Generic Callbacks
    unsigned long long (*hashFunc)(void* key);
    int (*cmpFunc)(void* keyA, void* keyB);
    void (*freeKey)(void* key);
    void (*freeVal)(void* val);
    void (*updateVal)(void* val);
} hashTable;

/*
This functions job is to return a ready made hashTable to use.
It does this by using malloc on the hashTable struct, checking for NULL,
initializing the buckets to 16, elements to 0, and callocing the table
to the size of the buckets (16) where all of them are set to NULL (0) to start, 
and made to store Nodes in its arrays.
*/
hashTable* hashTableInitial(
    unsigned long long (*hashFunc)(void*),
    int (*cmpFunc)(void*, void*),
    void (*freeKey)(void*),
    void(*freeVal)(void*),
    void (*updateVal)(void*)
);


/*
Takes in a hashtable and the key needed to be inserted for it. For this case, key is actually
hardcoded to be (char*)key inside the function, but the purpose of passing it as void* was to
practice casting things into void* and back into its actual type.

The insert function handles checking for if the hashtable needs to grow, needs to just increment
the val, or if it actually needs a new node to add into the hashtable.

A return of 0 is good, -1 is bad and will result an error.
*/
int insert(hashTable* ht, void* key, void* val);


/*
Takes in the hashtable and the array that was allocated on the stack; where we loop through the
entire hashtable's table and put them into the stack array so it can be linear which is needed for qsort().
*/
void flatten(hashTable* ht, Node* arr);


/*
Takes in the hashtable where its job is to empty out all of the hashtables contents such as key's, values,
and node itself; ending off with the actual hashtable. This is to help exit out of the program with no memory leaks.
*/
void emptyTable(hashTable* ht);


#endif

