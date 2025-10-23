#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

unsigned int systemClock;
/**
 * Struct for representing a memory partition,
 * occupied size is added to make computing memory
 * metrics easier.
 */
struct memoryPartition{
    unsigned int memSize;
    unsigned int number;
    unsigned int occupiedSize;
    int occupied;
};

/*
 * Struct for representing the
 * PCB as a process with all required information,
 * used as a node in a linked list.
 */
struct PCB{
    unsigned int PID;
    unsigned int memoryLocation;
    unsigned int size;
    unsigned int IOFrequency;
    unsigned int IODuration;
    unsigned int CPUTime;
    int remainingTime;
    unsigned int arrivalTime;
    unsigned int waitingTime;
    struct PCB *next; 
};

/**
 * Queue data structure declaration.
 */
struct queue{
    struct PCB *head;
    struct PCB *tail;
    unsigned int numElements;
};

/**
 * This struct is for returning pointers in
 * the priority implementation as two linked
 * lists are modified, this was required. 
 */
struct updatedLists{
    struct PCB *waitProgramList;
    struct PCB *priorityReadyList;
};

/**
 * Helper function to init array size based
 * on the number of lines in the input file. 
 */
int countLines(char *fileName){
  int numLines = 0;
  int maxRead = 256;
  char tempStore[maxRead];
  FILE *readTrace;
  readTrace = fopen(fileName, "r");
  assert(readTrace != NULL);

  if(readTrace != NULL){
    while(fgets(tempStore, maxRead, readTrace)){
      numLines += 1;
    } 
  }

 fclose(readTrace);
 return numLines;

}

/**
 * Function to intitialize memory table
 * with all of its values. 
 */
void initializeMemoryTable(struct memoryPartition *memoryTable){
    for(int i=1; i < 7; i++){
        memoryTable[i-1].number = i;
        memoryTable[i-1].occupied = -1;
        memoryTable[i-1].occupiedSize = 0;
        switch(i){
            case 1:
                memoryTable[i-1].memSize = 40;
                break;
            case 2:
                memoryTable[i-1].memSize = 25;
                break;
            case 3:
                memoryTable[i-1].memSize = 15;
                break;
            case 4:
                memoryTable[i-1].memSize = 10;
                break;
            case 5:
                memoryTable[i-1].memSize = 8;
                break;
            case 6:
                memoryTable[i-1].memSize = 2;
                break;
        }
    }
}

/**
 * Function that returns the best fit for a program,
 * marks the memory as occupied. 
 */
int searchMemory(struct memoryPartition *table, int size, int PID){
    int bestFit = 0;
    for(int i = 0; i < 6; i++){
        if(table[i].memSize >= size && table[i].occupied == -1){
            bestFit = i + 1;
        }
    }
    if(bestFit != 0){
        table[bestFit - 1].occupied = PID;
        table[bestFit - 1].occupiedSize = size;
    }
    return bestFit;
}

/**
 * Function for "clearing" out a process
 * from memory.
 */
void freeMemory(int PID, struct memoryPartition *table){
    for(int i = 0; i < 6; i++){
        if(table[i].occupied == PID){
            table[i].occupied = -1;
            table[i].occupiedSize = 0;
        }
    }
}

/**
 * Helper function to create a queue. 
 */
struct queue *createQueue(void){
    struct queue *newQueue = malloc(sizeof(struct queue));
    if(newQueue != NULL){
        newQueue->head = NULL;
        newQueue->tail = NULL;
        newQueue->numElements = 0;
    }
    return newQueue;
}

/**
 * Enqueue function, following general form.
 */
void enqueue(struct queue *updateQueue, struct PCB *addProcess){
    assert(updateQueue != NULL);
    if(updateQueue->head == NULL && updateQueue->tail == NULL){
        updateQueue->head = addProcess;
        updateQueue->tail = addProcess;
        updateQueue->numElements = 1;
    }
    else{
        updateQueue->tail->next = addProcess;
        updateQueue->tail = addProcess;
        updateQueue->numElements++;
    }
}

