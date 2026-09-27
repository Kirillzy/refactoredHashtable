#include "getWord.h"
#include "hash_table.h"
#include "wordPair.h"
#include <assert.h>
// stdio.h, stdlib.h, string.h are in the header files already

/* 
   Pass in the argv[1] which the assignment highlights is the only place count can appear.
   With that knowledge, it is also known that count starts with "-" and the rest are numbers. (And we only accept positive numbers)
   So if the conversion is successful, we return the count, else we return -1 for count given.
   No count given also means that all of the words will be printed.
*/
int getCount(char* flag) {
	int count = atoi(flag+1);
	if (count <= 0) {
		return -1; // This means that we weren't given a count.
	}
	return count;
}

/*
   This function is needed to be passed in for qsort. 
   What is compared of the Nodes's values, which in this assignment signifies how much
   we have seen of the exact word pair in key. Then qsort uses this to correctly
   do descending order.
   Neg < 0 means "a" comes before "b"
   0 means theyre equal, so either order
   Pos > 0 means "b" comes before "a"
*/ 
int compareInts(const void* a, const void* b) {
	const Node* currA = (const Node*)a;
	const Node* currB = (const Node*)b;
	int valA = *(int*)currA->val;
	int valB = *(int*)currB->val;
	
	// Descending ordering
	if(valB > valA) {
		return 1;
	}
	if (valB < valA) {
		return -1;
	}
	return 0;
}


int main(int argc, char **argv) {
	// word pair counting logic using hash table goes here
	if (argc == 1) {
		fprintf(stderr, "No files given to arguement.\n");
		return -1; // -1 will be our designated "Failure/Error"
	}
	
	// Testing for the countTest function working properly
	char* countTest1 = "-5";
	assert(getCount(countTest1) == 5 && "Fail..");
	char* countTest2 = "wrong";
	assert(getCount(countTest2) == -1 && "Fail..");
	char* countTest3 = "-999";
	assert(getCount(countTest3) == 999 && "Fail..");
	char* countTest4 = "--99";
	assert(getCount(countTest4) == -1 && "Fail..");

	// --------------------
	// Hashtable initialization test
	hashTable* ht = hashTableInitial();
	assert(ht->buckets == 16 && "This shouldn't happen, buckets not 16?");
	assert(ht->elements == 0 && "Elements isnt 0??");
	assert(ht->table[0] == NULL && "How did memory get in here??");
	assert(ht->table[5] == NULL && "How did memory get in here??");
	
	int fileIndex = 2;

	// ---------------------
	while (fileIndex < argc) {
		// We will keep looping with opening new files until the while loop condition is met
		FILE* fp = fopen(argv[fileIndex], "r");
		if (fp == NULL) {
			fprintf(stderr, "File returned NULL\n");
			// We will emptyTable here and exit reminder
			return -1;
		}

		int pairing = pairingInserts(fp, ht); // pairs the words up and inserts them, 0 for good, -1 for bad
		if (pairing == -1) { // Insert the function should tell us the error.
			// We will emptyTable here and exit reminder
			return -1;
		}

		// Testing for when the argv's are -5 test.txt
		Node* test1 = ht->table[7];
		assert(*(int*)(test1->val) == 15 && "Increment doesnt work?");
		
		fileIndex += 1;
	}
	

	

	return 0;
}


