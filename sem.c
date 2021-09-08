
#include <sem.h>
#include <stdio.h>

int writeSem(sem_t* sem, char* src, char* dest){
        sem_post(sem);
        int offset = sprintf(dest, src);
	if(offset < 0) perror("sprintf failed at writeSem");
	return offset;
}

int readSem(sem_t* sem, char* src, char* dest){
        sem_wait(sem);
        int offset =  sprintf(dest, src);
	if(offset < 0) perror("sprintf failed at readSem");
	return offset;
}

