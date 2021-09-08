
#define ERROR (-1)
#define NON_ZERO 1

typedef struct blockCDT * blockADT;

typedef struct bufferCDT * bufferADT;

blockADT createBlock(const char * pathname, pid_t pid, size_t size);

bufferADT attachBuffer(int shmid, char* name);

void detachBuffer(bufferADT buffer);

void destroyBlock(blockADT block);

int getShmid(blockADT block);

size_t getSize(blockADT block);

long getOffset(bufferADT buffer);

//Implemento el sistema de semafors directamente aca para tener una base, este se puede abstraer de la shmem implementando un sistema prod/cons generico

void readBuffer(bufferADT buffer, char* dest);

void writeBuffer(bufferADT buffer, char* src);

