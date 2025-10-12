#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Enum to classify the instruction type, just to make string processing easier */
enum callType{ 
  CPU,
  SYSCALL,
  END_IO
};

/* Struct to store the instruction and its associated information */
struct instruction{
  enum callType thisCall;
  int vectorPosition;
  int executeTime;
};

/* This needed to be wrapped in a struct because I kept running into errors with pointer access for an 
array of strings */
struct ISR{
  char vector[7];
};

/* Function to count lines in the file so that the size of the array of instructions
can be set */
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


/* This function reads the file line by line, delimits the lines based on spaces and commas
and then stores each instruction into an instruction structure */
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

/* Initialize the ISR Table as an array of structs 
(trouble with string arrays) */
void initializeInterruptTable(struct ISR *intTable, int tableSize){ 
  int count = 0;
  FILE *readTable;
  readTable = fopen("vector_table.txt", "r");
  while (count < tableSize){
        fscanf(readTable,"%s", intTable->vector);
        count++;
        intTable++;
  }
}

/* This function is called when generating time for 
context switching and data transfer in a SYSCALL, some 
time limitations had to be set because of the nature of the rand() function, flag variable for
whether its the first time its called or second */
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

/* This is the main processing block for this program, process the instruction set array, generate random times,
and write to the output file based on the instruction type. Keep track of the system clock here */
void processInstructions(struct instruction *instSet, int numInst, struct ISR *vectorTable, char *fileName, char *outputFileName){
 
  FILE *outputFile;
  outputFile = fopen(outputFileName,"w");
  int count = 0;
  int systemClock = 0;
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
}

/* Main is used for processing file names, and intiailizing the struct arrays, 
function calls are made here*/
int main(int argc, char *argv[]){

  int numInstructions = 0;
  numInstructions = countLines(argv[1]);

  if (numInstructions == 0){
    printf("Empty file, please try again");
  }
  else {
    struct instruction programInstructions[numInstructions];
    storeInstructions(programInstructions, numInstructions, argv[1]);
    
    struct ISR ISRTable[25];  
    initializeInterruptTable(ISRTable, 25);

    processInstructions(programInstructions, numInstructions,ISRTable, argv[1], argv[2]);
  }

}
