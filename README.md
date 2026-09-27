This readme is designed to hopefully help navigate with how I left comments and the reason for my design choices.

## My way of commenting.

I only put my functions in the `.h` files that are used by other `.c` files that are not by the same name. For those functions, their comment block codes for how they work will be in the `.h` files.

For all other functions, I have mostly classified them as helper functions and their comments will mention who they are a helper towards and will also have a comment block on what they do, but its inside those `.c` files specifically.


## What design choice did I do and why?

I chose an initial capacity of 16 buckets to optimize both speed and memory efficiency. This decision was driven by three technical factors:

- Performance over Modulo: Instead of using the modulo operator (`hash % buckets`) to find an index, I used the bitwise AND approach (`hash & (buckets - 1)`). Bitwise operations are significantly faster at the CPU level, speeding up every single insert and resize operation. (Which I learned about while researching approaches to do a hashtable)

- The Power of 2 Requirement: For the bitwise AND technique to distribute keys evenly without heavy bias or clustering, the table size _must_ be a power of 2.

- Balancing Initial Load: I selected 16 ($2^4$) rather than a smaller power of 2 (like 2, 4, or 8) to prevent immediate resizing. With a load factor threshold of 0.75, starting too small would force the table to grow almost instantly, wasting time for little gain. 16 provided me a strong middle ground that handles small files cleanly while still demonstrating the table's growth capabilities on larger texts. (Also why I grow by `4x` to keep it powers of 2).
#### Trade-offs I Considered

- Hash Function Dependency: The bitwise AND approach (`hash & (buckets - 1)`) only evaluates the lowest bits of the hash. If paired with a weak hash function, this causes severe clustering compared to using modulo with a prime number (which was my second pick if I couldn't get this working). I mitigated this slightly by using `crc64`, but if I were to in the future get a better hash function or even make one on my own, it would still drastically help than using modulo.

- Memory Overhead: Another issue is when I had to regrow the tree each time, the (`4x`) multiplier to definitely takes a hit to how much memory I am taking up. Worst case is that it barely uses that new memory. Best case is that it uses most of it and stops running.

