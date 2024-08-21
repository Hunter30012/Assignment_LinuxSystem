CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o get_info.o
	${CC} main.o get_info.o -o ${TARGET} -lpthread -lm 
get_info.o: get_info.c
	${CC} ${CFLAGS} get_info.c -o get_info.o
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o
clean:
	rm -rf ${TARGET}
	rm -rf *.o