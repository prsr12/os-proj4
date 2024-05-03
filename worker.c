
#include "header.h"

bool hasResourcesToRelease ( int arr[] );
bool canRequestMore ( int arr1[], int arr2[] );

int main ( int argc, char *argv[] ) {

        int i, j;
        int totalProcessLimit = 100;
        int myPid = getpid();
        int ossPid = getppid();
        int processIndex;


        int maxClaimVector[20];
        int allocatedVector[20];


        if ( signal ( SIGINT, handle ) == SIG_ERR ) {
                perror ( "USER: signal failed." );
        }


        shmClockKey = 2001;
        if ( ( shmClockID = shmget ( shmClockKey, ( 2 * ( sizeof ( unsigned int ) ) ), 0666 ) ) == -1 ) {
                perror ( "USER: Failure to find shared memory space for simulated clock." );
                return 1;
        }

        if ( ( shmClock = (unsigned int *) shmat ( shmClockID, NULL, 0 ) ) < 0 ) {
                perror ( "USER: Failure to attach to shared memory space for simulated clock." );
                return 1;
        }

        shmBlockedKey = 2004;
        if ( ( shmBlockedID = shmget ( shmBlockedKey, ( totalProcessLimit * ( sizeof ( int ) ) ), 0666 ) ) == -1 ) {
                perror ( "USER: Failure to find shared memory space for blocked USER process array." );
                return 1;
        }

        if ( ( shmBlocked = (int *) shmat ( shmBlockedID, NULL, 0 ) ) < 0 ) {
                perror ( "USER: Failure to attach to shared memory space for blocked USER process array." );
                return 1;
        }


        messageKey = 6969;
        if ( ( messageID = msgget ( messageKey, IPC_CREAT | 0666 ) ) == -1 ) {
                perror ( "USER: Failure to create the message queue." );
                return 1;
        }
        time_t processSeed;
        srand ( ( int ) time ( &processSeed ) % getpid() );

        const int probUpper = 100;
        const int probLower = 1;
        const int requestProb = 100;
        const int releaseProb = 55;
        const int terminateProb = 10;

        maxClaimVector[0] = atoi ( argv[1] );
        maxClaimVector[1] = atoi ( argv[2] );
        maxClaimVector[2] = atoi ( argv[3] );
        maxClaimVector[3] = atoi ( argv[4] );
        maxClaimVector[4] = atoi ( argv[5] );
        maxClaimVector[5] = atoi ( argv[6] );
        maxClaimVector[6] = atoi ( argv[7] );
        maxClaimVector[7] = atoi ( argv[8] );
        maxClaimVector[8] = atoi ( argv[9] );
        maxClaimVector[9] = atoi ( argv[10] );
        maxClaimVector[10] = atoi ( argv[11] );
        maxClaimVector[11] = atoi ( argv[12] );
        maxClaimVector[12] = atoi ( argv[13] );
        maxClaimVector[13] = atoi ( argv[14] );
        maxClaimVector[14] = atoi ( argv[15] );
        maxClaimVector[15] = atoi ( argv[16] );
        maxClaimVector[16] = atoi ( argv[17] );
        maxClaimVector[17] = atoi ( argv[18] );
        maxClaimVector[18] = atoi ( argv[19] );
        maxClaimVector[19] = atoi ( argv[20] );
        processIndex = atoi ( argv[21] );

        for ( i = 0; i < 20; ++i ) {
                allocatedVector[i] = 0;
        }


        bool waitingOnRequest = false;

        int randomAction;
        int selectedResource;
        bool validResource;


        while ( 1 ) {
                if ( shmBlocked[processIndex] == 0 && waitingOnRequest == false ) {



                        if ( !canRequestMore ( maxClaimVector, allocatedVector ) ) {
                                message.msg_type = 5;
                                message.pid = myPid;
                                message.tableIndex = processIndex;
                                message.request = 0;
                                message.release = 0;
                                message.terminate = true;
                                message.resourceGranted = false;
                                message.messageTime[0] = shmClock[0];
                                message.messageTime[1] = shmClock[1];

                                if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                        perror ( "USER: Failure to send message." );
                                }

                                exit ( EXIT_SUCCESS );
                        }

                        randomAction = ( rand() % ( probUpper - probLower + 1 ) + probLower );

                        if ( randomAction > releaseProb && randomAction <= requestProb ) {
                                validResource = false;

                                while ( validResource == false ) {
                                        selectedResource = ( rand() % ( 19 - 0 + 1 ) + 0 );
                                        if ( allocatedVector[selectedResource] < maxClaimVector[selectedResource] ) {
                                                validResource = true;
                                        }
                                }

                                message.msg_type = 5;
                                message.pid = myPid;
                                message.tableIndex = processIndex;
                                message.request = selectedResource;
                                message.release = -1;
                                message.terminate = false;
                                message.resourceGranted = false;
                                message.messageTime[0] = shmClock[0];
                                message.messageTime[1] = shmClock[1];

                                if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                        perror ( "USER: Failure to send message." );
                                }

                                waitingOnRequest = true;
                        }


                        if ( randomAction > terminateProb && randomAction <= releaseProb ) {
                                validResource = false;


                                if ( hasResourcesToRelease ( allocatedVector ) ) {
                                        while ( validResource == false ) {
                                                selectedResource = ( rand() % ( 19 - 0 + 1 ) + 0 );
                                                if ( allocatedVector[selectedResource] > 0 ) {
                                                        validResource = true;
                                                }
                                        }
                                        message.msg_type = 5;
                                        message.pid = myPid;
                                        message.tableIndex = processIndex;
                                        message.request = -1;
                                        message.release = selectedResource;
                                        message.terminate = false;
                                        message.resourceGranted = false;
                                        message.messageTime[0] = shmClock[0];
                                        message.messageTime[1] = shmClock[1];

                                        if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                                perror ( "USER: Failure to send message." );
                                        }

                                        allocatedVector[selectedResource]--;
                                }
                        }


                        if ( randomAction > 0 && randomAction <= terminateProb ) {

                                message.msg_type = 5;
                                message.pid = myPid;
                                message.tableIndex = processIndex;
                                message.request = -1;
                                message.release = -1;
                                message.terminate = true;
                                message.resourceGranted = false;
                                message.messageTime[0] = shmClock[0];
                                message.messageTime[1] = shmClock[1];

                                if ( msgsnd ( messageID, &message, sizeof ( message ), 0 ) == -1 ) {
                                        perror ( "USER: Failure to send message." );
                                }


                                exit ( EXIT_SUCCESS );
                        }
                }

                msgrcv ( messageID, &message, sizeof ( message ), myPid, IPC_NOWAIT );

                if ( message.resourceGranted == true ) {
                        waitingOnRequest = false;
                        allocatedVector[selectedResource]++;
                }

        }

        return 0;
}

void handle ( int sig_num ) {
        if ( sig_num == SIGINT ) {
                printf ( "%d: Signal to terminate process was received.\n", getpid() );
                exit ( 0 );
        }
}
bool canRequestMore ( int arr1[], int arr2[] ) {
        int i;
        int sum1 = 0;
        int sum2 = 0;

        for ( i = 0; i < 20; ++i ) {
                sum1 += arr1[i];
                sum2 += arr2[i];
        }

        if ( sum2 < sum1 )
                return true;
        else
                return false;
}

bool hasResourcesToRelease ( int arr[] ) {
        int i;
        int sum = 0;
        for ( i = 0; i < 20; ++i ) {
                sum += arr[i];
        }

        if ( sum > 0 )
                return true;
        else
                return false;
}
