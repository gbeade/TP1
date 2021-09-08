#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
// #include <sys/wait.h>
#include <stdlib.h>


// Compile with -DSOLVER = '"path"'
#ifndef SOLVER
    #define SOLVER "./dummy"
#endif

// Compile with -DFETCHER = '"path"'
#ifndef FETCHER
    #define FETCHER 9 
#endif 

#define RD 0 
#define WR 1
#define BUFFSIZE 256 

static inline int callSolver( int * slverpipe, char * args[]); // INLINE? Or normal function? 
static int getlinefd(int fd, char * buffer, unsigned int buffsize); // getline for file descriptors

// TODOS in priority
// *****************************************************
// TODO: Make parent wait for child to finish before getting the line
// TODO: Add Fetcher (egrep from Ariel). popen + grep -o -e "Number of.*[0-9]\+" -e "CPU time.*" -e !! KEY ! 
// TODO: add closes 
// TODO: error library - index errors 


int main(int argc, char *argv[]) {

    char buffer[BUFFSIZE]; 
    char * args[2] = {buffer, NULL}; 
    int slvrpipe[2];


    if ( pipe(slvrpipe) < 0 ) {
        perror("Unable to create pipe at slave.\n");
        return 1;  
    } 
     
    while ( 1 ) { //Decidir como cortar el loop
        fgets(buffer, BUFFSIZE, stdin);
        if ( callSolver( slvrpipe, args) ) {
            perror("Unable to initialize solver.\n");
            return 1; 
        }
        // LO UNICO QUE FALTA ES EL WAIT 
        getlinefd(slvrpipe[RD], buffer, BUFFSIZE); 
        printf("%s", buffer); 
    }
} 


static inline int callSolver( int * slvrpipe, char * args[] ) {  
    pid_t pid = fork(); 
    if ( pid < 0 ) { 
        perror("Unable to fork.\n"); 
        return 1; 
    } else if ( 0 == pid ) { 
        dup2(slvrpipe[WR], STDOUT_FILENO);
        execvp(SOLVER, args);
        return 1; 
    }
    return 0; 
}


static int getlinefd(int fd, char * buffer, unsigned int buffsize) {
    int i = 0;
    while ( i<buffsize && read(fd, buffer+i, 1) > 0 &&  ( !i || buffer[i-1] != '\n') ) i ++;  
    buffer[i] = '\0'; 
    return 0; 
}



// Comentario interesante: file descirptor (int) vs file pointer (FILE * )
// You can create a FILE stream out of a filedescriptor with fdopen.
 
// The pipe function returns is pair of int file descriptors, 
// not FILE ones. That means that you can use read, write, or close on them but neither fgetc, nor feof.