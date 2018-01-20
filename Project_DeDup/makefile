# Define required macros here
SHELL = /bin/sh

OBJS =  block.o block_with_container.o container.o dedup_containers.o dedup_data_set.o dedup_file.o dynamic_array.o memory_pool.o
CFLAG = -Wall -g -std=c99
CC = gcc
INCLUDE = *.h
LIBS = -lm

dedup: ${OBJ}
   ${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

clean:
   -rm -f *.o 
   -rm -f error.txt

   