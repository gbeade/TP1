CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic
OBJS= application.o shmem.o sem.o 

all:application

application:$(OBJS) slave
	$(CC) $(OBJS) -o application -pthread 

slave:slave.o
	$(CC) slave.o -o slave 
slave.o:slave.c

application.o:application.c shmem.h

shmem.o:shmem.c sem.h

sem.o:sem.c

clean:
	rm *.o slave application 