/**
 * Dequeue function, also follows general form.
 */
struct PCB *dequeue (struct queue *updateQueue){
    assert(updateQueue != NULL);
    struct PCB *returnPCB;
    
    if(updateQueue->head == NULL && updateQueue->tail == NULL){
        return NULL;
    }
    if(updateQueue->numElements == 1){
        returnPCB = updateQueue->head;
        updateQueue->head = NULL;
        updateQueue->tail = NULL;
        updateQueue->numElements = 0;
    }
    else {
        returnPCB = updateQueue->head;
        updateQueue->head = updateQueue->head->next;
        updateQueue->numElements--;
    }
    returnPCB->next = NULL;
    return returnPCB;
}

/**
 * Initializes a linked list with all incoming programs,
 * from the input file, does not guarantee ordering, priority,
 * or entry to the system. 
 */
struct PCB *readInput(char *fileName, int numLines, struct PCB *head){
    FILE *readInput;
    readInput = fopen(fileName, "r");
    assert(readInput != NULL);
    int count = 0; 
    int PID, size, arrivalTime, CPUTime, IOFreq, IODuration; 
    
    struct PCB *currentProgram;
    struct PCB *returnPointer;
    currentProgram = head;

    while(count < numLines){
        
        fscanf(readInput, "%d, %d, %d, %d, %d, %d\n", &PID, &size, &arrivalTime, &CPUTime, &IOFreq, &IODuration);
            
        struct PCB *newProgram = malloc(sizeof(struct PCB));

        if(newProgram != NULL){
            newProgram->PID = PID;
            newProgram->size = size;
            newProgram->IOFrequency = IOFreq;
            newProgram->IODuration = IODuration;
            newProgram->CPUTime = CPUTime;
            newProgram->arrivalTime = arrivalTime;
            newProgram->remainingTime = CPUTime; 
            newProgram->memoryLocation = 7;
            newProgram->waitingTime = 0;  
            newProgram->next = NULL;

            if(currentProgram == head){
                currentProgram = newProgram;
                returnPointer = currentProgram;
            }
            else{
                currentProgram->next = newProgram;
                currentProgram = currentProgram->next;
            }
        }
        count++;
    }
    fclose(readInput);
    return returnPointer;
}

/**
 * Output helper function. 
 */
void setUpMemoryOutput(){
    FILE *memoryOutput = fopen("memory_status_101219700_101198779.txt", "w");
    fprintf(memoryOutput, "+------------------------------------------------------------------------------------------+\n");
    fprintf(memoryOutput, "| %-14s | %-11s | %-16s | %-17s | %-17s |\n", 
           "Time of Event", "Memory Used", "Partitions State", "Total Free Memory", "Usable Free Memory");
    fprintf(memoryOutput, "+------------------------------------------------------------------------------------------+\n");
    fclose(memoryOutput);
}

/**
 * Output helper function. 
 */
void closeMemoryOutput(){
    FILE *memoryOutput = fopen("memory_status_101219700_101198779.txt", "a");
    fprintf(memoryOutput, "+------------------------------------------------------------------------------------------+\n");
    fclose(memoryOutput);
}

/**
 * Output helper function. 
 */
void printMemory(struct memoryPartition *memoryTable, int sysClock){
    FILE *memoryOutput = fopen("memory_status_101219700_101198779.txt", "a");
    unsigned int memoryUsed = 0;
    unsigned int usableMemory = 0;
    for(int i = 0; i < 6; i++){
        if(memoryTable[i].occupied != -1){
            memoryUsed += memoryTable[i].occupiedSize;
        }
        else{
            usableMemory += memoryTable[i].memSize;
        }
    }
    fprintf(memoryOutput, "| %-14d | %-11d | ", sysClock, memoryUsed);
    fprintf(memoryOutput, " %-3d, %-3d, %-3d, %-3d, %-3d, %-3d | ", memoryTable[0].occupied, memoryTable[1].occupied, memoryTable[2].occupied, memoryTable[3].occupied, memoryTable[4].occupied, memoryTable[5].occupied);
    fprintf(memoryOutput, " %-17d | %-17d |\n", (100 - memoryUsed), usableMemory);
    fclose(memoryOutput);
}

