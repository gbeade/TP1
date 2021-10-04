// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _XOPEN_SOURCE 500
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



// TODO definilo en el makefile si no te gusta este tamaño 
#ifndef BLOCKSIZE
	#define BLOCKSIZE 4096
#endif

//TODO falta hacer las validaciones con pvs_studio, cambiar sprintf para que no de warnings

static int checkOffset(int newOffset,bufferADT buffer);


typedef struct bufferCDT{
	char * mem;
	sem_t * mutex;
	long offset;
} bufferCDT;


int createBlock(const char *shmName){

	int shmfd= shm_open(shmName, O_CREAT | O_RDWR, 0777); //El problema con esto es que shm_open toma un path absoluto y no uno relativo
	if(shmfd== ERROR){
		perror("shm_open could not create the shmem segment");
		return 1;
	}

	if(ftruncate(shmfd, BLOCKSIZE) == ERROR){
		perror("ftruncate could not asign a length to the shmem");
		return 1;
	}

	return shmfd;

}

bufferADT attachBuffer(int shmfd, char* semName){
	char * mem;

	if((mem = mmap(NULL, BLOCKSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == (char *) ERROR){
		perror("mmap could not attach the shmem segment");
		return NULL;
	}


	//TODO Hay que ver si el nombre del semaforo se pasar como parametro o se elige uno predeterminado
	sem_t* sem;
	if( (sem = sem_open(semName, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, 0)) == SEM_FAILED){
	    if(errno == 17)
		    sem = sem_open(semName, O_RDWR);
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

int detachBuffer(bufferADT buffer, int shmfd){

	if( munmap((void *) buffer->mem , BLOCKSIZE) == ERROR){
		perror("munmap could not detach shmem segment");
		return -1;
	}

	if( sem_close(buffer->mutex) == ERROR ){
		perror("sem_close could not close semaphore");
		return -1;
	}

	free(buffer);
	return 0;
}

int destroyBlock(int shmfd, char *semName, char *shmName){

	if(sem_unlink(semName) == ERROR){
		perror("shmem could not be unlinked");
		return -1;
	}

	if(shm_unlink(shmName) == ERROR){
		perror("sem could not be unlinked");
		return -1;
	}

	if(close(shmfd) == ERROR){
		perror("shmfd could not be closed");
		return -1; 	
	}
	return 0;
}


int writeBuffer(bufferADT buffer, char* src){
	int newOffset = writeSem(buffer->mutex, src, buffer->mem + buffer->offset) + 1;
	return checkOffset(newOffset,buffer);
}

int readBuffer(bufferADT buffer, char* dest){
	int newOffset = readSem(buffer->mutex, buffer->mem + buffer->offset, dest) + 1;
	return checkOffset(newOffset,buffer);
}

static int checkOffset(int newOffset, bufferADT buffer){
	if(newOffset < 0 ){	
		perror("reading shmem : ");
		return -1;
	}
	else buffer->offset += newOffset; 
	return newOffset;
}

int securityUnlink(char *semName, char *shmName){
	if(sem_unlink(semName) == ERROR && errno!=2){
		perror("shmem could not be unlinked");
		return -1;
	}

	if(shm_unlink(shmName) == ERROR && errno!=2){
		perror("sem could not be unlinked");
		return -1;
	}
	return 0;
}




