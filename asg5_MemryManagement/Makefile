CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o my_malloc.o
	${CC} main.o my_malloc.o -o ${TARGET}
my_malloc.o:
	${CC} ${CFLAGS} my_malloc/my_malloc.c -o my_malloc.o
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o -I ./my_malloc
clean:
	rm -rf ${TARGET}
	rm -rf *.o