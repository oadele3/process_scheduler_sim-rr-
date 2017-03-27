//
//  assignment1.c
//  
//
//  Created by Oluwamayowa Adeleke on 1/30/17.
//
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define MAXPROCESSES 64
#define MAXLINES 2048
#define MAXSTATUSCHARS 12

typedef struct {
    char type[MAXSTATUSCHARS];
    int duration;
    int process;
    char status[MAXSTATUSCHARS]; //notdone/active/done
    int valid;
} Event;

typedef struct {
    int id;
    int startTime;
    Event eventList[MAXLINES];
    char stateLocation[MAXSTATUSCHARS]; //USER, CORE's number, DISK
    char status[MAXSTATUSCHARS]; //NOTSTARTED/BLOCKED/ACTIVE/TERMINATED
    int currentEvent;
    int endTimeCurrent; //EndTime of current event
    int valid;
    int terminatedBefore;
} Process;

typedef struct {
    int id;
    int currentProcess;
    int busyUntilTime;
} Core;

typedef struct {
    int id;
    int currentProcess;
    int busyUntilTime;
} Disk;

typedef struct{
    int intArray[MAXPROCESSES];
    int front;
    int rear;
    int itemCount;
    char name[MAXSTATUSCHARS];
}Queue;

Process allProcessesArray[MAXPROCESSES];
int time[MAXLINES];
int currentTime;
int timeAddIndex;
int timeDoneIndex;
Core *arrayOfCores;
Disk singleDisk;
Queue readyQueue;
Queue diskQueue;

void deleteArrayElement (int position, int array[], int size)
{
    
}


void enqueue(int x, Queue* queue)
{
    //printf ("---- --%s front:%d back:%d count:%d\n", queue->name, queue->front, queue->rear, queue->itemCount);
    if((queue->front == 0 && queue->rear == MAXPROCESSES-1) || (queue->front > 0 && queue->rear == queue->front-1))
    {
        //printf("---- can't enqueue process %d in %s, Queue will overflow\n", x, queue->name);
    }
    else
    {
        if(queue->rear==MAXPROCESSES-1&&queue->front>0)
        {
            queue->rear=0;
            queue->intArray[queue->rear]=x;
            queue->itemCount++;
            //printf("---- process %d in has been added to %s\n", x, queue->name);
        }
        else
        {
            if((queue->front==0&&queue->rear==-1)||(queue->rear!=queue->front-1))
            {
                queue->intArray[++queue->rear]=x;
                queue->itemCount++;
                //printf("---- process %d in has been added to %s\n", x, queue->name);
            }
        }
        
    }
    //printf ("---- --%s has changed front:%d back:%d count:%d\n", queue->name, queue->front, queue->rear, queue->itemCount);
}

int dequeue( Queue *queue)
{
    //printf ("---- --%s front:%d back:%d count:%d\n", queue->name,queue->front, queue->rear, queue->itemCount);
    int a = -1;
    if((queue->front==0)&&(queue->rear==-1))
    {
        //printf("---- can't dequeue, queue is empty\n");
    }
    else if(queue->front==queue->rear)
    {
        a=queue->intArray[queue->front];
        queue->rear=-1;
        queue->front=0;
    }
    else
    {
        if(queue->front==MAXPROCESSES-1)
        {
            a=queue->intArray[queue->front];
            queue->front=0;
        }
        else
        {
            a=queue->intArray[queue->front++];
        }
    }
    if (a!=-1)
    {
        queue->itemCount--;
        //printf("---- process %d in has been removed from %s\n", a, queue->name);
    }
    //printf ("---- --%s has changed front:%d back:%d count:%d\n", queue->name, queue->front, queue->rear, queue->itemCount);
    return a;
}

int compare( const void* a, const void* b)
{
    int int_a = * ( (int*) a );
    int int_b = * ( (int*) b );
    
    if ( int_a == int_b ) return 0;
    else if ( int_a < int_b ) return -1;
    else return 1;
}