/**
 * Output helper function. 
 */
void setUpOutput(){
    FILE *outputFile = fopen("execution_101219700_101198779.txt", "w");
    fprintf(outputFile, "+------------------------------------------------+\n");
    fprintf(outputFile, "|%-18s|%-5s|%-10s|%-10s|\n", "Time of Transition", "PID", "Old State", "New State");
    fprintf(outputFile, "+------------------------------------------------+\n");
    fclose(outputFile);
}

/**
 * Output helper function. 
 */
void closeOutput(){
    FILE *outputFile = fopen("execution_101219700_101198779.txt", "a");
    fprintf(outputFile, "+------------------------------------------------+\n");
    fclose(outputFile);
}

/**
 * Output helper function, takes in an int representing
 * the state change, and the system clock, to output to 
 * the file. 
 */
void addToOutput(int stateChange, int PID, int sysClock){
    FILE *outputFile = fopen("execution_101219700_101198779.txt", "a");
    switch(stateChange){
        case 0:
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "NEW", 
               "READY");
            break;
        case 1:
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "READY", 
               "RUNNING");
            break;
        case 2:
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "RUNNING", 
               "WAITING");
            break;
        case 3:
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "WAITING", 
               "READY");
            break;
        case 4: 
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "RUNNING", 
               "TERMINATED");
            break;
        case 5:
            fprintf(outputFile,"|%-18d|%-5d|%-10s|%-10s|\n",
               sysClock, 
               PID, 
               "RUNNING", 
               "READY");
    }
    fclose(outputFile);
}

/**
 * Waiting list add function, implemented
 * as a linked list as it does not follow FIFO.
 */
struct PCB *addToWaiting(struct PCB *head, struct PCB *newProcess){
    newProcess->waitingTime = 0;
    if(head == NULL){
        head = newProcess;
    }
    else{
        struct PCB *trace = head;
        while(trace->next != NULL){
            trace = trace->next;
        }
        trace->next = newProcess;
    }
    return head;
}

/**
 * Function to increment the wait times
 * of the processes in the waiting list,
 * called in main processing block. 
 */
void incrementWaitTimes(struct PCB *waitList){
    if(waitList != NULL){
        while(waitList != NULL){
            waitList->waitingTime++;
            waitList = waitList->next;
        }
    }
}

/**
 * Priority implementation is based on SJF,
 * this searches the priority list, and inserts
 * the incoming program based on SJF criteria. 
 */
struct PCB *addToReadyPriority(struct PCB *head, struct PCB *newProgram){  
    struct PCB *trace = head; 
    int insertFlag = 0;

    if(head == NULL){
        newProgram->next = NULL;
        head = newProgram;
    }
    else if(head->remainingTime > newProgram->remainingTime){
        newProgram->next = head;
        head = newProgram; 
    }
    else{
      while(trace->next != NULL){
        if(trace->next->remainingTime > newProgram->remainingTime){
            newProgram->next = trace->next;
            trace->next = newProgram;
            insertFlag = 1;
            break;
        }
        else{
            trace = trace->next;
        }
        } 
        if(!insertFlag){
            trace->next = newProgram;
            newProgram->next = NULL;
        } 
    }
    return head; 
}

/**
 * This function updates the waiting list linked list
 * based on whether the process has completed
 * its IO wait time. If this is priority, then control flag is 1, and
 * the process is added back to priority list, if this is RR or FCFS, then
 * it is queued.
 */
struct updatedLists *updateWaitingList(struct PCB *head, struct queue *readyQueue, int controlFlag, struct PCB *readyList){
    
    struct updatedLists *newLists = malloc(sizeof(struct updatedLists));

    newLists->waitProgramList = head;
    newLists->priorityReadyList = readyList;

