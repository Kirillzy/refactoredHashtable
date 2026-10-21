#include "getWord.h"
#include "hash_table.h"
#include "wordPair.h"
// stdio.h, stdlib.h, string.h are in the header files already


// Defining how to hash specific key
unsigned long long myStringHash(void* key) {
    return crc64((char*)key); 
}

// Defining how to compare specific keys
int myStringCmp(void* keyA, void* keyB) {
    return strcmp((char*)keyA, (char*)keyB);
}

// Defining how to free payloads
void myStandardFree(void* ptr) {
    free(ptr);
}

void myUpdateVal(void* val) {
	*(int*)val += 1;
}


/* 
   Pass in the argv[1] which is only where count flag may appear.
   It is also known that count starts with '-' and the rest are numbers. (And we only accept positive numbers)
   If the provided flag doesn't have a '-' at the start, we return -1 since its a file then.
   Once we start the conversion to int, if it fails, we return -2 and close the program for invalid flag.
   If success, we just return the count back
*/
int getCount(char* flag) {
	if (flag[0] != '-') { return -1; }

	int count = atoi(flag+1);
	if (count <= 0) { 
		fprintf(stderr, "Invalid count flag provided.\n");
		return -2; 
	}
	return count;
}

/*
   This function is needed to be passed in for qsort. Takes in Node*'s that were degraded.
   What is compared is the Nodes's values, which in this assignment signifies how much
   we have seen of the exact word pair in key. 
   Then qsort uses this to correctly do descending order.

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

/*
   Takes in the counter, which is how many word pairs we want printed.
   Also takes in the element size, and the Node* arr we'll print from.
   If the count is positive, we print for the amount that was asked,
   else, we print it all out.
*/
void printPairs(int count, int elements, Node* arr) {
	int index = 0;
	while (count != 0 && index < elements) {
		fprintf(stdout, "%10d %s\n", *(int*)arr[index].val, (char*)arr[index].key);

		index += 1;
		
		if (count == -1) {
			continue;
		}
		count -= 1;
	}
}



int main(int argc, char **argv) {	
	// Managing argc and argv edge cases
	if (argc == 1) {
		fprintf(stderr, "No files given to arguement.\n");
		return -1; 
	}
	
	int count = -1;  // -1 signifies count hasnt been provided.
	if (argc >= 2) {  
		count = getCount(argv[1]);
		
		if (count == -2) { return -1; }

		else if (count != -1 && argc == 2) {
			fprintf(stderr, "No files, only count given.\n");
			return -1;
		}
	}

	// Setting up the file index
	int fileIndex = 1;
	if (count != -1) { fileIndex = 2; } 
 
	hashTable* ht = hashTableInitial(myStringHash, myStringCmp, myStandardFree, myStandardFree, myUpdateVal);
	if (ht == NULL) { return -1; } 
	
	// Goes through each file and inserting pairs of words into the hashtable
	while (fileIndex < argc) {
		int fd = open(argv[fileIndex], "O_RDONLY");
		if (fd < 0) {
			fprintf(stderr, "fd -1 return\n");
			fileIndex += 1;
			continue;
		}
		
		// pairs the words up and inserts them, 0 for good, -1 for bad
		int pairing = pairingInserts(fd, ht); 
		if (pairing == -1) { 
			emptyTable(ht);
			close(fd);
			return -1;
		}
		
		fileIndex += 1;
		close(fd);
	}
	if (ht->elements == 0) {
		emptyTable(ht);
		return 0;
	}
    Node arr[ht->elements];
	// Will put all the nodes linearly with no collisions for qsort()
	flatten(ht, arr);

	// Sorts the array using the values inside the nodes.
	qsort(arr, ht->elements, sizeof(Node), compareInts);
	printPairs(count, ht->elements, arr);
	emptyTable(ht);

	return 0;
}

