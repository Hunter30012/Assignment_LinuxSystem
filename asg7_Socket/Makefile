CC=gcc
CFLAGS=-g -c -Wall
TARGET=server client

${TARGET}: main.o
	${CC} main.o -o ${TARGET}
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o

all:
	${CC} client.c -o client 
	${CC} server.c -o server -lpthread
clean:
	rm -rf ${TARGET}
	rm -rf *.o