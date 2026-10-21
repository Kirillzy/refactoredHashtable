# Generic C Hash Table with POSIX I/O and Binary Serialization

A high-performance, fully generic hash table library written in C. Originally built to parse and count word pair frequencies across text files, this project was architected into a robust systems-level library featuring `void*` genericity, raw POSIX system calls for file I/O, and custom binary serialization for state persistence.

## Key Engineering Features

### 1. True Generics via Function Pointers

The core hash table (`hash_table.c`) is completely decoupled from specific data types. It utilizes `void*` payloads and requires user-provided function pointers (callbacks) upon initialization to handle:

* Dynamic hashing algorithms
* Key comparison logic
* Memory allocation and deallocation (Garbage Collection)
* Value updating

### 2. Low-Level POSIX System I/O

Transitioned away from C Standard Library buffered streams (`FILE*`, `fopen`, `fgetc`) in favor of direct POSIX system calls (`open`, `read`, `write`, `close`).

* **Custom Memory Buffering:** Implemented a static 4096-byte memory buffer to drastically minimize kernel system call overhead during large file reads, processing files nearly instantaneously.

### 3. Graph Serialization and State Persistence

Engineered a custom binary protocol to save the complex, pointer-based linked-list graph to disk and accurately reconstruct it dynamically in memory during subsequent executions.

* The `saveTable` and `loadTable` functions flatten the `O(1)` bucket chains into a binary file (`database.bin`), allowing users to parse gigabytes of data once, save the state, and load it instantly later.

### 4. High-Performance Architecture

* **Hashing:** Utilizes CRC64 hashing algorithms.
* **Bitwise Masking:** Replaced standard modulo arithmetic with bitwise AND masking (`hash & (buckets - 1)`) for `O(1)` index calculation.
* **Dynamic Resizing:** Strict power-of-2 bucket resizing triggers at a 0.75 load factor.
* **Memory Safety:** Strict memory handoff protocols ensure zero memory leaks across thousands of dynamic heap allocations (Verified watertight via `Valgrind`).

## Compilation and Usage

Compile the project using the included Makefile:

```bash
make
```

### Modes of Operation

**1. Standard Parsing Mode**
Parse text files, count word pair frequencies, print the top `N` results, and save the state to disk.

```bash
# Example: Print top 10 word pairs from dracula.txt and frankenstein.txt
./pairsofwords -10 dracula.txt frankenstein.txt
```

**2. Binary Load Mode (`-load`)**
Bypass text parsing by loading the pre-computed binary database directly into memory. You can also append new text files to the loaded database.

```bash
# Example: Load previous state and instantly print top 20 pairs
./pairsofwords -20 -load

# Example: Load previous state, append a new book, and save the updated state
./pairsofwords -10 -load newbook.txt
```

## Memory Verification

This project maintains strict memory hygiene. You can verify the absence of memory leaks using Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./pairsofwords -10 -load
```