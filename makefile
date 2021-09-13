CC = gcc
CFLAGS = -std=gnu99 -g -Wall -pedantic
OBJS= master.o shmem_posix.o sem.o 

all:master

master:$(OBJS) slave view
	$(CC) $(OBJS) -o master -pthread -lrt

view:shmem_posix.o sem.o view.o
	$(CC) shmem_posix.o sem.o view.o -o view -pthread -lrt


slave:slave.o
	$(CC) slave.o -o slave 
slave.o:slave.c

master.o:master.c shmem_posix.h

shmem_posix.o:shmem_posix.c sem.h

sem.o:sem.c

view.o:view.c

clean:
	rm *.o slave master view results

