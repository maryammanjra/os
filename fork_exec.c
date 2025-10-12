#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*Enum for fork and exec calls*/
enum traceCallType{
    FORK,
    EXEC
};

/*Enum for other program calls*/
enum callType{ 
  CPU,
  SYSCALL,
  END_IO
};

/*Vector struct, needed to be wrapped because of pointer errors*/
struct ISR{
  char vector[7];
};

/*Program struct for external file date*/
struct program{
    unsigned int size;
    char name[21];
};

/*Stores calls of the type fork, and exec*/
struct traceInstruction{
    enum traceCallType thisCall;
    int executeTime;
    char name[26];
};

/*Stores calls of the type SYSCALL, END_IO, CPU*/
struct instruction{
  enum callType thisCall;
  int vectorPosition;
  int executeTime;
};

/*Memory partition struct*/
struct partition{
    unsigned int size;
    unsigned int number;
    char name[21];
};

/*PCB, no execution time implemented yet*/
struct PCB{
    unsigned int PID;
    unsigned int memoryLocation;
    unsigned int size;
    char name[21];
    struct PCB *child;
    struct PCB *next; 
};

/*Helper function for intiializing struct array sizes*/
int countLines(char *fileName){
  int numLines = 0;
  int maxRead = 256;
  char tempStore[maxRead];
  FILE *readTrace;
  readTrace = fopen(fileName, "r");

  if(readTrace != NULL){
    while(fgets(tempStore, maxRead, readTrace)){
      numLines += 1;
    } 
  }

 fclose(readTrace);
 return numLines;

}

/*Initialize PCB table with just the init process*/
struct PCB *initPCBTable(struct PCB *head){
    struct PCB *firstPCB = malloc(sizeof(struct PCB));
    assert(firstPCB != NULL);
    
    firstPCB->PID = 0;
    firstPCB->memoryLocation = 6;
    firstPCB->size = 1;
    strcpy(firstPCB->name, "init");
    firstPCB->next = NULL;
    firstPCB->child = NULL;

    head = firstPCB;
    return head;
}

/*Initialize a new PCB, the flag determines whether it's a child of init, or a child of another process*/
struct PCB *initNewPCB(struct PCB *head, int flag){
    
    struct PCB *newPCB = (malloc(sizeof(struct PCB)));
    assert(newPCB != NULL); 

    struct PCB *tracker = head;
    while(tracker->next != NULL){
        tracker = tracker->next; 
    }

    newPCB->PID = tracker->PID + 1;
    tracker->next = newPCB;
    newPCB->next = NULL;
    newPCB->child = NULL;

    
    if(flag == 0){
      newPCB->memoryLocation = head->memoryLocation;
      newPCB->size = head->size;
      strcpy(newPCB->name, head->name);
    }
    else{
      newPCB->memoryLocation = tracker->memoryLocation;
      newPCB->size = tracker->size;
      strcpy(newPCB->name, tracker->name);
    }

    return head;

}

/*Finds the PCB of the specified name */
struct PCB *findPCB(char *name, struct PCB *head){
  struct PCB *trace = head;
  while(trace != NULL){
    if(strcmp(trace->name, name) == 0){
      return trace;
    }
    else{
      trace = trace->next;
    }
  }
  return NULL;
}

/*Initialize the memory table, with only init at first*/
void initializeMemory(struct partition *memoryTable){
    for(int i  = 1; i < 7; i++){
        switch(i){
            case 1:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 40;
            strcpy(memoryTable[i-1].name, "free");
            break;

            case 2:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 25;
            strcpy(memoryTable[i-1].name, "free");
            break;

            case 3:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 15;
            strcpy(memoryTable[i-1].name, "free");
            break;

            case 4:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 10;
            strcpy(memoryTable[i-1].name, "free");
            break;

            case 5:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 8;
            strcpy(memoryTable[i-1].name, "free");
            break;

            case 6:
            memoryTable[i-1].number = i;
            memoryTable[i-1].size = 2;
            strcpy(memoryTable[i-1].name, "init");
            break;
        }
    }
}

