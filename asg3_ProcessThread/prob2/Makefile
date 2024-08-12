CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o daemon.o
	${CC} main.o daemon.o -o ${TARGET}
daemon.o:
	${CC} ${CFLAGS} daemon/daemon.c -o daemon.o
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o -I ./daemon
clean:
	rm -rf ${TARGET}
	rm -rf *.o