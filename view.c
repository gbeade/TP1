// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "shmem_posix.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFSIZE 256

int main(int argc , char * argv[]){
	
	char * shmname = NULL, * semname = NULL; 
	ssize_t n1 = 0, n2 = 0; 
	size_t buffsize = BUFFSIZE; 

	if (argc == 1) { 
		n1 = getline(&shmname, &buffsize, stdin); 
		n2 = getline(&semname, &buffsize, stdin);
		shmname[n1-1] = '\0'; 
		semname[n2-1] = '\0';  
	} else if (argc == 3) {
		shmname = argv[1]; 
		semname = argv[2]; 	
	} else {
		return 1; 
	}

	int shmfd = createBlock(shmname);  // Salva: tamaño va a usar 
	bufferADT buffer = attachBuffer(shmfd, semname);

	char file_no_str[6] = {0}; 
	readBuffer(buffer, file_no_str);
	int files = atoi(file_no_str); 	

	char resultBuffer[BUFFSIZE];
	while (files){
		readBuffer(buffer, resultBuffer);
		printf("%s\n", resultBuffer);
		files--;
	}

	detachBuffer(buffer, shmfd);

	if (n1) free(shmname); 
	if (n2)	free(semname); 
	
	return 0;
	
}