    if(head == NULL){
        return newLists; 
    }

    struct PCB *trace = head;
    struct PCB *copy; 

    while(trace->next != NULL){
        if(trace == head && (trace->IODuration == trace->waitingTime)){
            addToOutput(3, trace->PID, systemClock);
            copy = trace;
            head = trace->next;
            trace = trace->next;
            copy->next = NULL;
            if(!controlFlag){
                enqueue(readyQueue, copy);
            }
            else{
                newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
            }
        }
        else if(trace->next->IODuration == trace->next->waitingTime){
            addToOutput(3, trace->next->PID, systemClock);
            copy = trace->next;
            trace->next = trace->next->next;
            copy->next = NULL;
            if(!controlFlag){
                enqueue(readyQueue, copy);
            }
            else{
                newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
            }
        }
        else{
            trace = trace->next;
        }
    }

    if(head->next == NULL && head->IODuration == head->waitingTime){
        addToOutput(3, head->PID, systemClock);
        copy = head;
        head = NULL;
        if(!controlFlag){
            enqueue(readyQueue, copy);
        }
        else{
            newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
        }
    }

    newLists->waitProgramList = head;
    return newLists;
}

/**
 * This function is responsible for 
 * bringing programs in to become processes, if
 * this is FCFS or RR then it queues them in the ready queue
 * based on whether there is enough memory, 
 * otherwise if this is priority then it calls the addToPriority function.
 */
struct updatedLists *updateProgramList(struct PCB *head, struct memoryPartition *memoryTable, struct queue *readyQueue, int systemClock, int controlFlag, struct PCB *readyList){
    
    struct updatedLists *newLists = malloc(sizeof(struct updatedLists));

    newLists->waitProgramList = head;
    newLists->priorityReadyList = readyList;

    if(head == NULL){
        return newLists; 
    }

    struct PCB *trace = head;
    struct PCB *copy; 

    while(trace->next != NULL){
        if(trace == head && (trace->arrivalTime <= systemClock)){
            if(searchMemory(memoryTable, trace->size, trace->PID) != 0){
                addToOutput(0, trace->PID, systemClock);
                copy = trace;
                head = trace->next;
                trace = trace->next;
                copy->next = NULL;
                if(!controlFlag){
                    enqueue(readyQueue, copy);
                }
                else{
                    newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
                }
            }
        }
        else if(trace->next->arrivalTime <= systemClock){
            if(searchMemory(memoryTable, trace->size, trace->PID) != 0){
                addToOutput(0, trace->next->PID, systemClock);
                copy = trace->next;
                trace->next = trace->next->next;
                copy->next = NULL;
                if(!controlFlag){
                    enqueue(readyQueue, copy);
                }
                else{
                    newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
                }
                printMemory(memoryTable, systemClock);
            }
        }
        else{
            trace = trace->next;
        }
    }

    if(head->next == NULL && head->arrivalTime <= systemClock){
        if(searchMemory(memoryTable, head->size, head->PID) != 0){
            addToOutput(0, head->PID, systemClock);
            copy = head;
            head = NULL;
            if(!controlFlag){
                enqueue(readyQueue, copy);
            }
            else{
                newLists->priorityReadyList = addToReadyPriority(newLists->priorityReadyList, copy);
            }
            printMemory(memoryTable, systemClock);
        }
    }

    newLists->waitProgramList = head;
    return newLists;
}

/**
 * This function is the main processing loop,
 * the control flag specifies the type of scheduler implemented,
 * this loops through until the list of programs awaiting entry is null, 
 * there is no process in the CPU, the ready queue is empty, the wait list is empty, and 
 * in the case of priority, the priority list is empty. 
 */
