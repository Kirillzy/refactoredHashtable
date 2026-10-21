#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include "getWord.h"

#define BUF_SIZE 4096

// Static to keep these vars between our func calls
static char readBuffer[BUF_SIZE];
static int bytesInBuffer = 0;
static int bufferIndex = 0;

// Custom replacement for fgetc
static int customGetC(int fd) {
	if (bufferIndex >= bytesInBuffer) {
		bytesInBuffer = read(fd, readBuffer, BUF_SIZE);
		bufferIndex = 0; // Reset index to start of new chunk.

		// read() returns 0 at EOF, and -1 on error
		if (bytesInBuffer <= 0) {
			return EOF;
		}
	}
	// Return curr char and move index forward
	return (unsigned char)readBuffer[bufferIndex++];
}

/* Reads characters from fd until a single word is assembled */
/* and returns a copy of the word allocated from the heap.   */
/* NULL is returned at EOF.									 */
/* Words are defined to be separated by whitespace and start */
/* with an alphabetic character.  All alphabetic characters  */
/* translated to lower case and punctuation is removed.      */

char* getNextWord(int fd) {
	int ch;								/* holds current character, int to handle EOF safely */
	char wordBuffer[DICT_MAX_WORD_LEN];		/* buffer for build a word */
	int putChar = 0;						/* current pos in buffer   */

	assert(fd >= 0);		/* the file descriptor better not be under 0 */

	/* read characters until we find an alphabetic one (or an EOF) */
	while ((ch = customGetC(fd)) != EOF) {
		if (isalpha(ch)) break;
	}
	if (ch == EOF) return NULL;		/* if we hit an EOF, we're done */

	/* otherwise, we have found the first character of the next word */
	wordBuffer[putChar++] = tolower(ch);

	/* loop, getting more characters (unless there's an EOF) */
	while ((ch = customGetC(fd)) != EOF) {
		/* the word is ended if we encounter whitespace */
		/* or if we run out of room in the buffer       */
		if (isspace(ch) || putChar >= DICT_MAX_WORD_LEN - 1) break;

		/* otherwise, copy the (lowercase) character into the word   */
		/* but only if it is alphanumeric, thus dropping punctuation */
		if (isalnum(ch)) {
			wordBuffer[putChar++] = tolower(ch);
		}
	}

	wordBuffer[putChar] = '\0';		/* terminate the word          */
	return strdup(wordBuffer);		/* re-allocate it off the heap */
}
