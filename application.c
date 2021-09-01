#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

// TODO: create share memory. Application prints the name for said buffer

#define QSLAVES 5 

int main(int argc, char *argv[]) {

    pid_t spids[QSLAVES];     
    char * args[] = {".", NULL}; 
    
    for ( int i=0; i<QSLAVES; i++ ) {
        spids[i] = fork(); 
        if ( spids[i] < 0 ) { 
            perror("Error in fork\n"); 
            return 1; 
        } else if ( !spids[i] ) { // TODO: Este if se corre un poco redundante. Puede evitarse? 
            execvp("./dummy", args); 
        }
    }
    

}

