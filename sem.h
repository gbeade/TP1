#ifndef SEM_H
#define SEM_H

#define _SVID_SOURCE 1
#include <semaphore.h>



int writeSem(sem_t* sem, char* src, char* dest);

int readSem(sem_t* sem, char* src, char* dest);

#endif
