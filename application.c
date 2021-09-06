#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

 
#define MS 0 
#define SM 1 
#define RD 0 
#define WR 1

// TODO: create share memory. Application prints the name for said buffer

#define QSLAVES 5 

int main(int argc, char *argv[]) {
    
    // Slave, flow direction, read|write
    int pipes[QSLAVES][2][2]; // pipes[slave_id][ MS | SM ][ RD | WR ]
    
    for (int i=0; i<QSLAVES; i++) 
        for (int j=0; j<2; j++)
            if ( pipe(pipes[i][j]) < 0 ) {
                perror("Unable to create pipe at master\n"); 
                return 1; 
            }  
    
    
    pid_t spids[QSLAVES];     
    char * args[] = {NULL}; 
    
    for (int i=0; i<QSLAVES; i++) {
        spids[i] = fork(); 
        if ( spids[i] < 0 ) { 
            perror("Unable to fork\n"); 
            return 1; 
        } else if ( !spids[i] ) {
            close(pipes[i][MS][WR]); 
            close(pipes[i][SM][RD]); 
            dup2(pipes[i][MS][RD], STDIN_FILENO);
            dup2(pipes[i][SM][WR], STDOUT_FILENO);
            execvp("./slave", args); 
        } 
        close(pipes[i][MS][RD]); 
        close(pipes[i][SM][WR]); 
    }
}

