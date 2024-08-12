CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o
	${CC} main.o -o ${TARGET}
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o
clean:
	rm -rf ${TARGET}
	rm -rf *.o