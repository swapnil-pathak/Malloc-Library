# Malloc-Library

Design overview:
Implements buddy algorithm using doubly linked list and maintains freelist of free-blocks.
Any memory request greater than 2048 bytes is handled by mmap()
Otherwise, it is handled by sbrk() system call
The library is thread-safe

Improvements:
Big locks implemented. Making them smaller

How to Run:
1. Run 'make lib' to create libmalloc.so(Might want to do 'make clean' to clear any compiled files at the start)
2. Run 'make check' to run test cases using the test-file

Simply running 'make check' will generate libmalloc.so and run it against the test-file(t-test1.c)
