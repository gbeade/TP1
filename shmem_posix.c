#include "shmem_posix.h"
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//TODO falta hacer las validaciones con pvs_studio, cambiar sprintf para que no de warnings

static void checkOffset(int newOffset,bufferADT buffer);

typedef struct blockCDT{ 
	int shmid;
	int size;
} blockCDT;

typedef struct bufferCDT{
	char * mem;
	sem_t * mutex;
	long offset;
} bufferCDT;


blockADT createBlock(const char *pathname, int size){

	int shmid = shm_open("/something", O_CREAT | O_RDWR, 0777); //El problema con esto es que shm_open toma un path absoluto y no uno relativo
	if(shmid == ERROR){
		perror("shm_open could not create the shmem segment");
		return NULL;
	}

	if(ftruncate(shmid, size) == ERROR){
		perror("ftruncate could not asign a length to the shmem");
		return NULL;
	}

	blockADT block = malloc(sizeof(struct blockCDT));
	if(block == NULL) return NULL;

	block -> shmid = shmid;
	block -> size = size;

	return block;

}

bufferADT attachBuffer(int shmid, char* name){
	char * mem;
	struct stat *statbuf = malloc(sizeof(struct stat));
	if(fstat(shmid, statbuf) == ERROR){
		perror("fstat could not access the block size");
		return NULL;
	}

	if((mem = mmap(NULL, statbuf->st_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0)) == (char *) ERROR){
		perror("shmat could not attach the shmem segment");
		return NULL;
	}

	free(statbuf);

	//TODO Hay que ver si el nombre del semaforo se pasar como parametro o se elige uno predeterminado
	sem_t* sem;
	if( (sem = sem_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, 0)) == SEM_FAILED){
	    if(errno == 17)
		    sem = sem_open(name, O_RDWR);
	    else{
		    perror("sem_open could not open the semaphore");
		    return NULL;
	    }
	}

	bufferADT buffer = malloc(sizeof(bufferCDT));
	if(buffer==NULL) return NULL;

	buffer->mem = mem;
	buffer->mutex = sem;
	buffer->offset = 0;

	return buffer;
}

//TODO tener cuidado si se detachea la shmem pero no el semaforo
void detachBuffer(bufferADT buffer, int shmid){
	struct stat *statbuf = malloc(sizeof(struct stat));

	if(fstat(shmid, statbuf) == ERROR){
		perror("fstat could not acces the segment size");
		return;
	}

	if(munmap((void *) buffer->mem, statbuf->st_size) == ERROR){
		perror("shmdt could not detach shmem segment");
		return;
	}
	free(statbuf);

	if(sem_close(buffer->mutex) == ERROR){
		perror("sem_close could not close semaphore");
		return;
	}
	free(buffer);
}

void destroyBlock(blockADT block){

	if(sem_unlink("LoCoco") == ERROR){
		perror("shmem could not be unlinked");
		return;
	}

	if(shm_unlink("/something") == ERROR){
		perror("sem could not be unlinked");
		return;
	}

	if(close(block->shmid) == ERROR){
		perror("shmctl could not distroy the shmem segment");
		return; 	
	}
	free(block);
}


void writeBuffer(bufferADT buffer, char* src){
	int newOffset = writeSem(buffer->mutex, src, buffer->mem + buffer->offset) + 1;
	checkOffset(newOffset,buffer);
}

void readBuffer(bufferADT buffer, char* dest){
	int newOffset = readSem(buffer->mutex, buffer->mem + buffer->offset, dest) + 1;
	checkOffset(newOffset,buffer);
}

static void checkOffset(int newOffset,bufferADT buffer){
	if(newOffset < 0 ) 
		perror("reading shmem : ");
	else buffer->offset += newOffset; 
}


int getShmid(blockADT block){
	return block->shmid;
}

int getSize(blockADT block){
	return block->size;
}

long getOffset(bufferADT buffer){
	return buffer->offset;
}
