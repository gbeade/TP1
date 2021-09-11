#include "sem.h"
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

int writeSem(sem_t* sem, char* src, char* dest){
        sem_post(sem);
        int offset = sprintf(dest, src);
	if(offset < 0) perror("sprintf failed at writeSem");
	return offset;
}

int readSem(sem_t* sem, char* src, char* dest){
        sem_wait(sem);
        int offset = sprintf(dest,"%s", src);
	if(offset < 0) perror("sprintf failed at readSem");
	return offset;
}

//algo asi
int aux(char * src , char * dest){
	int offset = sprintf(dest,"%s",src);
	if(offset < 0 ) return -1;
	return offset;
}