void HandleDisplayInputRequest(int id)
{
    //In that process, Increase EndTimeofcurrentevent by nextevent.time
    //and set stateLocation to 'user', CurrentEvent = nextevent.type
    allProcessesArray[id].currentEvent++;
    int eventDuration = allProcessesArray[id].eventList[allProcessesArray[id].currentEvent].duration;
    //printf("---- Process %d starts a DISPLAY or INPUT request at t = %d ms for %d ms\n", id, currentTime, eventDuration);
    allProcessesArray[id].endTimeCurrent = currentTime + eventDuration;
    //In that process, set stateLocation to 'user', CurrentEvent = nextevent.type
    strcpy(allProcessesArray[id].stateLocation, "USER");
    strcpy(allProcessesArray[id].status, "BLOCKED");

    //in the event itself set status to the [location]
    strcpy(allProcessesArray[id].eventList[allProcessesArray[id].currentEvent].status, "done");
    
    //add end time to time array
    time[timeAddIndex++] = allProcessesArray[id].endTimeCurrent;
}

void HandleCoreRequest(int coreID, int procID)
{
    //In that process, Increase EndTimeofcurrentevent by nextevent.time
    //and set stateLocation to 'user', CurrentEvent = nextevent.type
    allProcessesArray[procID].currentEvent++;
    int eventDuration = allProcessesArray[procID].eventList[allProcessesArray[procID].currentEvent].duration;
    allProcessesArray[procID].endTimeCurrent = currentTime + eventDuration;
    //In that process, set stateLocation to 'user', CurrentEvent = nextevent.type
    strcpy(allProcessesArray[procID].stateLocation, "CORE");
    strcpy(allProcessesArray[procID].status, "RUNNING");
    
    //in the event itself set status to the [location]
    strcpy(allProcessesArray[procID].eventList[allProcessesArray[procID].currentEvent].status, "active");
    
    //add end time to time array
    time[timeAddIndex++] = allProcessesArray[procID].endTimeCurrent;
    
    //in the core used set currentProcess and busy until time to new values
    //printf("---- Request for duration of %d ms\n", eventDuration);
    arrayOfCores[coreID].currentProcess = procID;
    arrayOfCores[coreID].busyUntilTime = allProcessesArray[procID].endTimeCurrent;
    //printf("---- Process %d will release core %d at %d ms\n", procID, coreID, arrayOfCores[coreID].busyUntilTime);
    
}

void HandleDiskRequest(int procID)
{
    //and set stateLocation to 'user', CurrentEvent = nextevent.type
    allProcessesArray[procID].currentEvent++;
    int eventDuration = allProcessesArray[procID].eventList[allProcessesArray[procID].currentEvent].duration;
    allProcessesArray[procID].endTimeCurrent = currentTime + eventDuration;
    //In that process, set stateLocation to 'user', CurrentEvent = nextevent.type
    strcpy(allProcessesArray[procID].stateLocation, "DISK");
    strcpy(allProcessesArray[procID].status, "BLOCKED");
    
    //in the event itself set status to the [location]
    strcpy(allProcessesArray[procID].eventList[allProcessesArray[procID].currentEvent].status, "active");
    
    //add end time to time array
    time[timeAddIndex++] = allProcessesArray[procID].endTimeCurrent;
    
    //in the core used set currentProcess and busy until time to new values
    singleDisk.currentProcess = procID;
    singleDisk.busyUntilTime = allProcessesArray[procID].endTimeCurrent;
    //printf("---- Process %d will release disk at %d ms\n", procID, singleDisk.busyUntilTime);
}


