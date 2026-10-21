#include "hash_table.h"

hashTable* hashTableInitial(
    unsigned long long (*hashFunc)(void*),
    int (*cmpFunc)(void*, void*),
    void (*freeKey)(void*),
    void(*freeVal)(void*),
    void (*updateVal)(void*)
) {
    hashTable* ht = (hashTable*)malloc(sizeof(hashTable));
    if (ht == NULL) {
        fprintf(stderr, "Malloc Failure for ht...\n");
        return NULL;
    }

    ht->buckets = 16;
    ht->elements = 0;

    // Assigning the callbacks to handle them generically.
    ht->hashFunc = hashFunc;
    ht->cmpFunc = cmpFunc;
    ht->freeKey = freeKey;
    ht->freeVal = freeVal;
    ht->updateVal = updateVal;

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

// saves the total buckets, and tot elements, and then maps out exact length of every linked list chain to rebuilt later.
int saveTable(hashTable* ht, const char* filename, void (*writeNode)(int fd, void* key, void* val)) {

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { return -1; }

    // core metadata
    write(fd, &ht->buckets, sizeof(int));
    write(fd, &ht->elements, sizeof(int));

    // saves layout in a for loop
    for (int i = 0; i < ht->buckets; i++) {
        int chainLen = 0;
        Node* cursor = ht->table[i];

        // Counts nodes in this bucket
        while (cursor != NULL) {
            chainLen++;
            cursor = cursor->next;
        }

        // Writes the chain length so loader knows how many to expect.
        write(fd, &chainLen, sizeof(int));

        // Iteratate again but actually writing the actual generic data
        cursor = ht->table[i];
        while (cursor != NULL) {
            writeNode(fd, cursor->key, cursor->val);
            cursor = cursor->next;
        }
    }
    close(fd);
    return 0;
}

hashTable* loadTable(const char* filename,
    unsigned long long (*hashFunc)(void*),
    int (*cmpFunc)(void*, void*),
    void (*freeKey)(void*),
    void (*freeVal)(void*),
    void (*updateVal)(void*),
    void (*readNode)(int fd, void** key, void** val)) {

        int fd = open(filename, O_RDONLY);
        if (fd < 0) { return NULL; }

        int loadedBuckets;
        int loadedElements;
        read(fd, &loadedBuckets, sizeof(int));
        read(fd, &loadedElements, sizeof(int));

        // Initialize using generic initializers
        hashTable* ht = hashTableInitial(hashFunc, cmpFunc, freeKey, freeVal, updateVal);
        if (ht == NULL) {
            close(fd);
            return NULL;
        }

        // Override default (16 buckets) with the saved state
        free(ht->table);
        ht->buckets = loadedBuckets;
        ht->elements = loadedElements;
        ht->table = calloc(ht->buckets, sizeof(Node*));

        for (int i = 0; i < ht->buckets; i++) {
            int chainLen;
            read(fd, &chainLen, sizeof(int));

            Node* tail = NULL; // Keep track of end of list to append 

            for (int j = 0; j < chainLen; j++) {
                void* key = NULL;
                void* val = NULL;

                // What user provided to read the generic data
                readNode(fd, &key, &val);

                // Rebuild node
                Node* newNode = createNode();
                newNode->key = key;
                newNode->val = val;

                // Link into chain
                if (ht->table[i] == NULL) {
                    ht->table[i] = newNode;
                }
                else {
                    tail->next = newNode;
                }
                tail = newNode;
                
            }
        }
        close(fd);
        return ht;
}


// Helper function for "int growTable(hashTable* ht)"
/*
   Takes in the table you want to insert into, the node you want to put, and
   the tables buckets amount, hashes it, and places it into the table by
   using (hash & (buckets -1)) as its index to place into the table.
*/
void growInsert(hashTable* ht, Node** table, Node* node, int buckets) {
    
    node->next = NULL; // Needed, otherwise it'll hold the prev tables collisions.

    
    // OLD: unsigned long long hash = crc64((char*)node->key); 
    unsigned long long hash = ht->hashFunc(node->key);

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
            growInsert(ht, growTable, cursor, growBuckets);
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
        // OLD: if (strcmp((char*)head->key, (char*)key) == 0) { return head; }
        if (ht->cmpFunc(head->key, key) == 0) { return head; }
        head = head->next;
    }
    return NULL;
}



// Helper function for "insert(hashTable* ht, void* key)"
// Return 2 if doesn't exist, otherwise 0 for success.
int increment(hashTable* ht, void* key) {

    // OLD: unsigned long long hash = crc64((char*)key);
    unsigned long long hash = ht->hashFunc(key);

    int index = hash & (ht->buckets - 1);
    Node* foundNode = contains(ht, key, index);
    if (foundNode == NULL) { return 2; } // Doesnt exist

    // OLD: *(int*)foundNode->val += 1;
    ht->updateVal(foundNode->val);
    return 0; 
}




int insert(hashTable* ht, void* key, void* val) {

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
        ht->freeKey(key);
        ht->freeVal(val);
        return 0; 
    }

    // Inserting node section.
    Node* newNode = createNode();
    if (newNode == NULL) { return -1; }

    newNode->key = key;
    newNode->val = val;

    // OLD: unsigned long long hash = crc64((char*)key);
    unsigned long long hash = ht->hashFunc(key);
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
void freeNode(hashTable* ht, Node* node) {
    if (ht->freeKey != NULL && node->key != NULL) {
        ht->freeKey(node->key);
    }
    if (ht->freeVal != NULL && node->val != NULL) {
        ht->freeVal(node->val);
    }
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
            freeNode(ht, toFree);
        }
    }
    free(ht->table);
    ht->table = NULL;
    free(ht);
    ht = NULL;
}

