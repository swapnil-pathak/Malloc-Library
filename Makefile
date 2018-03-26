CC=gcc
CFLAGS=-g -O0 -fPIC -Werror -Wall

TESTS=t-test1
HEADERS=dsnf.h buddy.c malloc.c realloc.c free.c calloc.c posix_memalign.c memalign.c malloc_stats.c

all:	${TESTS} libmalloc.so

clean:
	rm -rf *.o *.so ${TESTS}

lib: libmalloc.so

libmalloc.so: buddy.c malloc.c realloc.c free.c calloc.c posix_memalign.c memalign.c malloc_stats.c
	$(CC) $(CFLAGS) -shared -Wl,--unresolved-symbols=ignore-all buddy.c malloc.c realloc.c free.c calloc.c posix_memalign.c memalign.c malloc_stats.c -o $@

%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

# For every XYZ.c file, generate XYZ.o.
%.o: %.c ${HEADERS}
	$(CC) $(CFLAGS) $< -c -o $@ -lpthread

check:	libmalloc.so t-test1
	LD_PRELOAD=`pwd`/libmalloc.so ./t-test1

dist: clean
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
