# Define required macros here
SHELL = /bin/sh

OBJS =  block.o block_with_container.o container.o dedup_containers.o dedup_data_set.o dedup_file.o dynamic_array.o memory_pool.o
CFLAG = -Wall -g -std=c99
CC = gcc
INCLUDE = *.h
LIBS = -lm

hello:${OBJ}
   ${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

clean:
   -rm -f *.o 

.cpp.o:
   ${CC} ${CFLAGS} ${INCLUDES} -c $<