#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>


// Compile with -DSOLVER = '"solver_path"'
#ifndef SOLVER
    #define SOLVER "/usr/bin/minisat"
#endif

// Compile with -DPARSER ='"cmd_parser"'
#ifndef PARSER
  #define PARSER "grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\""
#endif 

#define RD 0 
#define WR 1
#define BUFFSIZE 256 

// TODO: error library - index errors 
int main(int argc, char *argv[]) {

    char buffer[BUFFSIZE]; 

    while (1) {
        // El problema con usar getline es que arma un string en el heap, que despues hay que liberar. No esta bueno, es ineficiente. 
        // Por otro lado, getline devuelve el # del buffer final; fgets no devuelve la # de caracteres repetidos
        // Por otro lado, fgets puede trabajar con un buffer en el stack
        fgets(buffer, BUFFSIZE, stdin);
        int n = strlen(buffer); //INEFICIENTE! Hay que encontrar una manera de getline sin \n pero en el stack
        buffer[n-1] = '\0';  

        FILE * parser = popen(PARSER,"w");
        if( parser == NULL) {
		    perror("Unable to popen the grep.\n");
		    return -1;
	    }
        
        int fd = fileno(parser);
        if(fd < 0 ) {
		    perror("Unable to fileno\n");
		    return -1;
	    }

	    pid_t pid = fork();
        if ( pid < 0 ){
		    perror("Unable to fork\n");
		    return -1;
	    } else if(pid==0){
            dup2(fd, STDOUT_FILENO); //check if fails 
            execl(SOLVER, SOLVER, buffer, NULL);
            perror("Unable to exec solver\n");
            return -1; 
	    } else{
	        int status; 
            waitpid(pid,&status,0);
	        pclose(parser);
        }
    }
    return 0; 
} 
