#ifndef WORDPAIR_H
#define WORDPAIR_H

#include <string.h> // Used for strlen()
#include <stdio.h> // Used for snprintf()
#include <stdlib.h> // Used for malloc(), free(), and NULL
#include "hash_table.h" // Used insert()
#include "getWord.h" // Used for getNextWord()

/*
Takes in a file pointer to read words from and a hashtable to insert word in.

Grabs 2 words from the file using the getNextWord() function provided by
getWord.h, puts the words together using malloc/snprintf. Then uses
insert from the hash_table.h to put that pair into a key, and has the val
be the amount of time it has seen that exact pair of words.

Note: After every loop, it makes 2nd word in the pair become the first,
this means that every word gets used twice except for the start/end words.

0 is returned for good, -1 is returned if something went bad.
*/
int pairingInserts(FILE* fp, hashTable* ht); 


#endif

