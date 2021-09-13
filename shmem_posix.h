#ifndef SHMEM_H
#define SHMEM_H

#define _SVID_SOURCE 1
#include "sem.h"

#define ERROR (-1)
#define NON_ZERO 1

typedef struct bufferCDT * bufferADT;

int createBlock(const char * shmName);

bufferADT attachBuffer(int shmid, char* semName);

void detachBuffer(bufferADT buffer, int shmid);

void destroyBlock(int shmdi, char* semName, char* shmName);

void readBuffer(bufferADT buffer, char* dest);

void writeBuffer(bufferADT buffer, char* src);

#endif
