#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>


#define MAXLENGTH 6

char * getPath( char * buf){
    char character ; 

    int currentPos=0;
    while ( ( character=getchar() ) != '\n' ){
        if( character == EOF)
            return NULL;

        buf[currentPos++]=character;
    	if(currentPos == MAXLENGTH){
		errno = EOVERFLOW;
		return NULL;
	}
    }

    buf[currentPos]=0;
    return buf;
}



int main(){
    char path[MAXLENGTH];
    
    while (1)
    {
        if( getPath(path)==NULL ){
		if(errno == EOVERFLOW {)
			perror("Path given is too large : ");
			return -1;
		}else return 0;
	}

	printf("Pid: %d\n", getpid());
	printf("%s\n",path);
	
        FILE * shellParse = popen("grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"","w");
	
	if(shellParse == NULL) {
		perror("popen : ");
		return -1;
	}

	int fd = fileno(shellParse);
	
	if(fd < 0 ) {
		perror("fileno : ");
		return -1;
	}




	pid_t pid = fork();
	if(pid < 0){
		perror("fork ");
		return -1;
	}


       if(pid==0){
            dup2(fd,1); //check if fails 
            execl("/usr/bin/minisat","minisat",path,NULL);
            perror("exec: ");
	}
        else{
	    int status; 
            waitpid(pid,&status,0);
	    pclose(shellParse);
        }

    }
   
}
