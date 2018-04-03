# Define required macros here
SHELL = /bin/sh

OBJS =  block.o block_with_container.o container.o dedup_containers.o dedup_data_set.o dedup_file.o dynamic_array.o memory_pool.o avltree.o
CFLAG = -Wall -g -std=c99
CC = gcc
INCLUDE = *.h
LIBS = -lm

default: dedup

dedup: ${OBJS}
	${CC} ${CFLAG}  -o $@ ${OBJS} 
	
block.o: block.c block.h
	${CC} ${CFLAG} -c block.c  
	
block_with_container.o: block_with_container.c block_with_container.h
	${CC} ${CFLAG} -c block_with_container.c 
	
container.o: container.c container.h
	${CC} ${CFLAG} -c container.c 
	
dedup_containers.o: dedup_containers.c dedup_containers.h
	${CC} ${CFLAG} -c dedup_containers.c 
	
dedup_data_set.o: dedup_data_set.c dedup_data_set.h
	${CC} ${CFLAG} -c dedup_data_set.c
	
dedup_file.o: dedup_file.c dedup_file.h
	${CC} ${CFLAG} -c dedup_file.c
	
dynamic_array.o: dynamic_array.c dynamic_array.h
	${CC} ${CFLAG} -c dynamic_array.c
	
memory_pool.o: memory_pool.c memory_pool.h
	${CC} ${CFLAG} -c memory_pool.c  

avltree.o: avltree.c avltree.h
	${CC} ${CFLAG} -c avltree.c  

clean:
	-rm -f *.o 

   