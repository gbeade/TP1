#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>



// Compile with -DSOLVER = '"path"'
#ifndef SOLVER
    #define SOLVER "./dummy"
#endif

#define RD 0 
#define WR 1
#define BUFFSIZE 256 

char * summarize( int fd );  // extern function defined in another file 
int callSolver( int * slverpipe, char * args[]); // INLINE? Or normal function? 


// TODO: add closes 
// TODO: solucionar args 
int main(int argc, char *argv[]) {

    char * args[] = { NULL, NULL}; 
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
        // Procesar e imprimir contenido del pipe por STDOUT
        // (...)
    }

    args[0] = buffer;      
    
    while(1) { 
        // Haltear y ponerse a leer de STDIN 
        // (...)
        if ( callSolver( slvrpipe, args) ) {
            perror("Unable to initialize solver.\n");
            return 1; 
        }
        // Procesar e imprimir contenido del pipe por STDOUT
        // (...)
    }
} 


int callSolver( int * slvrpipe, char * args[] ) { 
    pid_t pid = fork(); 
    if ( pid < 0 ) { 
        perror("Unable to fork.\n"); 
        return 1; 
    } else if ( ! pid ) { 
        dup2(slvrpipe[WR], STDOUT_FILENO);
        execvp(SOLVER, args); // CUIDADO!!!!!! No se por que no le pasa args!! 
        return 1; 
    }
    return 0; 

}
