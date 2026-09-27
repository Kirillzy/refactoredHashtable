pairsofwords: main.o hash_table.o crc64.o getWord.o wordPair.o
	cc -o pairsofwords main.o hash_table.o crc64.o getWord.o wordPair.o

testSuite: testSuite.o hash_table.o crc64.o getWord.o wordPair.o
	cc -o testSuite testSuite.o hash_table.o crc64.o getWord.o wordPair.o

main.o: main.c hash_table.h wordPair.h
	cc main.c -c

testSuite.o: testSuite.c hash_table.h wordPair.h
	cc testSuite.c -c

hash_table.o: hash_table.c hash_table.h crc64.h
	cc hash_table.c -c

crc64.o: crc64.c crc64.h
	cc crc64.c -c

getWord.o: getWord.c getWord.h
	cc getWord.c -c

wordPair.o: wordPair.c wordPair.h hash_table.h
	cc wordPair.c -c

run: pairsofwords
	./pairsofwords

.PHONY: clean
clean:
	rm pairsofwords *.o
