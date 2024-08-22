CC=gcc
CFLAGS=-g -c -Wall
TARGET=run

${TARGET}: main.o get_info.o speed_test.o
	${CC} main.o get_info.o speed_test.o -o ${TARGET} -lpthread -lm 
get_info.o: get_info.c
	${CC} ${CFLAGS} get_info.c -o get_info.o
speed_test.o: speed_test.c
	${CC} ${CFLAGS} speed_test.c -o speed_test.o
main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o
clean:
	rm -rf ${TARGET}
	rm -rf *.o