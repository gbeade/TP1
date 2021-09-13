#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "shmem_posix.h"
#include <stdlib.h>
#include <string.h>

#define SEM_NAME "sem"
#define SHMEM_NAME "/shmem"

int main(int argc , char * argv[]){
	
	char keyString[5];
	createBlock(SHMEM_NAME, 20*300);
	if(argc != 2 ) { //hay que leerlo de entrada estandar.
		char * line;
		size_t dim = 5 ; 
		getline(&line,&dim,stdin);
		strcpy(keyString,line);
	}else{
		strcpy(keyString , argv[1]);
	
	}
	int key = strtol(keyString,NULL,10);



	bufferADT buffer = attachBuffer(key,SEM_NAME);

	char argno[5];
	readBuffer(buffer,argno);
 	int files = strtol(argno,NULL,10);

	char toPrint[300];
	while(files){
		readBuffer(buffer,toPrint);
		printf("%s\n",toPrint);
		files--;
	}

	detachBuffer(buffer, key);
}


