#ifndef SHMEM_H
#define SHMEM_H

#define _SVID_SOURCE 1
#include "sem.h"

#define ERROR (-1)
#define NON_ZERO 1

typedef struct blockCDT * blockADT;

typedef struct bufferCDT * bufferADT;

blockADT createBlock(const char * pathname, int size);

bufferADT attachBuffer(int shmid, char* name);

void detachBuffer(bufferADT buffer, int shmid);

void destroyBlock(blockADT block);

int getShmid(blockADT block);

int getSize(blockADT block);

long getOffset(bufferADT buffer);

//Implemento el sistema de semafors directamente aca para tener una base, este se puede abstraer de la shmem implementando un sistema prod/cons generico

void readBuffer(bufferADT buffer, char* dest);

void writeBuffer(bufferADT buffer, char* src);

#endif
