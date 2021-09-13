// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _POSIX_C_SOURCE  200809L

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "shmem_posix.h"
#include <fcntl.h>
 
#define SLAVEPATH "./slave" 
#define RESPATH "results"

#define QSLAVES 3
#define VIEW_WAIT 5
#define QINITTASKS 3 // Qty of tasks that are initially dealed per slave

#define MS 0 
#define SM 1 
#define RD 0 
#define WR 1

#define SEM_NAME "sem"
#define SHMEM_NAME "/shmem"

#define MAXPATH 256
#define MAXQUERY 300



// Setup and initialization
int createPipes(int pipes[QSLAVES][2][2]);
int createSlaves(pid_t ids[QSLAVES] , int pipes[QSLAVES][2][2]);
void initializeSet(fd_set * set, int  fd[QSLAVES][2][2]);  

// File and result deployment
void assignPath(int * currentPathIndex , int * active, int argc , int slaveIdx, int pipes[QSLAVES][2][2], char * argv[],int workPerSlave);
void deployResults(char * results, bufferADT buffer, int resultfd, int size); 

// Ancillary 
void closeOther(int me , int fd[QSLAVES][2][2]);
int  appendNewline(char * src , char * dest);

int main(int argc, char *argv[]) {
     
	// Slave, flow direction, read|write
	int pipes[QSLAVES][2][2]; // pipes[slave_id][ MS | SM ][ RD | WR ]
	if( createPipes(pipes) < 0 ){
		perror("Pipes could not be created.\n");
		return -1;
	}
	
	pid_t slvids[QSLAVES];     
	if( createSlaves(slvids,pipes) < 0 ) {
		printf("Slaves could not be created.\n");
		return -1;
	}

	int shmfd = createBlock(SHMEM_NAME); 
	bufferADT shmbuffer = attachBuffer(shmfd, SEM_NAME);
	printf("%s\n%s\n", SHMEM_NAME, SEM_NAME);
	fflush(stdout);
	sleep(VIEW_WAIT); 

	

	int activeSlaves = QSLAVES;
	int currentPathIndex = 1;
	int workPerSlave = 2 ; 	

	for(int i = 0 ; i < QSLAVES; i++)
		assignPath(&currentPathIndex, &activeSlaves, argc, i, pipes, argv,workPerSlave);
	

	int resultfd;
	if ((resultfd = open(RESPATH, O_RDWR|O_CREAT, S_IRWXU)) < 0) 
		printf("Result file could not be opened or created.\n");
	
	fd_set readings;
	initializeSet(&readings, pipes);
	
	char queryBuffer[MAXQUERY]; 

	while(activeSlaves){

		int updated;
		updated = select(FD_SETSIZE, &readings, NULL, NULL, NULL); 

		// Distribute tasks to unoccupied slaves
	   	for (int i = 0 ; i < QSLAVES && updated ; i++) 
			if(FD_ISSET(pipes[i][SM][RD], &readings)) {
				updated--;	
				assignPath(&currentPathIndex, &activeSlaves, argc, i, pipes, argv,1);//1 per slave when reasigning.
			}

		// Read slaves' results 
		for(int i = 0 ; i < QSLAVES ; i++){
			if(FD_ISSET(pipes[i][SM][RD],&readings)){
				int readChar = read(pipes[i][SM][RD], queryBuffer, MAXQUERY * workPerSlave); 
				if( readChar == 0 ) {
					pipes[i][MS][WR]=-1;
					activeSlaves--;			
				}else{	
					queryBuffer[readChar]=0;
					deployResults(queryBuffer, shmbuffer, resultfd, readChar); 
				}
			}
		} 
		initializeSet(&readings, pipes); // reset set
	}

	writeBuffer(shmbuffer, "@");

	for (int i = 0 ; i < QSLAVES ; i++)
		waitpid(slvids[i], NULL, 0);	

	detachBuffer(shmbuffer, shmfd);
	destroyBlock(shmfd, SEM_NAME, SHMEM_NAME);
	return 0;
}


void initializeSet(fd_set *  set, int  fd[QSLAVES][2][2]){
	FD_ZERO(set);
	for(int i = 0 ; i < QSLAVES ; i++) {
		if(fd[i][MS][WR] > 0) 
			FD_SET(fd[i][SM][RD], set);		
	} 
}

int createPipes( int pipes[QSLAVES][2][2] ){
	for (int i=0; i<QSLAVES; i++) 
		for (int j=0; j<2; j++)
			if (pipe(pipes[i][j]) < 0) 
				return -1;   
	return 0;
}

int createSlaves(pid_t slvids[QSLAVES] , int pipes[QSLAVES][2][2]){
	char * args[] = {SLAVEPATH,NULL}; 
	for (int i=0; i<QSLAVES; i++) {
		slvids[i] = fork(); 
		if (slvids[i] == -1) { 
			return -1; 
		} else if (!slvids[i]) {
			closeOther(i,pipes);
			close(pipes[i][MS][WR]); 
			close(pipes[i][SM][RD]); 
			if ( dup2(pipes[i][MS][RD], STDIN_FILENO) < 0  || dup2(pipes[i][SM][WR], STDOUT_FILENO) < 0 ) 
				return -1;
			execvp(SLAVEPATH, args); 
		} 
		close(pipes[i][MS][RD]); 
		close(pipes[i][SM][WR]); 
	}
	return 0;
}

void assignPath(int * currentPathIndex , int * active, int argc , int slaveIdx, int pipes[QSLAVES][2][2], char * argv[] , int workPerSlave ){
	char pass[MAXPATH];
	for(int i = 0 ; i < workPerSlave ; i++){ 
		if( *currentPathIndex < argc){
			int count = appendNewline(argv[*currentPathIndex],pass);
			if (write( pipes[slaveIdx][MS][WR] , pass , count) != count)
				printf("Write goes wrong\n");
			(*currentPathIndex)++;
		} else 
			close(pipes[slaveIdx][MS][WR]);
	}
}

void deployResults(char * results, bufferADT buffer, int resultfd, int size) {
	writeBuffer(buffer, results);
	write(resultfd, results, size);
}

int appendNewline(char * src , char * dest){
	int i = 0 ; 
	while (src[i]) {
		dest[i]=src[i];
		i++;	
	}
	dest[i++]='\n';
	dest[i]=0;
	return i ;
}

void closeOther(int me , int fd[QSLAVES][2][2]){
	for (int i = 0 ; i < me ; i++){
		close(fd[i][SM][RD]);
		close(fd[i][SM][WR]);
		close(fd[i][MS][RD]);
		close(fd[i][MS][WR]);
	}	
}
