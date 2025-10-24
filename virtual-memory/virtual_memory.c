#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct queue {
    int front;
    int rear;
    int *queue;
    int size;
    int capacity;
} queue_t;

int numAddresses;

int *readAddresses(char *file, int capacity){
    int *addresses = malloc(sizeof(int) * capacity);
    int count = 0;
    int currAddress; 

    FILE *fptr = fopen(file, "r");

    while(fscanf(fptr, "%d", &currAddress) == 1){
        if(count > capacity - 1){
            addresses = realloc(addresses, sizeof(int) * capacity * 2);
        }
        addresses[count] = currAddress; 
        count++;
    }

    numAddresses = count; 
    fclose(fptr);
    return addresses;
}

void findPhysicalAddress(int logical){
    int offset = logical & 0x000000FF;
    uint32_t frame = (logical & 0x0000FF00) >> 8;

    printf("Offset %d\n", offset);
    printf("Frame: %d\n", frame);
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
    return queue->size == 0;
}

void initFreeFrames(queue_t *queue){
    for(int i = 0; i < queue->capacity; i++){
        queue->queue[i] = i;
    }
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

// Update to check malloc doesn't return NULL
int *initializePageTable(int capacity){

    int *pageTable = malloc(sizeof(int) * capacity);
    for(int i = 0; i < capacity; i++){
        pageTable[i] = -1;
    }

    return pageTable; 
}

// Update to be safer, i.e do some checking 
void updatePageTable(int *pageTable, int freeFrame, int pageNumber){
    pageTable[pageNumber] = freeFrame; 
}

/*
A page table has page # and frame #, at every page request, you need to find a free frame (contiguously in this case), allocate this
frame to this page, initialize in the page table, "load into memory", and read page + offset. If frame size is 2^8 bytes then you cover 
256 bytes in one frame. The easiest way to keep a page table, is an array of size 256, with each value being the frame for that specific page initially all 
frames are -1, keep a free frame list, if frames are contiguous this is easy ... just an array. But if frames are non-contigously allocated for example after page
replacement, it may be better to just use a linked list */

int main(void){
    int *returnedArray;
    returnedArray = readAddresses("addresses.txt", 10);
    for(int i = 0; i < numAddresses; i++){
        findPhysicalAddress(returnedArray[i]);
    }
}