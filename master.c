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
#define MS 0 
#define SM 1 
#define RD 0 
#define WR 1

#define SEM_NAME "LoCoco"
#define MAXPATH 256
#define QSLAVES 5
#define MAXQUERY 300
#define WAITINGVIEW 10



int  changeFormat( char * src , char * dest);
void initializeSet(fd_set * set, int  fd[QSLAVES][2][2]);  

int createPipes( int pipes[QSLAVES][2][2] );
int createSlaves(pid_t ids[QSLAVES] , int pipes[QSLAVES][2][2]);
void closeOther(int me , int fd[QSLAVES][2][2]);
void assignPath(int * currentPathIndex , int * active, int argc , int slaveIdx, int pipes[QSLAVES][2][2],char * argv[]);
void sendFilesNo(bufferADT buffer , int argc);
void waitSlaves(pid_t slvids[QSLAVES]);
void WriteResults(int slvids[QSLAVES] , int pipes[QSLAVES][2][2] , int * currentPathIndex , int * active , bufferADT buffer , int Resultadofd , int argvc,char * argv[]);
void deployResults(int ReadingPipe , int slvids[QSLAVES], bufferADT buffer , int Resultadofd, int idx);

int main(int argc, char *argv[]) {
  
	   
	// Slave, flow direction, read|write
	int pipes[QSLAVES][2][2]; // pipes[slave_id][ MS | SM ][ RD | WR ]
	
	if( createPipes(pipes) < 0 ){
		printf("Pipes could not be created");
		return -1;
	}
	
	pid_t slvids[QSLAVES];     
	if( createSlaves(slvids,pipes) < 0 ) {
		printf("Slaves could not be created");
		return -1;
	}
	

	int activeSlaves = QSLAVES;
	int currentPathIndex = 1;
	
	for(int i = 0 ; i < QSLAVES ; i++)
		assignPath(&currentPathIndex ,&activeSlaves,argc , i , pipes,argv);
		


	blockADT block = createBlock(argv[0], argc*MAXQUERY); 
	bufferADT buffer = attachBuffer(getShmid(block), SEM_NAME);
	printf("%d\n", getShmid(block));
	sendFilesNo(buffer,argc);
	sleep(WAITINGVIEW);

	int Resultadofd;
	if( ( Resultadofd = open("resultado", O_RDWR|O_CREAT, S_IRWXU) ) < 0 )  {
		printf("Resultado file could be created");
	}

    	WriteResults(slvids , pipes ,  &currentPathIndex ,  &activeSlaves , buffer ,  Resultadofd,argc,argv);

	waitSlaves(slvids);
	
	detachBuffer(buffer);

	destroyBlock(block);

}


void initializeSet(fd_set *  set, int  fd[QSLAVES][2][2]){
	FD_ZERO(set);
	for(int i = 0 ; i < QSLAVES ; i++) {
		if(fd[i][MS][WR] > 0) 
			FD_SET(fd[i][SM][RD], set);		
	} 
}

void closeOther(int me , int fd[QSLAVES][2][2]){
	for (int i = 0 ; i < me ; i++){
		close(fd[i][SM][RD]);
		close(fd[i][SM][WR]);
		close(fd[i][MS][RD]);
		close(fd[i][MS][WR]);
	}	
}

int changeFormat(char * src , char * dest){
	int i = 0 ; 
	while (src[i]) {
		dest[i]=src[i];
		i++;	
	}
	dest[i++]='\n';
	dest[i]=0;
	return i ;
}


int createPipes( int pipes[QSLAVES][2][2] ){
	for (int i=0; i<QSLAVES; i++) 
		for (int j=0; j<2; j++)
			if (pipe(pipes[i][j]) < 0) {
				perror("Unable to create pipe at master\n"); 
				return -1; 
			}  
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
			int count = changeFormat(argv[*currentPathIndex],pass);
			if (write( pipes[slaveIdx][MS][WR] , pass , count) != count)
				printf("Write goes wrong\n");
			(*currentPathIndex)++;
		} else {
			close(pipes[slaveIdx][MS][WR]);
			pipes[slaveIdx][MS][WR]=-1;
			(*active)--;
		}


}


void sendFilesNo(bufferADT buffer,int argc){
	char argno[5];
	sprintf(argno,"%d",argc-1);
	writeBuffer(buffer,argno);
}

void waitSlaves(pid_t slvids[QSLAVES]){
	for (int i = 0 ; i < QSLAVES ; i++)
		waitpid(slvids[i],NULL,0);	
}


void WriteResults(int slvids[QSLAVES], int pipes[QSLAVES][2][2],int * currentPathIndex,int * active , bufferADT buffer , int Resultadofd,int argc,char * argv[]){
	fd_set readings;
	initializeSet(&readings, pipes);
	int updated;
	while(*active){
		updated = select(FD_SETSIZE, &readings, NULL, NULL, NULL); 

	   	for (int i = 0 ; i < QSLAVES && updated ; i++) 
			if(FD_ISSET(pipes[i][SM][RD], &readings)) {
				updated--;	
				assignPath(currentPathIndex ,active,argc , i , pipes,argv);
			}
		

		for(int i = 0 ; i < QSLAVES ; i++){
			if(FD_ISSET(pipes[i][SM][RD],&readings)){
				deployResults(pipes[i][SM][RD],slvids,buffer,Resultadofd,i);
			}
		} 

		initializeSet(&readings,pipes);
	}


}


void deployResults(int ReadingPipe , int slvids[QSLAVES], bufferADT buffer , int Resultadofd,int idx){
	char InterBuffer[MAXQUERY];
	int offset = sprintf(InterBuffer, "Process number:\t%d\n", slvids[idx]);
	int readChar = read(ReadingPipe,InterBuffer + offset,MAXQUERY); 
	InterBuffer[readChar+offset]=0;
	writeBuffer(buffer, InterBuffer);
	write(Resultadofd,InterBuffer,readChar + offset);


}
