SOLVER = "/usr/bin/minisat"  # Path of the executable that will process the files 
PARSER = "grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\""   # Shell command to parse the output of the solver
BLOCKSIZE = 4096  # Size of the block of share memory, in bytes. 

CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic
OBJS= master.o shmem_posix.o sem.o 
SHMEM=shmem_posix.o sem.o -pthread -lrt


all: master view slave 

master: shmem_posix.o sem.o master.c
	$(CC) $(CFLAGS) -o master master.c $(SHMEM)

view: shmem_posix.o sem.o view.c
	$(CC) $(CFLAGS) -o view view.c $(SHMEM)

slave: slave.o
	$(CC) $(CFLAGS) -o slave slave.o

slave.o:
	$(CC) $(CFLAGS) -DSOLVER='$(SOLVER)' -DPARSER='$(PARSER)' -c slave.c

shmem_posix.o: shmem_posix.c
	$(CC) $(CFLAGS) -DBLOCKSIZE=$(BLOCKSIZE) -c shmem_posix.c

sem.o: sem.c
	$(CC) $(CFLAGS) -c sem.c

clean:
	rm -f *.o view slave master results
