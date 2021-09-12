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
#include "shmem.h"
#include <fcntl.h>
 
#define SLAVEPATH "./slave" 
#define RESPATH "results"

#define QSLAVES 5
#define VIEW_WAIT 10

#define MS 0 
#define SM 1 
#define RD 0 
#define WR 1

#define SEM_NAME "LoCoco"
#define MAXPATH 256
#define MAXQUERY 300

// Setup and initialization
int createPipes(int pipes[QSLAVES][2][2]);
int createSlaves(pid_t ids[QSLAVES] , int pipes[QSLAVES][2][2]);
void initializeSet(fd_set * set, int  fd[QSLAVES][2][2]);  

// File and result deployment
void assignPath(int * currentPathIndex , int * active, int argc , int slaveIdx, int pipes[QSLAVES][2][2], char * argv[]);
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

	blockADT shmblock = createBlock(argv[0], argc*MAXQUERY); 
	bufferADT shmbuffer = attachBuffer(getShmid(shmblock), SEM_NAME);
	printf("%d\n", getShmid(shmblock));
	fflush(stdout);
	sleep(VIEW_WAIT); 
	
	int activeSlaves = QSLAVES;
	int currentPathIndex = 1;
	
	for(int i = 0 ; i < QSLAVES ; i++)
		assignPath(&currentPathIndex, &activeSlaves, argc, i, pipes, argv);
		
	char argno[5];
	sprintf(argno,"%d",argc-1);
	writeBuffer(shmbuffer, argno);

	int resultfd;
	if ((resultfd = open(RESPATH, O_RDWR|O_CREAT, S_IRWXU)) < 0) 
		printf("Result file could not be opened or created.\n");
	
	fd_set readings;
	initializeSet(&readings, pipes);
	
	int updated;
	char queryBuffer[MAXQUERY]; 
	while(activeSlaves){
		updated = select(FD_SETSIZE, &readings, NULL, NULL, NULL); 

		// Distribute tasks to unoccupied slaves
	   	for (int i = 0 ; i < QSLAVES && updated ; i++) 
			if(FD_ISSET(pipes[i][SM][RD], &readings)) {
				updated--;	
				assignPath(&currentPathIndex, &activeSlaves, argc, i, pipes, argv);
			}

		// Read slaves' results 
		for(int i = 0 ; i < QSLAVES ; i++){
			if(FD_ISSET(pipes[i][SM][RD],&readings)){
				int header = sprintf(queryBuffer, "Process number:\t%d\n", slvids[i]);
				int readChar = read(pipes[i][SM][RD], queryBuffer + header, MAXQUERY); 
				queryBuffer[readChar+header]=0;
				deployResults(queryBuffer, shmbuffer, resultfd, readChar+header); 
			}
		} 
		initializeSet(&readings,pipes); // reset set
	}

	for (int i = 0 ; i < QSLAVES ; i++)
		waitpid(slvids[i],NULL,0);	

	detachBuffer(shmbuffer);
	destroyBlock(shmblock);
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

void assignPath(int * currentPathIndex , int * active, int argc , int slaveIdx, int pipes[QSLAVES][2][2], char * argv[]){
	char pass[50];
	if( *currentPathIndex < argc){
			int count = appendNewline(argv[*currentPathIndex],pass);
			if (write( pipes[slaveIdx][MS][WR] , pass , count) != count)
				printf("Write goes wrong\n");
			(*currentPathIndex)++;
		} else {
			close(pipes[slaveIdx][MS][WR]);
			pipes[slaveIdx][MS][WR]=-1;
			(*active)--;
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