/*Initialize the interrupt table*/
void initializeInterruptTable(struct ISR *intTable, int tableSize){ 
  int count = 0;
  FILE *readTable;
  readTable = fopen("vector_table_101219700_101198779.txt", "r");
  while (count < tableSize){
        fscanf(readTable,"%s", intTable->vector);
        count++;
        intTable++;
  }
}

/*Store external file program data*/
void initExternalFiles (struct program *programSet, int arraySize){
    FILE *readExternal = fopen("external_files_101219700_101198779.txt", "r");
    char programName[21];
    char *nameDelimited;
    int programSize;
    int count = 0;

    while(count < arraySize){
        if(fscanf(readExternal, "%s%d", programName, &programSize)){
            nameDelimited = strtok(programName, ",");
            strcpy(programSet->name, nameDelimited);
            programSet->size = programSize;
            count++;
            programSet++;
        }
        else {
            printf("Error in reading your file");
            break;
        }
    }
}

/*Search for the size of the program you are looking to store in memory*/
int findProgramInfo(char *name, struct program *table, int tableSize){
    int size = 0;
    for(int i = 0; i < tableSize; i++){
        if(strcmp(table[i].name, name) == 0){
            size = table[i].size;
        }
    }
    return size;
}

/*Find a memory position best suited to that size*/
int searchMemory(int size, struct partition *memoryTable, char *programName){
    int bestFit = 0;
    for(int i = 0; i < 6; i++){
        if(strncmp(memoryTable[i].name, "free", 4) == 0 && memoryTable[i].size >= size){
            bestFit = i + 1;
        }
    }
     strcpy(memoryTable[bestFit-1].name, programName);
    return bestFit;
}

/*Update the PCB once memory has been allocated*/
struct PCB *updatePCB(struct PCB *head, int memoryLocation, int size, char *name){
    struct PCB *trace = head;
    struct PCB *parent = head;
   
    while(trace->next != NULL){
        trace = trace->next;
        if(trace->next != NULL){
          if(trace->next->next == NULL){
            parent = trace;
          }
        }
    }

        trace->memoryLocation = memoryLocation;
        trace->size = size;
        strcpy(trace->name, name);
    

    return head;
}

/*Helper function for calculating random times in SYSCALLS, and IO calls*/
int calculateRandTime(int remainingTime, int typeFlag){
  if (typeFlag == 0){
    return ((rand() % (remainingTime/3)) + (remainingTime/3));
  }
  else if (typeFlag == 1){
    return ((rand() % (remainingTime/4)) + (remainingTime/2));
  }
  else {
    return 0;
  }
}

/*Checks what type of instructions are being executed to call the appropriate function... see below, for now assumes file only contains fork, exec 
or SYSCALL, CPU, and END_IO, will be changed in the next iteration to assume a mix of both*/
int checkInstructionType(char *fileName){
  FILE *check = fopen(fileName, "r");
  char line[32];
  if(fgets(line, 32, check) != NULL){
    if(line[0] == 'F' || line[0] == 'E'){
      return 0;
    }
    else{
      return 1;
    }
  }
  return -1;
  fclose(check);
}

/*Stores instruction of type CPU, SYSCALL, and END_IO into an array with necessary information*/
void storeInstructions(struct instruction *instSet, int numInst, char *fileName){
  FILE *readTrace;
  readTrace = fopen(fileName, "r");

  char instruction[15];
  int time;
  int vectorNumber;
  int count = 0;

  while(count < numInst){
    if(fscanf(readTrace,"%s%d ,%d", instruction, &vectorNumber, &time)){
      if (instruction[0] == 'C'){
        instSet->thisCall = CPU;
        instSet->executeTime = vectorNumber;
      }
      else if (instruction[0] == 'S'){
        instSet->thisCall = SYSCALL;
        instSet->vectorPosition = vectorNumber;
        instSet->executeTime = time;
      }
      else if (instruction[0] == 'E'){
        instSet->thisCall = END_IO;
        instSet->vectorPosition = vectorNumber;
        instSet->executeTime = time;
      }
      else {
        printf("There is an error in your file formatting, please try again");
        break;
      } 
      instSet++;
      count++;
    }
    else{
      printf("Error in reading your file");
      break;
    } 
  }
  fclose(readTrace);
}

