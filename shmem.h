

typedef struct bufferCDT * bufferADT;

bufferADT createBuffer(const char * pathname, size_t len, size_t entrySize);

void attachBuffer(bufferADT buffer);

void detachBuffer(bufferADT buffer);

void destroyBuffer(bufferADT buffer);

int getShmid(bufferADT buffer);

key_t getKey(bufferADT buffer);

size_t getSize(bufferADT buffer);

//Implemento el sistema de semafors directamente aca para tener una base, este se puede abstraer de la shmem implementando un sistema prod/cons generico

void * readBuffer(bufferADT buffer);

void writeBuffer(bufferADT buffer);

