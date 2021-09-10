#include "shmem.h"
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

//TODO falta hacer las validaciones con pvs_studio, cambiar sprintf para que no de warnings

typedef struct blockCDT{ 
	int shmid;
	key_t key;
	int size;
} blockCDT;

typedef struct bufferCDT{
	char * mem;
	sem_t * mutex;
	long offset;
} bufferCDT;


blockADT createBlock(const char *pathname, pid_t pid, int size){
	key_t key = ftok(pathname, pid);
	if(key == ERROR){
		perror("ftok could not create the key");
		return NULL;
	}

	int shmid = shmget(key, size, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | IPC_CREAT); //TODO Hay que mirar el tema de los permisos
	if(shmid == ERROR){
		perror("shget could not create the shmem segment");
		return NULL;
	}

	blockADT block = malloc(sizeof(struct blockCDT));
	if(block == NULL) return NULL;

	block -> shmid = shmid;
	block -> key = key;
	block -> size = size;

	return block;

}

bufferADT attachBuffer(int shmid, char* name){
	char * mem;
	if((mem = shmat(shmid, NULL, 0)) == (char *) ERROR){
		perror("shmat could not attach the shmem segment");
		return NULL;
	}

	//TODO Hay que ver si el nombre del semaforo se pasar como parametro o se elige uno predeterminado
	sem_t* sem;
	if( (sem = sem_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, 0)) == SEM_FAILED){
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
void detachBuffer(bufferADT buffer){
	if(shmdt((void *) buffer->mem) == ERROR){
		perror("shmdt could not detach shmem segment");
		return;
	}
	if(sem_close(buffer->mutex) == ERROR){
		perror("sem_close could not close semaphore");
		return;
	}
	free(buffer);
}

void destroyBlock(blockADT block){
	if(shmctl(block->shmid, IPC_RMID, NULL) == ERROR){
		perror("shmctl could not distroy the shmem segment");
		return;
	}

	free(block);
}

void writeBuffer(bufferADT buffer, char* src){
	buffer->offset += writeSem(buffer->mutex, src, buffer->mem + buffer->offset) + 1;
}

void readBuffer(bufferADT buffer, char* dest){
	buffer->offset += readSem(buffer->mutex, buffer->mem + buffer->offset, dest) + 1;
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