/*Stores the instructions from the init file, and files containing FORK and EXEC calls into an array */
void storeTraceInstructions(struct traceInstruction *traceInstructionSet, char *fileName, int arraySize){
    FILE *readTrace = fopen(fileName, "r");

    int time; 
    int count = 0;
    char line[32];
    char *strTime;
    char *programName;

    while(count < arraySize){
        if(fgets(line, 32, readTrace) != NULL){
            if(line[0] == 'F'){
                traceInstructionSet->thisCall = FORK;
                strTime = strtok(line, ", ");
                strTime = strtok(NULL, " ");
                time = strtol(strTime, NULL, 10);
                traceInstructionSet->executeTime = time;
            }
            else if(line[0] == 'E'){
                traceInstructionSet->thisCall = EXEC;
                programName = strtok(line, " ");
                programName = strtok(NULL, ", ");
                strTime = strtok(NULL, " ");
                time = strtol(strTime, NULL, 10);
                traceInstructionSet->executeTime = time;
                strcpy(traceInstructionSet->name, programName);
            }
            else {
                printf("There is an error in your trace file, please try again");
                break;
            }
            traceInstructionSet++;
            count++;
        }
        else {
            printf("Error in reading your file");
            break;
        }
    }
}

/*Prints the status everytime a new PCB is initialized, I wasn't sure how to set the font after looking into it, sorry*/
void printStatus(struct PCB *head, int time){
    struct PCB *tracker = head;
    FILE *printStatus = fopen("system_status_101219700_101198779.txt", "a");
    char header[65] = "! ----------------------------------------------------------- !";
    
    char table[65] = "+--------------------------------------------+";
    fprintf(printStatus, "%s\n", header);
    fprintf(printStatus,"%s %d %s\n", "Save time: ", time, " ms");
    fprintf(printStatus, "%s\n", table);
    fprintf(printStatus, "%s\n", "|  PID  | Program Name | Partition Number  | size |");
    fprintf(printStatus, "%s\n", table);

    while(tracker != NULL){
        fprintf(printStatus, "%s%d%s%s%s%d%s%d%s\n", "|  ", tracker->PID,"  | ", tracker->name," | ", tracker->memoryLocation,"  |", tracker->size," |" );
        tracker = tracker->next;
    }
    fprintf(printStatus, "%s\n", table);
    fclose(printStatus);
}

/*Main processing block for instructions of type SYSCALL, END_IO, and CPU*/ 
int processInstructions(struct instruction *instSet, int numInst, struct ISR *vectorTable, char *outputFileName, int currentClock){
 
  FILE *outputFile = fopen(outputFileName, "a");
  int count = 0;
  int systemClock = currentClock;
  int contextTime;

  
  int runISR;
  int dataTransfer;
  int errorCheck;
  int executeTimeCopy;

  while(count < numInst){
    if(instSet->thisCall == 0){
      fprintf(outputFile,"%d, %d, %s\n", systemClock, instSet->executeTime, "CPU execution");
      systemClock += instSet->executeTime;
      count += 1;
      instSet++;
    }
    else if(instSet->thisCall == 1){
      fprintf(outputFile,"%d, %d, %s\n", systemClock, 1, "switch to kernel mode");
      systemClock += 1;

      contextTime = (rand() % 2) + 1;
      fprintf(outputFile,"%d, %d, %s\n", systemClock, contextTime, "context saved");
      systemClock += contextTime; 

      fprintf(outputFile,"%d, %d, %s %d %s %s\n", systemClock,1,"find vector", instSet->vectorPosition, "in memory position", vectorTable[instSet->vectorPosition - 1].vector);
      systemClock += 1;

      fprintf(outputFile, "%d, %d, %s %s %s\n", systemClock, 1, "load address", vectorTable[instSet->vectorPosition - 1].vector, "into the PC");
      systemClock += 1;

      executeTimeCopy = instSet->executeTime;
      runISR = calculateRandTime(executeTimeCopy, 0);
      fprintf(outputFile,"%d, %d, %s\n", systemClock, runISR, "SYSCALL: run the ISR");
      executeTimeCopy -= runISR; 
      systemClock += runISR;

      dataTransfer = calculateRandTime(executeTimeCopy, 1);
      fprintf(outputFile,"%d, %d, %s\n", systemClock, dataTransfer, "transfer data");
      systemClock += dataTransfer;
      executeTimeCopy -= dataTransfer;

      fprintf(outputFile, "%d, %d, %s\n", systemClock, executeTimeCopy, "check for errors");
      systemClock += executeTimeCopy;
      
      fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "IRET");
      count++;
      instSet++;
    }
    else if (instSet->thisCall == 2){
      fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "check interrupt priority");
      systemClock += 1; 

      fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "check if masked");
      systemClock += 1; 


      fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "switch to kernel mode");
      systemClock += 1; 

      contextTime = (rand() % 2) + 1;
      fprintf(outputFile,"%d, %d, %s\n", systemClock, contextTime, "context saved");
      systemClock += contextTime; 

      fprintf(outputFile,"%d, %d, %s %d %s %s\n", systemClock,1,"find vector", instSet->vectorPosition, "in memory position", vectorTable[instSet->vectorPosition - 1].vector);
      systemClock += 1;

      fprintf(outputFile, "%d, %d, %s %s %s\n", systemClock, 1, "load address", vectorTable[instSet->vectorPosition - 1].vector, "into the PC");
      systemClock += 1;

      fprintf(outputFile, "%d, %d, %s\n", systemClock, instSet->executeTime, "END_IO");
      systemClock += instSet->executeTime;

      fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "IRET");
      systemClock += 1;
      count++;
      instSet++;
    }
    else {
      printf("%s\n", "ERROR");
      count += 1;
    }
  }
  return systemClock;
}

