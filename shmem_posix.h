#ifndef SHMEM_H
#define SHMEM_H

#define _SVID_SOURCE 1
#include "sem.h"

#define ERROR (-1)
#define NON_ZERO 1

typedef struct bufferCDT * bufferADT;

int createBlock(const char * shmName);

bufferADT attachBuffer(int fd, char* semName);

int detachBuffer(bufferADT buffer, int fd);

int destroyBlock(int shmdi, char* semName, char* shmName);

int readBuffer(bufferADT buffer, char* dest);

int writeBuffer(bufferADT buffer, char* src);

int securityUnlink(char* shmName, char* semName);

#endif
