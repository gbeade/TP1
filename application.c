#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

 
#define MS 0 
#define SM 1 
#define RD 0 
#define WR 1

#define MAXPATH 256
#define QSLAVES 2

void closeOther(int me , int fd[QSLAVES][2][2]);
int  convert( char * src , char * dest);
void initializeSet(fd_set * set, int  fd[QSLAVES][2][2]);  

// TODO: create share memory. Application prints the name for said buffer
// TODO: al principio, enviar dos o mas tareas inicialmente a cada slave
// TODO: 
int main(int argc, char *argv[]) {
  
	   
	// Slave, flow direction, read|write
	int pipes[QSLAVES][2][2]; // pipes[slave_id][ MS | SM ][ RD | WR ]
	
	for (int i=0; i<QSLAVES; i++) 
		for (int j=0; j<2; j++)
			if (pipe(pipes[i][j]) < 0) {
				perror("Unable to create pipe at master\n"); 
				return 1; 
			}  
   
	
	pid_t spids[QSLAVES];     
	char * args[] = {"./slave",NULL}; 
	
	for (int i=0; i<QSLAVES; i++) {
		spids[i] = fork(); 
		if (spids[i] == -1) { 
			perror("Unable to fork\n"); 
			return 1; 
		} else if (!spids[i]) {
			closeOther(i,pipes);
			close(pipes[i][MS][WR]); 
			close(pipes[i][SM][RD]); 
			dup2(pipes[i][MS][RD], STDIN_FILENO);
			dup2(pipes[i][SM][WR], STDOUT_FILENO);
			execvp("./slave", args); 
		} 
		close(pipes[i][MS][RD]); 
		close(pipes[i][SM][WR]); 
	}

	sleep(15);


	// First shipment of files to slaves 
	int active = QSLAVES;
	int currentPathIndex = 1;
	char pass[50];
	for(int i = 0 ; i < QSLAVES ; i++){
		if(currentPathIndex < argc){
			int count = convert(argv[currentPathIndex],pass);
			if (write(pipes[i][MS][WR] , pass , count) != count)
				printf("Write goes wrong\n");
			currentPathIndex++;
		} else {
			close(pipes[i][MS][WR]);
			pipes[i][MS][WR]=-1;
			active--;
		}
	}


	// Recurrent shipment of files to slaves 
	fd_set readings;
	initializeSet(&readings, pipes);

	int resfd = open("resultado", O_RDWR|O_CREAT, S_IRWXU);

	int updated;
	while(active){
		updated = select(FD_SETSIZE, &readings, NULL, NULL, NULL); 

	   	for (int i = 0 ; i < QSLAVES && updated ; i++) {
			if(FD_ISSET(pipes[i][SM][RD], &readings)) {
				updated--;		
				if(currentPathIndex < argc){
					int count = convert(argv[currentPathIndex],pass);
					write(pipes[i][MS][WR],pass,count);					
					currentPathIndex++;
				} else { 
					close(pipes[i][MS][WR]);
					pipes[i][MS][WR]=-1;
					active--;
				}
			}
		}

		for(int i = 0 ; i < QSLAVES ; i++){
			if(FD_ISSET(pipes[i][SM][RD],&readings)){
				char tr[300];
				int readC = read(pipes[i][SM][RD],tr,300);
				dprintf(resfd,"Process number:\t%d\n",spids[i]);
				write(resfd,tr,readC);
			}
		} 
		initializeSet(&readings,pipes);
	}

	for (int i = 0 ; i < QSLAVES ; i++){
		waitpid(spids[i],NULL,0);
	}
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

int convert(char * src , char * dest) {
	int i = 0 ; 
	while (src[i]) {
		dest[i]=src[i];
		i++;	
	}
	dest[i++]='\n';
	dest[i]=0;
	return i ;
}