void processingLoop(struct PCB *listOfPrograms, struct memoryPartition *memoryTable, int numPrograms, int controlFlag){
    systemClock = 0;
    int roundRobinTimer = 0;
    struct PCB *currentRunning = NULL;
    struct PCB *movingHelper;
    struct PCB *waitList = NULL;
    struct PCB *readyList = malloc(sizeof(struct PCB));
    readyList = NULL;
    setUpOutput();
    setUpMemoryOutput();
    struct queue *readyQueue = createQueue();

    while(listOfPrograms != NULL || currentRunning != NULL || readyQueue->numElements > 0 || waitList != NULL || readyList != NULL){
       
        if(controlFlag == 0 || controlFlag == 2){
            struct updatedLists *addList = updateProgramList(listOfPrograms, memoryTable, readyQueue, systemClock, 0, NULL);
            listOfPrograms = addList->waitProgramList;
            free(addList);
            if(readyQueue->numElements > 0 && currentRunning == NULL){
                currentRunning = dequeue(readyQueue);
                addToOutput(1, currentRunning->PID, systemClock);
                roundRobinTimer = 0;
            }
        }

        else if(controlFlag == 1){
            struct updatedLists *addListP = updateProgramList(listOfPrograms, memoryTable, NULL, systemClock, 1, readyList);
            listOfPrograms = addListP->waitProgramList;
            readyList = addListP->priorityReadyList;
            free(addListP);
            if(readyList != NULL && currentRunning == NULL){
                currentRunning = readyList;
                addToOutput(1, currentRunning->PID, systemClock);
                readyList = readyList->next;
                currentRunning->next = NULL;
            }
        }

        systemClock++;
        incrementWaitTimes(waitList);
        roundRobinTimer++;

        if(controlFlag == 0 || controlFlag == 2){
            struct updatedLists *waitNew = updateWaitingList(waitList, readyQueue, 0, NULL);
            waitList = waitNew->waitProgramList;
            free(waitNew);
        }
        else if(controlFlag == 1){
            struct updatedLists *waitNewP = updateWaitingList(waitList, NULL, 1, readyList);
            waitList = waitNewP->waitProgramList;
            readyList = waitNewP->priorityReadyList;
            struct PCB *copy = readyList;
            free(waitNewP);
        }

        if(currentRunning != NULL){
            currentRunning->remainingTime--;
            if(currentRunning->remainingTime == 0){
                addToOutput(4, currentRunning->PID, systemClock);
                freeMemory(currentRunning->PID, memoryTable);
                printMemory(memoryTable, systemClock);
                free(currentRunning);
                currentRunning = NULL;
            }
            else if(currentRunning->IOFrequency != 0){
                if((currentRunning->CPUTime - currentRunning->remainingTime) % currentRunning->IOFrequency == 0){ 
                    addToOutput(2, currentRunning->PID, systemClock);
                    waitList = addToWaiting(waitList, currentRunning);
                    currentRunning = NULL;
                }
            }
            else if(controlFlag == 2 && roundRobinTimer == 100 && readyQueue->numElements > 0){ 
                addToOutput(5, currentRunning->PID, systemClock);
                enqueue(readyQueue, currentRunning);
                currentRunning = NULL;
            }
        }
    }
    free(readyQueue);
    free(readyList);
    closeOutput();
    closeMemoryOutput();
}

/**
 * Self explanatory, set control flag based on input, initialize memory,
 * and start processing. 
*/
int main(int argc, char *argv[]){

    if (argc != 3) {
        printf("%s Less than 3 parameters entered\n", argv[0]);
        return 1; 
    }

    char *fileName = argv[1];
    char *scheduler = argv[2];
    int controlFlag;

    if (strcmp(scheduler, "FCFS") == 0) {
        controlFlag = 0;
    } else if (strcmp(scheduler, "EP") == 0) {
        controlFlag = 1;
    } else if (strcmp(scheduler, "RR") == 0) {
        controlFlag = 2;
    } else {
        printf("Valid options are FCFS, RR, or EP.\n");
        return 1;
    }

    struct memoryPartition memoryTable[6];
    initializeMemoryTable(memoryTable);

    int numPrograms = countLines(fileName);
    struct PCB *programList;
    programList = readInput(fileName, numPrograms, programList);

    processingLoop(programList, memoryTable, numPrograms, controlFlag);
}