/*Main processing block of instructions of type FORK, and EXEC, does printing and begins execution of forked files recursively*/
int processTraceInstructions(struct traceInstruction *instructionArray, struct program *programArray, struct partition *memoryTable, struct PCB *head, struct ISR *vectorTable, int sOne, int sTwo, int currentClock){ 
    
    FILE *outputFile;
    int contextTime, forkTime, schedulerTime, currentProgramSize, execTime, memLocation, searchTime, markTime, updateTime;
    int randomRange; 

    int count = 0;
    int systemClock = currentClock;
    int parentFlag;
    
    if(systemClock == 0){
      parentFlag = 0;
    }
    else{
      parentFlag = 1;
    }


    while(count < sOne){
        outputFile = fopen("execution_101219700_101198779.txt","a");
        if(instructionArray->thisCall == 0){
            fprintf(outputFile,"%d, %d, %s\n", systemClock, 1, "switch to kernel mode");
            systemClock += 1;

            contextTime = (rand() % 2) + 1;
            fprintf(outputFile,"%d, %d, %s\n", systemClock, contextTime, "context saved");
            systemClock += contextTime; 

            fprintf(outputFile,"%d, %d, %s %d %s\n", systemClock, 1,"find vector", 2, "in memory position 0x0004");
            systemClock += 1;

            fprintf(outputFile, "%d, %d, %s %s %s\n", systemClock, 1, "load address", vectorTable[1].vector, "into the PC");
            systemClock += 1;

            forkTime = (rand() % (instructionArray->executeTime -1)) + 1;
            fprintf(outputFile, "%d, %d, %s\n", systemClock, forkTime, "FORK: copy parent PCB to child PCB");
            head = initNewPCB(head, parentFlag);
            systemClock += forkTime;
            printStatus(head,systemClock);

            schedulerTime = instructionArray->executeTime - forkTime;
            fprintf(outputFile, "%d, %d, %s\n", systemClock, schedulerTime, "scheduler called");
            systemClock += schedulerTime;

            fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "IRET");
            systemClock += 1;
        }
        else if(instructionArray->thisCall == 1){
            fprintf(outputFile,"%d, %d, %s\n", systemClock, 1, "switch to kernel mode");
            systemClock += 1;

            contextTime = (rand() % 2) + 1;
            fprintf(outputFile,"%d, %d, %s\n", systemClock, contextTime, "context saved");
            systemClock += contextTime; 

            fprintf(outputFile,"%d, %d, %s %d %s\n", systemClock, 1,"find vector", 3, "in memory position 0x0006");
            systemClock += 1;

            fprintf(outputFile, "%d, %d, %s %s %s\n", systemClock, 1, "load address", vectorTable[2].vector, "into the PC");
            systemClock += 1; 
            
            currentProgramSize = findProgramInfo(instructionArray->name, programArray, sTwo);
            randomRange = instructionArray->executeTime / 2;
            execTime = (rand() % randomRange) + 1;
            fprintf(outputFile, "%d, %d, %s %s %s %d%s\n", systemClock, execTime, "EXEC: load ", instructionArray->name, "of size", currentProgramSize, "Mb");
            systemClock += execTime;

            memLocation = searchMemory(currentProgramSize, memoryTable, instructionArray->name);
            randomRange = (instructionArray->executeTime - execTime) / 2;
            searchTime = (rand() % randomRange) + 1;
            fprintf(outputFile, "%d, %d, %s %d %s %d %s\n", systemClock, searchTime, "found partition", memLocation, "with", memoryTable[memLocation-1].size, "Mb of size");
            systemClock += searchTime;

            randomRange = (instructionArray->executeTime - (searchTime + execTime)) / 2;
            markTime = (rand() % randomRange) + 1;
            fprintf(outputFile, "%d, %d, %s %d %s\n", systemClock, markTime, "marked partition", memLocation, "as occupied");
            systemClock += markTime;

            randomRange = (instructionArray->executeTime - (markTime + searchTime + execTime)) / 2;
            updateTime = (rand() % randomRange) + 1;
            updatePCB(head, memLocation, currentProgramSize, instructionArray->name);
            fprintf(outputFile, "%d, %d, %s\n", systemClock, updateTime, "updating PCB with new information");
            systemClock += updateTime;
            printStatus(head,systemClock);

            schedulerTime = instructionArray->executeTime - (execTime + searchTime + markTime + updateTime);
            fprintf(outputFile, "%d, %d, %s\n", systemClock, schedulerTime, "scheduler called");
            systemClock += schedulerTime;

            fprintf(outputFile, "%d, %d, %s\n", systemClock, 1, "IRET");
            systemClock += 1;

            fclose(outputFile);
            char fileName[26];
            strcpy(fileName, instructionArray->name);
            strcat(fileName, ".txt");
            int numLines = countLines(fileName);
            if(checkInstructionType(fileName) == 1){
              struct instruction instructionArray[numLines];
              storeInstructions(instructionArray, numLines, fileName);
              systemClock = processInstructions(instructionArray, numLines, vectorTable, "execution_101219700_101198779.txt", systemClock);
            }
            else if(checkInstructionType(fileName) == 0){
              struct traceInstruction programInstr[numLines];
              storeTraceInstructions(programInstr, fileName, numLines);
              systemClock = processTraceInstructions(programInstr, programArray, memoryTable, head, vectorTable, numLines, sTwo, systemClock);
            }
        }
        fclose(outputFile);
        count++;
        instructionArray++;
    }
    return systemClock;
}

/*Initialize all struct arrays in main, no argv specifiers because of longer file names this time*/
int main(void){
    int numInstructions = 0;
    int numExternalFiles = 0;

    numInstructions = countLines("trace_101219700_101198779.txt");
    numExternalFiles = countLines("external_files_101219700_101198779.txt");

    struct traceInstruction instructionArray[numInstructions];
    storeTraceInstructions(instructionArray,"trace_101219700_101198779.txt",numInstructions);

    struct program programArray[numExternalFiles];
    initExternalFiles(programArray, numExternalFiles);

    struct ISR vectorTable[25];
    initializeInterruptTable(vectorTable, 25);

    struct partition memoryTable[6];
    initializeMemory(memoryTable);

    struct PCB *head;
    head = initPCBTable(head);
   
    processTraceInstructions(instructionArray,programArray,memoryTable,head,vectorTable,numInstructions,numExternalFiles, 0);

    struct PCB *trace = head;
    while(trace != NULL){
        printf("%s %d %d %d\n", trace->name, trace->memoryLocation, trace->size, trace->PID);
        trace = trace->next;
    }
}