#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int numAddresses;

typedef struct queue {
    int front;
    int rear;
    int *queue;
    int size;
    int capacity;
} queue_t;

typedef struct TLB {
    int **TLB;
    int numEntries;
    int capacity;
} TLB_t;


int *readAddresses(char *file, int capacity){
    int *addresses = malloc(sizeof(int) * capacity);
    int count = 0;
    int currAddress; 

    FILE *fptr = fopen(file, "r");

    while(fscanf(fptr, "%d", &currAddress) == 1){
        if(count >= capacity){
            capacity *= 2;
            addresses = realloc(addresses, sizeof(int) * capacity);
        }
        addresses[count] = currAddress; 
        count++;
    }

    numAddresses = count; 
    fclose(fptr);
    return addresses;
}

int getOffset(int logical){
    return logical & 0x000000FF;
}

int findPageNumber(int logical){
    return (logical & 0x0000FF00) >> 8;
}

queue_t *initializeQueue(int capacity){
    queue_t *new_queue = malloc(sizeof(queue_t));

    if(!new_queue){
        return NULL;
    }

    new_queue->queue = malloc(sizeof(int) * capacity);
    new_queue->capacity = capacity;
    new_queue->front = 0;
    new_queue->rear = 0;
    new_queue->size = 0;

    return new_queue;
}

bool isEmpty(queue_t *queue){
    printf("Queue size %d\n", queue->size);
    return queue->size == 0;
}

queue_t *initFreeFrames(queue_t *queue){
    for(int i = 0; i < queue->capacity; i++){
        queue->queue[i] = i;
        queue->size++;
    }
    queue->rear = queue->capacity - 1;
    return queue;
}

int findFreeFrame(queue_t *queue){
    if(isEmpty(queue)){
        return -1;
    }

    int freeFrame = queue->queue[queue->front];
    queue->size--;

    if(queue->size == 0){
        queue->front = 0;
        queue->rear = 0;
    } else {
        queue->front = (queue->front + 1) % queue->capacity;
    }

    return freeFrame;
}

TLB_t *initializeTLB(int capacity){
    TLB_t *newTLB = malloc(sizeof(TLB_t));
    
    if(!newTLB){
        return NULL;
    }


    newTLB->TLB = malloc(sizeof(int *) * capacity);
    
    if(!newTLB->TLB){
        return NULL; 
    }

    for(int i = 0; i < capacity; i++){
        newTLB->TLB[i] = malloc(sizeof(int)* 2);
        if(!newTLB->TLB[i]){
            return NULL;
        }
    }

    newTLB->capacity = capacity;
    newTLB->numEntries = 0;

}



int *initializePageTable(int capacity){

    int *pageTable = malloc(sizeof(int) * capacity);
    for(int i = 0; i < capacity; i++){
        pageTable[i] = -1;
    }

    return pageTable; 
}

void updatePageTable(int *pageTable, int freeFrame, int pageNumber){
    pageTable[pageNumber] = freeFrame; 
}

int findFrame(int *pageTable, int pageNumber){
    return pageTable[pageNumber];
}

void printPageTable(int *pageTable, int capacity) {
    printf("\n==================== PAGE TABLE ====================\n");
    printf("|  Page Number  |  Frame Number  |\n");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < capacity; i++) {
        if (pageTable[i] != -1)
            printf("|     %3d       |      %3d       |\n", i, pageTable[i]);
    }

    printf("====================================================\n\n");
}


int main(void){
    int *addresses;
    int *pageTable = initializePageTable(256);
    addresses = readAddresses("addresses.txt", 10);
    queue_t *frameList = initializeQueue(256);
    frameList = initFreeFrames(frameList);

    for(int i = 0; i < numAddresses; i++){
        int pageNumber = findPageNumber(addresses[i]);
        int offset = getOffset(addresses[i]);
        int freeFrame;

        if (findFrame(pageTable, pageNumber) == -1) {
            freeFrame = findFreeFrame(frameList);
            updatePageTable(pageTable, freeFrame, pageNumber);
        } else {
            freeFrame = findFrame(pageTable, pageNumber);
        }

        printf("Physical address for %d, is %d\n", addresses[i], ((findFrame(pageTable, pageNumber) * 256) + offset));
    }
    printPageTable(pageTable, 256);
}