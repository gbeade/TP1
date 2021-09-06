#include <shmem.h>
#include <semaphore.h>

#define ERROR (-1)
#define NON_ZERO 1

struct bufferCDT {
	int shmid;
	key_t key;
	size_t size;

	sem_t empty;
	sem_t full;
	sem_t mutex;
}


bufferADT createBuffer(const char *pathname, size_t size){
	key_t key = ftok(pathname);
	if(key == ERROR){
		perror("ftok");
		return NULL;
	}

	int shmid = shmget(key, size, IPC_CREAT | OBJ_PERMS); //Hay que mirar el tema de los permisos, use los mismos que los de la clase de horacio
	if(shmid == ERROR){
		perror("shget");
		return NULL;
	}

	bufferADT buffer = malloc(sizeof(struct bufferCDT));
	if(buffer == NULL) return NULL;

	buffer -> shmid = shmid;
	buffer -> key = key;
	buffer -> size = size;

	//Me parece que deberiamos usar semaforos con nombre para poder compartirlos entre procesos, esto se hace con sem_open
	if(sem_init(&buffer->empty, NON_ZERO, size) == ERROR){
	       perror("initializing empty semaphore");
	       return NULL;
	}	       
	
	if(sem_init(&buffer->full, NON_ZERO, 0) == ERROR){
	       perror("initializing full semaphore");
	       return NULL;
	}	       

	if(sem_init(&buffer->mutex, NON_ZERO, 1) == ERROR){
	       perror("initializing mutex semaphore");
	       return NULL;
	}	       
}


char* attachBuffer(bufferADT buffer){
	char * ret;
	if((ret = shmat(buffer -> shmid, NULL, 0)) == (void *) ERROR){
		perror("shmat");
		return NULL;
	}
	return ret; //shmat devuelve la direccion virtual en la que se enctentra la seccion de shmem, el problema que trae esto es que para cada proceso el buffer debera tener un direccion de memoria virtual diferente
}

void detachShmem(bufferADT buffer){
	if(shmdt() == ERROR) perror("shmdt");
}

void destroyShmem(bufferADT buffer){
	shmctl(buffer->shmid, IPC_RMID, NULL);

	//sem_destroy es para semaforos sin nombre, para semaforos con nombre se usa sem_unlink
	if(sem_destroy(&buffer->full) == ERROR){
	       perror("destroying full semaphore");
	       return NULL;
	}

	if(sem_destroy(&buffer->empty) == ERROR){
	       perror("destroying empty semaphore");
	       return NULL;
	}

	if(sem_destroy(&buffer->mutex) == ERROR){
	       perror("destroying mutex semaphore");
	       return NULL;
	}
	free(buffer);
}

//La implementacion de la sincronizacion de los procesos se podria abstraer a un sem.c
//Si usamos entradas de tamanio fijo para el buffer podriamos asignarle a la shmem un fd con shm_open y usar las syscalls read y write

void writeBuffer(bufferADT buffer){
	sem_wait(&buffer->empty);
	sem_wait(&buffer->mutex);
	//write
	sem_post(&buffer->mutex);
	sem_post(&buffer->full);
}

void* readBuffer(bufferADT buffer){
	sem_wait(&buffer->full);
	sem_wait(&buffer->mutex);
	//read
	sem_post(&buffer->mutex);
	sem_post(&buffer->empty);
}

int getShmid(bufferADT buffer){
	return buffer -> shmid;
}

key_t getKey(bufferADT buffer){
	return buffer -> key;
}

size_t getSize(bufferADT buffer){
	return buffer -> size;
}

