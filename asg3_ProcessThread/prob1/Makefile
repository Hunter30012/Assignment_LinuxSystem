CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o
	${CC} main.o -o ${TARGET}
main1.o: main1.c
	${CC} ${CFLAGS} main1.c -o main1.o
main2.o: main2.c
	${CC} ${CFLAGS} main2.c -o main2.o
method1: main1.o
	${CC} main1.o -o ${TARGET}
method2: main2.o
	${CC} main2.o -o ${TARGET} -lpthread
clean:
	rm -rf ${TARGET}
	rm -rf *.o