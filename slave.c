#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


// Compile with -DSOLVER = '"path"'
#ifndef SOLVER
    #define SOLVER "./minisat"
#endif

#define RD 0 
#define WR 1
#define BUFFSIZE 256 

char * summarize( int fd );  // extern function defined in another file 
int callSolver( int * slverpipe, char * args[]); // INLINE? Or normal function? 


// TODO: add closes 
int main(int argc, char *argv[]) {

    char * args = { NULL, NULL}; 
    int slvrpipe[2];
    char buffer[BUFFSIZE];

    if ( pipe(slvrpipe) < 0 ) {
        perror("Unable to create pipe at slave.\n");
        return 1;  
    } 
    
    int i = 1; 
    while ( argv[i] ) {
        args[0] = argv[i++];
        if ( callSolver( slvrpipe, args) ) {
            perror("Unable to initialize solver.\n");
            return 1; 
        }
        wait(); 
        puts(summarize(slvrpipe[RD])); 
    }

    args[0] = buffer;      
    
    while(1) { 
        fgets(buffer, BUFFSIZE, STDIN_FILENO);
        if ( callSolver( slvrpipe, args) ) {
            perror("Unable to initialize solver.\n");
            return 1; 
        }
        puts(summarize(slvrpipe[RD])); 
    }
} 


int callSolver( int * slvrpipe, char *args[] ) { 
    pid_t pid = fork(); 
    if ( pid < 0 ) { 
        perror("Unable to fork.\n"); 
        return 1; 
    } else if ( ! pid ) { 
        dup2(slvrpipe[WR], STDOUT_FILENO);
        execvp(SOLVER, args); 
        return 1; 
    }
    return 0; 

}