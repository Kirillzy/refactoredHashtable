#include "wordPair.h"

int pairingInserts(FILE* fp, hashTable* ht) {
    
    char* word1 = getNextWord(fp); // Gives us a malloced word.

    if (word1 == NULL) { return 0; } // NULL due to getWord.h specifying it. 

    char* word2 = NULL;

    while( (word2 = getNextWord(fp)), word2 != NULL) {

        int wordPairLength = strlen(word1) + strlen(word2);
        // +2 is to have " " and the null terminator included.
        char* pair = (char*)malloc((wordPairLength+2)*sizeof(char));
        
        if (snprintf(pair, wordPairLength+2, "%s %s", word1, word2) < 0) {
            fprintf(stderr, "The snprintf apparently has a bad encoding.");
            return -1;
        }

        // Allocate the value since the hash table no longer does it
        int* countVal = (int*)malloc(sizeof(int));
        if (countVal == NULL) {
            fprintf(stderr, "Malloc failure for countVal.\n");
            free(pair);
            return -1;
        }
        *countVal = 1; // Initial count

        int hashInsert = insert(ht, (void*)pair, (void*)countVal);
        if (hashInsert == -1) { 
            free(pair);
            free(countVal);
            return -1; }
        
        // Pair isn't freed since hashtable key owns it now.
        free(word1);
        word1 = word2;
        word2 = NULL;
        pair = NULL;
    }
    free(word1);
    word1 = NULL;
    return 0;
}

