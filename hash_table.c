#include "hash_table.h"




hashTable* hashTableInitial(void) {
    hashTable* ht = (hashTable*)malloc(sizeof(hashTable));
    if (ht == NULL) {
        fprintf(stderr, "Malloc Failure for ht...\n");
        return NULL;
    }

    ht->buckets = 16;
    ht->elements = 0;
    ht->table = calloc(ht->buckets, sizeof(Node*));
    if (ht->table == NULL) {
        fprintf(stderr, "Malloc Failure for table...\n");
        return NULL;
    }
    return ht;
}

Node* createNode(void) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Malloc Failure on newNode.\n");
        return NULL;
    }
    newNode->next = NULL;
    newNode->key = NULL;
    newNode->val = NULL;
    return newNode;
}


// Helper function for "int growTable(hashTable* ht)"
/*
   Takes in the table you want to insert into, the node you want to put, and
   the tables buckets amount, hashes it, and places it into the table by
   using (hash & (buckets -1)) as its index to place into the table.
*/
void growInsert(Node** table, Node* node, int buckets) {
    
    node->next = NULL; // Needed, otherwise it'll hold the prev tables collisions.

    
    unsigned long long hash = crc64((char*)node->key); 
    int index = hash & (buckets - 1);

    Node* head = table[index];
    if (head == NULL) {
        table[index] = node;
        return;    
    }

    while(head->next != NULL) {
        head = head->next;
    }
    head->next = node;
    return;
}


// Helper function for insert "int insert(hashTable* ht, void* key)"
/*
   Takes the hashtable in order to grow its table size and to replace all of elements
   from the current (old) bucket into its new one.

   0 return for good, -1 return for bad (maloc failures).
*/
int growTable(hashTable* ht) {
    
    // Prevents integer overflow on 4x growth (cap at 2^30)
    // 1 << 28 is 268,435,456. Multiplied by 4, it hits the 32-bit limit.
    if (ht->buckets > (1 << 28)) {
        fprintf(stderr, "Max capacity reached, cannot grow further.\n");
        return -1; 
    }

    int growBuckets = ht->buckets * 4;

    Node** growTable = (Node**)calloc(growBuckets, sizeof(Node*));
    if(growTable == NULL) {
        fprintf(stderr, "Calloc failed inside of growTable\n");
        return -1;
    }

    int maxSize = ht->buckets;
    Node** oldTable = ht->table;
    for (int i = 0; i < maxSize; i++) {
        Node* cursor = oldTable[i];

        while(cursor != NULL) {
            Node* nextNode = cursor->next;
            growInsert(growTable, cursor, growBuckets);
            cursor = nextNode;
        }
    }
    free(oldTable);
    ht->table = growTable;
    ht->buckets = growBuckets;
    return 0;
}


// Helper function for "increment(hashTable* ht, void* key)"
/*
   Takes the hashTable, key not in table yet, and index from "hash & (ht->buckets - 1)".
   Then we see if that key already exists in the hashtable. If it does, we return it,
   otherwise we return NULL which signifies it doesn't exist.
*/
Node* contains(hashTable *ht, void* key, int index) {
    Node* head = ht->table[index];

    while (head != NULL) {
        if (strcmp((char*)head->key, (char*)key) == 0) { return head; }
        head = head->next;
    }
    return NULL;
}



// Helper function for "insert(hashTable* ht, void* key)"
// Return 2 if doesn't exist, otherwise 0 for success.
int increment(hashTable* ht, void* key) {

    unsigned long long hash = crc64((char*)key);
    int index = hash & (ht->buckets - 1);
    Node* foundNode = contains(ht, key, index);
    if (foundNode == NULL) { return 2; } // Doesnt exist

    *(int*)foundNode->val += 1;
    return 0; 
}




int insert(hashTable* ht, void* key) {

    // Checking for if we need to grow the tree now.
    double loadFactor = (double)(ht->elements) / (ht->buckets);
    if (loadFactor >= 0.75) {
        int successful = growTable(ht); 
        if (successful == -1) { return -1; }
    }

    // Checking, if key exists, increment val by 1.
    int exists = increment(ht, key);
    // If 0, increment successful.
    if (exists == 0) { 
        free(key);
        return 0; 
    }

    // Inserting node section.
    Node* newNode = createNode();
    if (newNode == NULL) { return -1; }

    int* newNodeVal = malloc(sizeof(int));
    if (newNodeVal == NULL) {
        fprintf(stderr, "Malloc Failure for newNodeVal.\n");
        return -1;
    }

    *newNodeVal = 1;
    newNode->key = (char*)key;
    newNode->val = newNodeVal;

    unsigned long long hash = crc64((char*)key);
    int index = hash & (ht->buckets - 1);

    Node* head = ht->table[index];
    if (head == NULL) {
        ht->table[index] = newNode;
        ht->elements += 1;
        return 0; 
    }
    
    // Searching for next available spot
    while(head->next != NULL) { head = head->next; }

    head->next = newNode;
    ht->elements += 1;
    return 0;
}


void flatten(hashTable *ht, Node* arr) {
    int arrIndex = 0;
    int tableIndex = 0;
    
    while (arrIndex < ht->elements) {
        Node* head = ht->table[tableIndex];
        while (head != NULL) {
            arr[arrIndex] = *head;
            arrIndex += 1;
            head = head->next;
        }
        tableIndex += 1;
    }
}


// Helper function for "emptyTable(hashTable* ht)"
// Just does the usual freeing off to the side.
void freeNode(Node* node) {
    free(node->key); 
    free(node->val);
    node->key = NULL;
    node->val = NULL;
    node->next = NULL;
    free(node);
    node = NULL;
}

void emptyTable(hashTable* ht) {
    for (int i = 0; i < ht->buckets; i++) {
        Node* cursor = ht->table[i]; // (ht->table + i)
        while(cursor != NULL) {
            Node* toFree = cursor;
            cursor = cursor->next;
            freeNode(toFree);
        }
    }
    free(ht->table);
    ht->table = NULL;
    free(ht);
    ht = NULL;
}

