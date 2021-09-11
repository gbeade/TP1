CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic
OBJS= master.o shmem.o sem.o 

all:master

master:$(OBJS) slave
	$(CC) $(OBJS) -o master -pthread 

slave:slave.o
	$(CC) slave.o -o slave 
slave.o:slave.c

master.o:master.c shmem.h

shmem.o:shmem.c sem.h

sem.o:sem.c

clean:
	rm *.o slave master resultado 

