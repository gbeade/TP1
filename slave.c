#define _POSIX_C_SOURCE 2


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

#define BUFFSIZE 256 


char * getPath(char * path);
pid_t callSolver(char * buffer , int parserStdin );


int main(int argc, char *argv[]) {

	char buffer[BUFFSIZE];  
	FILE * parser;	
	int parserfd;
	pid_t solverpid;
	int status;

	while (getPath(buffer) != NULL) {

		 if( ( parser = popen(PARSER,"w") ) == NULL){
			perror("Unable to open the parser\n");	
			return -1;
		}

		parserfd = fileno(parser);		

		solverpid = callSolver(buffer, parserfd);
	    if(solverpid < 0 ){
			perror("Solver could not be open\n");
			return -1;
		} 

		waitpid(solverpid,&status,0);
		pclose(parser);
	}
	return 0; 
} 

char * getPath(char path[BUFFSIZE]){
	if( fgets(path, BUFFSIZE, stdin) == NULL)
		return NULL;
	int n = strlen(path);
	path[n-1]=0;	
	return path;
}

pid_t callSolver(char * buffer , int parserfd) {
	pid_t pid = fork();
	if( pid < 0 ) 
		return -1;
	else if(pid==0){
		if ( dup2(parserfd, STDOUT_FILENO) < 0) 
		       return -1;		
		execl(SOLVER, SOLVER, buffer, NULL);
		return -1; 
	}
	return pid;
}
















