CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic
OBJS= master.o shmem.o sem.o 

all:master

master:$(OBJS) slave view
	$(CC) $(OBJS) -o master -pthread 

view:shmem.o sem.o view.o
	$(CC) shmem.o sem.o view.o -o view -pthread


slave:slave.o
	$(CC) slave.o -o slave 
slave.o:slave.c

master.o:master.c shmem.h

shmem.o:shmem.c sem.h

sem.o:sem.c

view.o:view.c

clean:
	rm *.o slave master view resultado 

