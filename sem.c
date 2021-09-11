#include "sem.h"
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>

static int copy(char * src , char * dest);


int writeSem(sem_t* sem, char* src, char* dest){
        sem_post(sem);
	return copy(src,dest);
}

int readSem(sem_t* sem, char* src, char* dest){
        sem_wait(sem);
	return copy(src,dest);
	


}

static int copy(char * src , char * dest){
	int offset = sprintf(dest,"%s",src);
	if(offset < 0 ) return -1;
	return offset;
}

