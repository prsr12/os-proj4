#include "header.h"

typedef struct {
        int front, rear, size;
        unsigned capacity;
        int *array;
        int pid;
} Queue;


Queue* createQueue ( unsigned capacity );
int isFull ( Queue* queue );
int isEmpty ( Queue* queue );
void enqueue ( Queue* queue, int item );
int dequeue ( Queue* queue );
int front ( Queue* queue );
int rear ( Queue* queue );


void calculateNeed ( int need[maxProcesses][maxResources], int maximum[maxProcesses][maxResources], int allot[maxProcesses][maxResources] );
bool isSafeState ( int available[], int maximum[][maxResources], int allot[][maxResources] );
void incrementClock ( unsigned int shmClock[] );
void printAllocatedResourcesTable( int num1, int array[][maxResources] );
void printMaxClaimTable( int num1, int array[][maxResources] );
void terminateIPC();

int totalResourcesRequested;
int totalRequestsGranted;
int totalSafeStateChecks;
int totalResourcesReleased;
int totalProcessesCreated;
int totalProcessesTerminated;

const int maxRunningProcesses = 18;
const int totalProcessLimit = 100;
const int maxAmountOfEachResource = 4;
FILE *fp;

int main ( int argc, char *argv[] ) {

        int i, j;
        srand ( time ( NULL ) );
        unsigned int newProcessTime[2] = { 0, 0 };
        totalProcessesCreated = 0;
        int myPid = getpid();


        int numberOfLines = 0;
        char logName[20] = "logfile.txt";
        fp = fopen ( logName, "w+" );

        const int killTimer = 5;
        alarm ( killTimer );
        if ( signal ( SIGINT, handle ) == SIG_ERR ) {
                perror ( "OSS: ctrl-c signal failed." );
        }

        if ( signal ( SIGALRM, handle ) == SIG_ERR ) {
                perror ( "OSS: alarm signal failed." );
        }

        shmClockKey = 2001;
        if ( ( shmClockID = shmget ( shmClockKey, ( 2 * ( sizeof ( unsigned int ) ) ), IPC_CREAT | 0666 ) ) == -1 ) {
                perror ( "OSS: Failure to create shared memory space for simulated clock." );
                return 1;
        }

        shmBlockedKey = 2004;
        if ( ( shmBlockedID = shmget ( shmBlockedKey, ( totalProcessLimit * ( sizeof ( int ) ) ), IPC_CREAT | 0666 ) ) == -1 ) {
                perror ( "OSS: Failure to create shared memory space for blocked USER process array." );
                return 1;
        }


        if ( ( shmClock = (unsigned int *) shmat ( shmClockID, NULL, 0 ) ) < 0 ) {
                perror ( "OSS: Failure to attach to shared memory space for simulated clock." );
                return 1;
        }
        shmClock[0] = 0;
        shmClock[1] = 0;

        if ( ( shmBlocked = (int *) shmat ( shmBlockedID, NULL, 0 ) ) < 0 ) {
                perror ( "OSS: Failure to attach to shared memory space for blocked USER process array." );
                return 1;
        }


        for ( i = 0; i < totalProcessLimit; ++i ) {
                shmBlocked[i] = 0;
        }


        messageKey = 6969;
        if ( ( messageID = msgget ( messageKey, IPC_CREAT | 0666 ) ) == -1 ) {
                return 1;
        }


        int totalResourceTable[20];
        for ( i = 0; i < 20; ++i ) {
                totalResourceTable[i] = ( rand() % ( 10 - 1 + 1 ) + 1 );
        }


        int maxClaimTable[totalProcessLimit][20];
        for ( i = 0; i < totalProcessLimit; ++i ) {
                for ( j = 0; j < 20; ++j ) {
                        maxClaimTable[i][j] = 0;
                }
        }


        int allocatedTable[totalProcessLimit][20];
        for ( i = 0; i < totalProcessLimit; ++i ) {
                for ( j = 0; j < 20; ++j ) {
                        allocatedTable[i][j] = 0;
                }
        }


        int availableResourcesTable[20];
        for ( i = 0; i < 20; ++i ) {
                availableResourcesTable[i] = totalResourceTable[i];
        }


        int requestedResourceTable[totalProcessLimit];
        for ( i = 0; i < totalProcessLimit; ++i ) {
                requestedResourceTable[i] = -1;
        }
int opt;
        const char opstr[] = "n:s:i:f";
        int proc = 0;
        int simul = 0;
        int interval = 1;
        char *filername = NULL;

while ((opt = getopt(argc, argv, opstr)) != -1) {
                switch(opt) {
                        case 'n':
                                proc = atoi(optarg);
                                if (proc > 18) {
                                        printf("The max number of processes is 18\n");
                                        exit(1);
                                }
                                break;
                        case 's':
                                simul = atoi(optarg);
                                if (simul > 18) {
                    printf("The max number of simultaneous processes is 18\n");
                    exit(1);
                }
                        case 'i':
                                interval = atoi(optarg);
                                break;
                        case 'f':
                                filername = optarg;
                                break;
                                
                        default:
                                exit(EXIT_FAILURE);
                }
        }



        pid_t pid;
        int processIndex = 0;
        int currentProcesses = 0;
        unsigned int nextRandomProcessTime;
        unsigned int nextProcessTimeBound = 5000;
        bool timeCheck, processCheck;
        bool createProcess;

        Queue* blockedQueue = createQueue ( totalProcessLimit );


        int tempPid;
        int tempIndex;
        int tempRequest;
        int tempRelease;
        bool tempTerminate;
        bool tempGranted;
        unsigned int tempClock[2];
        unsigned int receivedTime[2];
        int tempHolder;


        while ( 1 ) {


                if ( numberOfLines >= 10000 ) {
                        kill ( getpid(), SIGINT );
                }

                createProcess = false;


                if ( shmClock[0] > newProcessTime[0] || ( shmClock[0] == newProcessTime[0] && shmClock[1] >= newProcessTime[1] ) ) {
                        timeCheck = true;
                } else {
                        timeCheck = false;
                }


                if ( ( currentProcesses < maxRunningProcesses ) && ( totalProcessesCreated < totalProcessLimit ) ) {
                        processCheck = true;
                } else {
                        processCheck = false;
                }


                if ( ( timeCheck == true ) && ( processCheck == true ) ) {
                        createProcess = true;
                }


                if ( createProcess ) {
                        processIndex = totalProcessesCreated;
                        for ( i = 0; i < 20; ++i ) {
                                maxClaimTable[processIndex][i] = ( rand() % ( maxAmountOfEachResource - 1 + 1 ) + 1 );
                        }

                        fprintf ( fp, "Max Claim Vector for new newly generated process: Process %d\n", processIndex);
                        printf ("Max Claim Vector for new newly generated process: Process %d\n", processIndex);
                        for ( i = 0; i < 20; ++i ) {
                                fprintf ( fp, "%d: %d\t", i, maxClaimTable[processIndex][i] );
                                printf ( "%d: %d\t", i, maxClaimTable[processIndex][i] );
                        }
                        fprintf ( fp, "\n" );
                        
                        pid = fork();

                        if ( pid < 0 ) {
                                kill ( getpid(), SIGINT );
                        }


                        if ( pid == 0 ) {

                                char intBuffer0[3], intBuffer1[3], intBuffer2[3], intBuffer3[3], intBuffer4[3];
                                char intBuffer5[3], intBuffer6[3], intBuffer7[3], intBuffer8[3], intBuffer9[3];
                                char intBuffer10[3], intBuffer11[3], intBuffer12[3], intBuffer13[3], intBuffer14[3];
                                char intBuffer15[3], intBuffer16[3], intBuffer17[3], intBuffer18[3], intBuffer19[3];
                                char intBuffer20[3];


                                sprintf ( intBuffer0, "%d", maxClaimTable[processIndex][0] );
                                sprintf ( intBuffer1, "%d", maxClaimTable[processIndex][1] );
                                sprintf ( intBuffer2, "%d", maxClaimTable[processIndex][2] );
                                sprintf ( intBuffer3, "%d", maxClaimTable[processIndex][3] );
                                sprintf ( intBuffer4, "%d", maxClaimTable[processIndex][4] );
                                sprintf ( intBuffer5, "%d", maxClaimTable[processIndex][5] );
                                sprintf ( intBuffer6, "%d", maxClaimTable[processIndex][6] );
                                sprintf ( intBuffer7, "%d", maxClaimTable[processIndex][7] );
                                sprintf ( intBuffer8, "%d", maxClaimTable[processIndex][8] );
                                sprintf ( intBuffer9, "%d", maxClaimTable[processIndex][9] );
                                sprintf ( intBuffer10, "%d", maxClaimTable[processIndex][10] );
                                sprintf ( intBuffer11, "%d", maxClaimTable[processIndex][11] );
                                sprintf ( intBuffer12, "%d", maxClaimTable[processIndex][12] );
                                sprintf ( intBuffer13, "%d", maxClaimTable[processIndex][13] );
                                sprintf ( intBuffer14, "%d", maxClaimTable[processIndex][14] );
                                sprintf ( intBuffer15, "%d", maxClaimTable[processIndex][15] );
                                sprintf ( intBuffer16, "%d", maxClaimTable[processIndex][16] );
                                sprintf ( intBuffer17, "%d", maxClaimTable[processIndex][17] );
                                sprintf ( intBuffer18, "%d", maxClaimTable[processIndex][18] );
                                sprintf ( intBuffer19, "%d", maxClaimTable[processIndex][19] );
                                sprintf ( intBuffer20, "%d", processIndex );

                                fprintf ( fp, "OSS: Process %d (PID: %d) was created at %d:%d.\n", processIndex,
                                         getpid(), shmClock[0], shmClock[1] );
                                printf ("OSS: Process %d (PID: %d) was created at %d:%d.\n", processIndex,
                                         getpid(), shmClock[0], shmClock[1] );
                                numberOfLines++;

                                execl ( "./worker", "worker", intBuffer0, intBuffer1, intBuffer2, intBuffer3,
                                       intBuffer4, intBuffer5, intBuffer6, intBuffer7, intBuffer8,
                                       intBuffer9, intBuffer10, intBuffer11, intBuffer12, intBuffer13,
                                       intBuffer14, intBuffer15, intBuffer16, intBuffer17, intBuffer18,
                                       intBuffer19, intBuffer20, NULL );

                                exit ( 127 );
                        }

                        newProcessTime[0] = shmClock[0];
                        newProcessTime[1] = shmClock[1];
                        nextRandomProcessTime = ( rand() % ( nextProcessTimeBound - 1 + 1 ) + 1 );
                        newProcessTime[1] += nextRandomProcessTime;
                        newProcessTime[0] += newProcessTime[1] / 1000000000;
                        newProcessTime[1] = newProcessTime[1] % 1000000000;


                        timeCheck = false;
                        processCheck = false;


                        currentProcesses++;
                        totalProcessesCreated++;
                }

                msgrcv ( messageID, &message, sizeof( message ), 5, IPC_NOWAIT );

                tempPid = message.pid;
                tempIndex = message.tableIndex;
                tempRequest = message.request;
                tempRelease = message.release;
                tempTerminate = message.terminate;
                tempGranted = message.resourceGranted;
                tempClock[0] = message.messageTime[0];
                tempClock[1] = message.messageTime[1];

                if ( tempRequest != -1 ) {
                        fprintf ( fp, "OSS: Process %d requested Resource %d at %d:%d.\n", tempIndex,
                                 tempRequest, tempClock[0], tempClock[1] );
                        printf ("OSS: Process %d requested Resource %d at %d:%d.\n", tempIndex,
                                 tempRequest, tempClock[0], tempClock[1] );
                        numberOfLines++;
                        totalResourcesRequested++;


                        requestedResourceTable[tempIndex] = tempRequest;


                        allocatedTable[tempIndex][tempRequest]++;
                        availableResourcesTable[tempRequest]--;


                        if (isSafeState ( availableResourcesTable, maxClaimTable, allocatedTable ) ) {
                                totalRequestsGranted++;
                                message.msg_type = tempIndex;
                                message.pid = getpid();
                                message.tableIndex = tempIndex;
                                message.request = -1;
                                message.release = -1;
                                message.terminate = false;
                                message.resourceGranted = true;
                                message.messageTime[0] = shmClock[0];
                                message.messageTime[1] = shmClock[1];

                                if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                        perror ( "OSS: Failure to send message." );
                                }

                                fprintf ( fp, "OSS: Process %d was granted its request of Resource %d at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                printf ("OSS: Process %d was granted its request of Resource %d at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                numberOfLines++;
                        }

                        else {

                                allocatedTable[tempIndex][tempRequest]--;
                                availableResourcesTable[tempRequest]++;


                                enqueue ( blockedQueue, tempIndex );


                                shmBlocked[tempIndex] = 1;

                                fprintf ( fp, "OSS: Process %d was denied its request of Resource %d and was blocked at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                printf ("OSS: Process %d was denied its request of Resource %d and was blocked at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                numberOfLines++;
                        }

                        incrementClock ( shmClock );
                }


                if ( tempRelease != -1 ) {
                        fprintf ( fp, "OSS: Process %d indicated that it was releasing some of Resource %d at %d:%d.\n",
                                 tempIndex, tempRelease, tempClock[0], tempClock[1] );
                        printf ("OSS: Process %d indicated that it was releasing some of Resource %d at %d:%d.\n",
                                 tempIndex, tempRelease, tempClock[0], tempClock[1] );
                        numberOfLines++;

                        totalResourcesReleased++;
                        allocatedTable[tempIndex][tempRelease]--;
                        availableResourcesTable[tempRelease]++;

                        fprintf ( fp, "OSS: Process %d release notification was handled at %d:%d.\n", tempIndex, shmClock[0],
                                 shmClock[1] );
                        printf ("OSS: Process %d release notification was handled at %d:%d.\n", tempIndex, shmClock[0],
                                 shmClock[1] );
                        numberOfLines++;
                        incrementClock ( shmClock );
                }


                if ( tempTerminate == true ) {
                        fprintf ( fp, "OSS: Process %d terminated at %d:%d.\n", tempIndex, tempClock[0], tempClock[1] );
                        printf ("OSS: Process %d terminated at %d:%d.\n", tempIndex, tempClock[0], tempClock[1] );
                        numberOfLines++;

                        for ( i = 0; i < 20; ++i ) {
                                tempHolder = allocatedTable[tempIndex][i];
                                allocatedTable[tempIndex][i] = 0;
                                availableResourcesTable[i] += tempHolder;
                        }
                        currentProcesses--;

                        fprintf ( fp, "OSS: Process %ds termination notification was handled at %d:%d.\n", tempIndex,
                                 shmClock[0], shmClock[1] );
                        printf ("OSS: Process %ds termination notification was handled at %d:%d.\n", tempIndex,
                                 shmClock[0], shmClock[1] );
                        numberOfLines++;
                        incrementClock ( shmClock );
                }


                if ( !isEmpty ) {

                        tempIndex = dequeue ( blockedQueue );
                        tempRequest = requestedResourceTable[tempIndex];
                        totalResourcesRequested++;


                        allocatedTable[tempIndex][tempRequest]++;
                        availableResourcesTable[tempRequest]--;


                        if (isSafeState ( availableResourcesTable, maxClaimTable, allocatedTable ) ) {
                                totalRequestsGranted++;
                                message.msg_type = tempIndex;
                                message.pid = getpid();
                                message.tableIndex = tempIndex;
                                message.request = -1;
                                message.release = -1;
                                message.terminate = false;
                                message.resourceGranted = true;
                                message.messageTime[0] = shmClock[0];
                                message.messageTime[1] = shmClock[1];

                                if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                        perror ( "OSS: Failure to send message." );
                                }


                                shmBlocked[tempIndex] = 0;

                                fprintf ( fp, "OSS: Process %d was granted its request of Resource %d at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                printf ("OSS: Process %d was granted its request of Resource %d at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                numberOfLines++;
                        } else {

                                allocatedTable[tempIndex][tempRequest]--;
                                availableResourcesTable[tempRequest]++;


                                enqueue ( blockedQueue, tempIndex );


                                shmBlocked[tempIndex] = 1;

                                fprintf ( fp, "OSS: Process %d was denied it's request of Resource %d and was blocked at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                printf ("OSS: Process %d was denied it's request of Resource %d and was blocked at %d:%d.\n",
                                         tempIndex, tempRequest, shmClock[0], shmClock[1] );
                                numberOfLines++;
                        }
                        incrementClock ( shmClock );
                }

                incrementClock ( shmClock );

                if ( numberOfLines % 20 == 0 ) {

                        fprintf ( fp, "Currently Allocated Resources\n" );
                        fprintf ( fp, "\tR0\tR1\tR2\tR3\tR4\tR5\tR6\tR7\tR8\tR9\tR10\tR11\tR12\tR13\tR14\tR15\tR16\tR17\tR18\tR19\n" );
                        printf ("Currently Allocated Resources\n" );
                        printf ("\tR0\tR1\tR2\tR3\tR4\tR5\tR6\tR7\tR8\tR9\tR10\tR11\tR12\tR13\tR14\tR15\tR16\tR17\tR18\tR19\n" );
                        for ( i = 0; i < totalProcessesCreated; ++i ) {
                                fprintf ( fp, "P%d:\t", i );
                                printf ("P%d:\t", i );
                                for ( j = 0; j < 20; ++j ) {
                                        fprintf ( fp, "%d\t", allocatedTable[i][j] );
                                        printf ("%d\t", allocatedTable[i][j] );
                                }
                                fprintf ( fp, "\n" );
                                printf ("\n" );
                                
                        }
                }

        }


        terminateIPC();

        return 0;
}
void calculateNeed ( int need[maxProcesses][maxResources], int maximum[maxProcesses][maxResources], int allot[maxProcesses][maxResources] ) {
        int i, j;
        for ( i = 0; i < maxProcesses; ++i ) {
                for ( j = 0; j < maxResources; ++j ) {
                        need[i][j] = maximum[i][j] - allot[i][j];
                }
        }
}

bool isSafeState ( int available[], int maximum[][maxResources], int allot[][maxResources] ) {
        totalSafeStateChecks++;
        int index;

        int need[maxProcesses][maxResources];
        calculateNeed ( need, maximum, allot );

        bool finish[maxProcesses] = { 0 };


        int work[maxResources];
        int i;
        for ( i = 0; i < maxResources; ++i ) {
                work[i] = available[i];
        }

        int count = 0;

        while ( count < maxProcesses ) {
                int p;
                bool found = false;
                for ( p = 0; p < maxProcesses; ++p ) {
                        if ( finish[p] == 0 ) {
                                int j;
                                for ( j = 0; j < maxResources; ++j ) {
                                        if ( need[p][j] > work[j] )
                                            break;
                                }
                                if ( j == maxResources ) {
                                        int k;
                                        for ( k = 0; k < maxResources; ++k ) {
                                                work[k] += allot[p][k];
                                        }
                                        finish[p] = 1;
                                        found = true;
                                }
                        }
                }

                if ( found == false ) {

                        return false;
                }
        }

        return true;

}


void handle ( int sig_num ) {
        if ( sig_num == SIGINT || sig_num == SIGALRM ) {
                terminateIPC();
                kill ( 0, SIGKILL );
                wait ( NULL );
                exit ( 0 );
        }
}


void printAllocatedResourcesTable( int num1, int array[][maxResources] ) {
        int i, j;
        num1 = totalProcessesCreated;

        printf ( "Currently Allocated Resources\n" );
        printf ( "\tR0\tR1\tR2\tR3\tR4\tR5\tR6\tR7\tR8\tR9\tR10\tR11\tR12\tR13\tR14\tR15\tR16\tR17\tR18\tR19\n" );
        for ( i = 0; i < totalProcessesCreated; ++i ) {
                printf ( "P%d:\t", i );
                for ( j = 0; j < 20; ++j ) {
                        printf ( "%d\t", array[i][j] );
                }
                printf ( "\n" );
        }
}


void printMaxClaimTable( int num1, int array[][maxResources] ){
        int i, j;
        num1 = totalProcessesCreated;

        printf ( "Max Claim Table\n" );
        printf ( "\tR0\tR1\tR2\tR3\tR4\tR5\tR6\tR7\tR8\tR9\tR10\tR11\tR12\tR13\tR14\tR15\tR16\tR17\tR18\tR19\n" );
        for ( i = 0; i < totalProcessesCreated; ++i ) {
                printf ( "P%d:\t", i );
                for ( j = 0; j < 20; ++j ) {
                        printf ( "%d\t", array[i][j] );
                }
                printf ( "\n" );
        }
}


void incrementClock ( unsigned int shmClock[] ) {
        int processingTime = 5000;
        shmClock[1] += processingTime;

        shmClock[0] += shmClock[1] / 1000000000;
        shmClock[1] = shmClock[1] % 1000000000;
}

void terminateIPC() {

        fclose ( fp );


        shmdt ( shmClock );
        shmdt ( shmBlocked );

        shmctl ( shmClockID, IPC_RMID, NULL );
        shmctl ( shmBlockedID, IPC_RMID, NULL );

        msgctl ( messageID, IPC_RMID, NULL );
}


Queue* createQueue ( unsigned capacity ) {
        Queue* queue = (Queue*) malloc ( sizeof ( Queue ) );
        queue->capacity = capacity;
        queue->front = queue->size = 0;
        queue->rear = capacity - 1;
        queue->array = (int*) malloc ( queue->capacity * sizeof ( int ) );

        return queue;
}

int isFull ( Queue* queue ) {
        return ( queue->size == queue->capacity );
}

int isEmpty ( Queue* queue ) {
        return ( queue->size == 0 );
}

void enqueue ( Queue* queue, int item ) {
        if ( isFull ( queue ) )
                return;

        queue->rear = ( queue->rear + 1 ) % queue->capacity;
        queue->array[queue->rear] = item;
        queue->size = queue->size + 1;
}

int dequeue ( Queue* queue ) {
        if ( isEmpty ( queue ) )
                return INT_MIN;

        int item = queue->array[queue->front];
        queue->front = ( queue->front + 1 ) % queue->capacity;
        queue->size = queue->size - 1;

        return item;
}

int front ( Queue* queue ) {
        if ( isEmpty ( queue ) )
                return INT_MIN;

        return queue->array[queue->front];
}

int rear ( Queue* queue ) {
        if ( isEmpty ( queue ) )
                return INT_MIN;

        return queue->array[queue->rear];
}