int main()
{
    //TODO: initialize these????
    //printf("-- WARNING: All output lines starting with a double dash are there to explain\n-- in detail how the simulation proceeds and are NOT REQUIRED.\n\n");
    
    readyQueue.front = 0;
    readyQueue.rear = -1;
    readyQueue.itemCount = 0;
    strcpy(readyQueue.name, "readyQueue");

    
    diskQueue.front = 0;
    diskQueue.rear = -1;
    diskQueue.itemCount = 0;
    strcpy(diskQueue.name, "diskQueue");


    int numProcesses = 0;
    
    currentTime = 0;
    for (int i = 0; i < MAXLINES; i++)
        time[i]=INT_MAX;
    timeAddIndex = -1;
    timeDoneIndex = -1;
    
    
    int terminationReportRequired[MAXPROCESSES];
    int terminationIndex = -1;
    
    // declare slice, array of cores and initialize values
    int slice, nCores, value;
    char operation[7];
    //TODO: confirm that NCORES will always be the first line
    scanf("%s %d", operation, &nCores);
    arrayOfCores = malloc(nCores * sizeof(Core));
    //TODO: confirm that SLICE will always be the second line
    scanf("%s %d", operation, &slice);
    //printf ("no of cores: %d\ntime-slice: %d\n\n", ncores, slice);
    for (int i = 0; i < nCores; i = i + 1)
    {
        //initialize values in array of cores
        arrayOfCores[i].id = i;
        arrayOfCores[i].currentProcess = -1;
        arrayOfCores[i].busyUntilTime = -1;
    }
    //initialize values in disk
    singleDisk.id = 1;
    singleDisk.currentProcess = -1;
    singleDisk.busyUntilTime = -1;
    
    //read in events and store them in arrays
    int nextProcID = -1;
    int nextEventID = -1;
    while(scanf("%s %d", operation, &value) == 2)
    {
        if (strcmp(operation, "NEW") == 0)
        {
            nextProcID ++;
            allProcessesArray[nextProcID].id = nextProcID;
            allProcessesArray[nextProcID].startTime = value;
            strcpy(allProcessesArray[nextProcID].stateLocation, "");
            strcpy(allProcessesArray[nextProcID].status, "NOTSTARTED");
            allProcessesArray[nextProcID].currentEvent = -1;
            allProcessesArray[nextProcID].endTimeCurrent = -1;
            allProcessesArray[nextProcID].valid = 1;
            allProcessesArray[nextProcID].terminatedBefore = 0;
            nextEventID = -1;
            time[timeAddIndex++] = value;
            numProcesses ++;
        }
        else if (strcmp(operation, "CORE") == 0)
        {
            int timeDuration = value;
            while (timeDuration > slice)
            {
                nextEventID++;
                strcpy(allProcessesArray[nextProcID].eventList[nextEventID].type, operation);
                allProcessesArray[nextProcID].eventList[nextEventID].duration = slice;
                allProcessesArray[nextProcID].eventList[nextEventID].process = nextProcID;
                strcpy(allProcessesArray[nextProcID].eventList[nextEventID].status, "notdone");
                allProcessesArray[nextProcID].eventList[nextEventID].valid = 1;
                timeDuration = timeDuration - slice;
            }
            
            nextEventID++;
            strcpy(allProcessesArray[nextProcID].eventList[nextEventID].type, operation);
            allProcessesArray[nextProcID].eventList[nextEventID].duration = timeDuration;
            allProcessesArray[nextProcID].eventList[nextEventID].process = nextProcID;
            strcpy(allProcessesArray[nextProcID].eventList[nextEventID].status, "notdone");
            allProcessesArray[nextProcID].eventList[nextEventID].valid = 1;
        }
        else
        {
            nextEventID++;
            strcpy(allProcessesArray[nextProcID].eventList[nextEventID].type, operation);
            //printf("here %s\n",operation);
            allProcessesArray[nextProcID].eventList[nextEventID].duration = value;
            allProcessesArray[nextProcID].eventList[nextEventID].process = nextProcID;
            strcpy(allProcessesArray[nextProcID].eventList[nextEventID].status, "notdone");
            allProcessesArray[nextProcID].eventList[nextEventID].valid = 1;
        }
    }
    
    /* to remove
    nextProcID = 0;
    while (allProcessesArray[nextProcID].valid == 1)
    {
        printf("process: %d\n", allProcessesArray[nextProcID].id);
        printf("start time: %d\n", allProcessesArray[nextProcID].startTime);
        printf("state location: %s\n", allProcessesArray[nextProcID].stateLocation);
        printf("current event: %d\n", allProcessesArray[nextProcID].currentEvent);
        printf("status: %s\n", allProcessesArray[nextProcID].status);
        printf("end-time of the current event: %d\n", allProcessesArray[nextProcID].endTimeCurrent);
        printf("events: [");
        nextEventID = 0;
        while (allProcessesArray[nextProcID].eventList[nextEventID].valid == 1)
        {
            printf("%s %d, ", allProcessesArray[nextProcID].eventList[nextEventID].type, allProcessesArray[nextProcID].eventList[nextEventID].duration);
            nextEventID++;
        }
        printf("]\n");
        printf("\n");
        
        nextProcID++;
    }
    */
    
    //sort time array
    qsort( time, timeAddIndex, sizeof(int), compare);
    
    //Increase time to min(processes-starttime)
    currentTime = time[timeDoneIndex++];
    
    int check = 1;

    while (check==1)
    {
        
        //printf("============ time = %d ============\n", currentTime);
        ////printf("---- **check if its time to start a new process**\n");
        for (int i=0;i<numProcesses;i++)
        {
            ////printf("---- i: %d\n",i);
            if (currentTime == allProcessesArray[i].startTime)
            {
                //if at a particular processes' starttime
                //printf("---- ARRIVAL event for process %d at t = %d ms\n", i, currentTime);
                //printf("---- Process %d starts at t = %d ms\n", i, currentTime);
                strcpy(allProcessesArray[i].status, "READY");
                //printf("---- Process %d requests a core at t = %d ms\n", i, currentTime);
                //add process to readyqueue
                enqueue(allProcessesArray[i].id, &readyQueue);
                //printf("---- Process %d added to ready queue at t = %d ms\n", i, currentTime);
                
            }
        }
        

        ////printf("---- **check if event of an active process just  completed**\n");
        for (int i=numProcesses-1;i>=0;i--)
        {
            ////printf("---- i: %d\n",i);
            if (currentTime == allProcessesArray[i].endTimeCurrent)
            {
                char currEventType[MAXSTATUSCHARS];
                strcpy(currEventType, allProcessesArray[i].eventList[allProcessesArray[i].currentEvent].type);
                //printf("---- %s completion event for process %d at t = %dms\n", currEventType, i, currentTime );
                //Mark current event of that process as done
                strcpy(allProcessesArray[i].eventList[allProcessesArray[i].currentEvent].status, "done");
                
                //mark the resource used by completed resource as free.
                //dont neet to do this, I can just compare the busy until time.
                
                //If there is a nextevent for that process exists i.e. if next is valid
                if (allProcessesArray[i].eventList[allProcessesArray[i].currentEvent+1].valid==1)
                {
                    char nextEventType[MAXSTATUSCHARS];
                    strcpy(nextEventType, allProcessesArray[i].eventList[allProcessesArray[i].currentEvent+1].type);
                    int dur = allProcessesArray[i].eventList[allProcessesArray[i].currentEvent+1].duration;
                    
                    //printf ("---- next event - %s", nextEventType);
                    while (dur == 0 && strcmp(nextEventType, "DISK")==0)
                    {
                        allProcessesArray[i].currentEvent++;
                        //printf ("**** process %d makes a zero delay disk access\n",i);
                        strcpy(nextEventType, allProcessesArray[i].eventList[allProcessesArray[i].currentEvent+1].type);
                        dur = allProcessesArray[i].eventList[allProcessesArray[i].currentEvent+1].duration;
                    }
                    
                    if (strncmp(nextEventType, "CORE",4)==0)
                    {
                        //if nextevent = core, Add the process to readyqueue
                        //printf("---- Process %d requests a core at t = %d ms for %d ms\n", i, currentTime, dur);
                        enqueue(i, &readyQueue);
                        strcpy(allProcessesArray[i].status, "READY");
                        //printf("---- Process %d added to ready queue at t = %d ms\n", i, currentTime);
                    }
                    else if (strcmp(nextEventType, "DISK")==0)
                    {
                        enqueue(i, &diskQueue);
                        //printf("---- Process %d requests a disk at t = %d ms for %d ms\n", i, currentTime, dur);
                        strcpy(allProcessesArray[i].status, "BLOCKED");
                        //printf("---- Process %d added to disk queue at t = %d ms\n", i, currentTime);
                    }
                    else if ((strncmp(nextEventType, "DISPLAY",7)==0) || (strncmp(nextEventType, "INPUT",5)==0))// (strcmp(nextEventType, "DISPLAYP")==0) ||
                    {
                        //TODO: write this subroutine
                        HandleDisplayInputRequest(allProcessesArray[i].id);
                        strcpy(allProcessesArray[i].status, "BLOCKED");
                    }
                }
                //TODO: I need to move this part to after new event starts.
                else //if process has exausted its events
                {
                    //mark process as terminated
                    strcpy(allProcessesArray[i].status, "TERMINATED");
                    //printf("---- Process %d terminates at t = %d\n" , i, currentTime);
                    //shedule a print termination report
                    //printf ("---- terminationIndex = %d\n", terminationIndex);
                    terminationReportRequired[++terminationIndex] = i;
                }
            }
        }
        for(int i = 0; i < nCores; i++)
        {
            if (arrayOfCores[i].busyUntilTime <= currentTime && readyQueue.itemCount>0)
            {
                //printf("---- Core %d is free and ready queue has %d processes waiting\n", i, readyQueue.itemCount);
                int procID = dequeue(&readyQueue);
                HandleCoreRequest(i, procID);
                //printf("---- Process %d gets a core %d at t = %d ms\n", procID, i, currentTime);
            }
        }
        if (singleDisk.busyUntilTime <= currentTime && diskQueue.itemCount>0)
        {
            //printf("---- Disk is free and disk queue has %d processes waiting\n", diskQueue.itemCount);
            int procID = dequeue(&diskQueue);
            HandleDiskRequest(procID);
            //printf("---- Process %d gets disk at t = %d ms\n", procID, currentTime);
        }
        //print reports for processes that have terminated in this loop
        int nFreeCores = 0;
        for(int i = 0; i < nCores; i++)
        {
            //printf("---- core %d busy until%d\n", i, arrayOfCores[i].busyUntilTime);
            if (arrayOfCores[i].busyUntilTime > currentTime)
                nFreeCores++;
        }
        char diskState[MAXSTATUSCHARS];
        if (singleDisk.busyUntilTime <= currentTime)
        {
            strcpy(diskState, "IDLE");
        }
        else
        {
            strcpy(diskState, "BUSY");
        }
        int termind = 0;
        while (termind <= terminationIndex)
        {
            int procID = terminationReportRequired[termind];
            
            //printf("---- process %d terminates at t = %d\n", procID, currentTime);
            
            printf("\nCURRENT STATE OF THE SYSTEM AT t = %d\n", currentTime);
            printf("Current number of busy cores: %d\n", nFreeCores);
            printf("Disk is %s\n", diskState);
            
            //printf ("---- --%s front:%d back:%d count:%d\n", readyQueue.name, readyQueue.front, readyQueue.rear, readyQueue.itemCount);
            printf("Ready queue contains processes: \n[");
            int readyIndex = readyQueue.front;
            int count = 0;
            while (count < readyQueue.itemCount)
            {
                printf("%d  ", readyQueue.intArray[readyIndex]);
                readyIndex = (readyIndex + 1) % MAXPROCESSES;
                count++;
            }
            //printf("%d", readyQueue.intArray[readyQueue.rear]);
            printf("]\n");
            
            printf("Disk queue contains processes: \n[");
            int diskIndex = diskQueue.front;
            count = 0;
            while (count < diskQueue.itemCount)
            {
                printf("%d, ", diskQueue.intArray[diskIndex]);
                diskIndex = (diskIndex + 1) % MAXPROCESSES;
                count++;
            }
            //printf("%d", diskQueue.intArray[diskQueue.rear]);
            printf("]\n");
            
            printf("%-12s%-12s%-12s\n", "Process ID", "Start time", "Status");
            for (int i = 0; i < numProcesses; i++)
            {
                if (allProcessesArray[i].terminatedBefore == 0)
                {
                    printf("%-12d%-12d%-12s\n", i, allProcessesArray[i].startTime, allProcessesArray[i].status);
                }
            }
            allProcessesArray[procID].terminatedBefore = 1;
                
            termind ++;
        }
        terminationIndex = -1;
        //printf("\n");
                                            
                                            
        //increase current time to next time in time array
        //min of (all cores', and discs' 'BusyUntilTime', unstarted_processes' starttime)
        qsort( time, timeAddIndex, sizeof(int), compare);
        
        for (int i = 0; i < MAXLINES; i++)
        {
            if(time[i]>currentTime)
            {
                currentTime = time[i];
                break;
            }
            
            if(time[i] == INT_MAX)
            {
                break;//i dont think this will ever get executed.
            }
        }
                                            
        //break out of loop if all process is completed
        int stayAlive = 1;
        for (int i=0;i<MAXPROCESSES;i++)
        {
            if (currentTime == INT_MAX)//i.e if there is a process that is not terminated
            {
                stayAlive = 0;
            }
        }
        if (stayAlive == 0)
        {
            //printf("---- time up all processes - ending program\n");
            break;
        }
    }
    return 0;
}
