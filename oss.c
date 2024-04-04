
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <time.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <signal.h> 
#include <sys/time.h> 
 
typedef struct msgbuffer { 
	long mtype; 
	int intData; 
} msgbuffer; 

#define PERMS 0666 


typedef struct PCB {
        int occupied;           
        pid_t pid;              
        int startSeconds;       
        int startNano;          
} PCB;

PCB procctable[20]; 
  
typedef struct Clock { 
	int seconds; 
	int nanoseconds; 
} Clock; 


#define SH_KEY 123456
#define nano_sec 1000000000
#define incrementation 10000000 
#define max_ns 999999999  

void incrementClock(Clock *clock_ptr) {
	clock_ptr-> nanoseconds += incrementation;
		 
		if (clock_ptr-> nanoseconds >= nano_sec) { 
			clock_ptr-> seconds += 1; 
			clock_ptr-> nanoseconds -= nano_sec;
		}
}	

void printPCB(PCB *procctable, Clock *clock_ptr) { 
	printf("OSS PID:%d SysClockS: %d SysClockNano: %d\n", getpid(), clock_ptr->seconds, clock_ptr->nanoseconds); 
	printf("Process Table: \n");
	printf("%-3s %-7s %-7s %-5s %-5s\n", "Entry", "Occupied", "PID", "StartS", "StartN"); 
	for (int i = 0; i < 20; i++) { 
		printf("%-3d %-10d %-9d %-5d %-5d\n", i, 
               procctable[i].occupied,
               procctable[i].pid,
               procctable[i].startSeconds,
               procctable[i].startNano);
    }
}

int randoms(int min, int max) { 
	if (min==max) { 
		return min; 
	} else { 
		return min + rand() / (RAND_MAX / (max - min + 1) + 1); 
	}  
} 

pid_t child_PID[200]; 
int num_child; 
int shm_id; 
void *clock_ptr;
bool shm_detached = false;  

void myhandler(int s) { 
	for (int i = 0; i < num_child; i++) {
		if (kill(child_PID[i], 0) != -1) {  
			if (kill(child_PID[i], SIGKILL) == -1) { 
				perror("Error killing all processes"); 
		} 
	}
} 
	if (!shm_detached) { 
		printf("Detatching shared mem");
    	
    	if (shmdt(clock_ptr) == -1) {
        	fprintf(stderr, "Detatching mem failiure\n");
        	exit(1);
    	}
		shm_detached = true; 
		
    	printf("Shared memory detached\n");
   		shmctl(shm_id, IPC_RMID, NULL);
    	printf("Freeing shared memory segment\n"); 
	}
	exit(0); 
}


int main(int argc, char **argv) {
	signal(SIGALRM, myhandler);
    signal(SIGINT, myhandler);
    alarm(30); 	

		int shm_id = shmget(SH_KEY, sizeof(Clock), IPC_CREAT | 0666); 
	if (shm_id <= 0) { 
		fprintf(stderr, "shared memeory failiure"); 
		exit(1); 
	} 

	Clock *clock_ptr = (struct Clock*) shmat(shm_id, NULL, 0);
	if (clock_ptr == (void *) -1) {
    	perror("OSS: Shared memory failiure");
    	exit(1);
	}
	
	 
	clock_ptr -> seconds = 0; 
	clock_ptr -> nanoseconds = 0; 
	msgbuffer buf; 
	int msqid; 
	key_t msgkey; 
	system("touch msg_key.txt"); 

	if ((msgkey = ftok("msg_key.txt", 1)) == -1) { 
			perror("ftok"); 
			exit(1); 
	}

	if ((msqid = msgget(msgkey, PERMS | IPC_CREAT)) == -1) {  
			exit(1); 
	}

		

	int opt;
	int processes, simul, timelimit, interval; 
	char *logfile = NULL; 
	
	while ((opt = getopt(argc, argv, "n:s:t:i:f:")) != -1) { 
		switch (opt) { 
			case 'n': 
				processes = atoi(optarg); 
				break; 
			case 's': 
				simul = atoi(optarg); 
				break; 
			case 't': 
				timelimit = atoi(optarg); 
				break; 
            case 'i':
                 interval = atoi(optarg);
                 break;
			case 'f': 
				logfile = optarg; 
				break;
			default: 
				exit(EXIT_FAILURE); 
		}
	}
	
	 
	srand(time(0)); 
	int random_seconds = randoms(1, timelimit);  
	int random_nanoseconds = randoms(0, 999999999); 
	char str_seconds[10], str_nanoseconds[10]; 
	sprintf(str_seconds, "%d", random_seconds); 
	sprintf(str_nanoseconds, "%d", random_nanoseconds); 
 
	FILE *fp = fopen(logfile, "w"); 
	if (fp == NULL) { 
			fprintf(stderr, "File error"); 
			exit(EXIT_FAILURE); 	
	}  
	int num_processes = 0; 
	int total_processes = 0; 
	int max_processes = processes; 
	int max_simul = simul;
	bool assigned = false; 
	int last_assigned = 0;   
	while (total_processes < max_processes) { 
		incrementClock(clock_ptr);
		int status;
        pid_t wpid;
        while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
        	if (WIFEXITED(status)) {
                printf("Child with PID %d terminated with status %d\n", wpid, WEXITSTATUS(status));
            	num_processes--;
                printf("Process terminated: %d. Running processes: %d.\n", wpid, num_processes);
                printf("Unassigning process: %d on PCB table\n", wpid); 	

				int i; 
				for (i = 0; i < 20; i++) { 
					if (procctable[i].occupied && procctable[i].pid == wpid) { 
						procctable[i].occupied = 0; 
						break;
					} 

				}
				printPCB(procctable, clock_ptr); 
			}
        }
		if (num_processes < max_simul) {
			pid_t pid = fork();

			if (pid < 0) {
				fprintf(stderr, "Fork error"); 
				exit(EXIT_FAILURE); 
			}
			if (pid == 0) { 
				execlp("./worker", "./worker", str_seconds, str_nanoseconds, NULL);
				perror("execlp");
			} else { 
				num_processes++; 
				total_processes++; 
				child_PID[num_child++] = pid;
				printf("Child process created: %d. Total processes: %d. Running processes: %d.\n", pid, total_processes, num_processes);
        		printf("Assigning PID %d .....\n", pid); 
				for (int i = last_assigned; i < 20; i++) {
        			if (!procctable[i].occupied) {
            			procctable[i].occupied = 1;
            			procctable[i].pid = pid;
            			procctable[i].startSeconds = clock_ptr->seconds;
            			procctable[i].startNano = clock_ptr->nanoseconds;
						last_assigned = i; 
						assigned = true;
						fprintf(fp, "OSS: Sending message to worker %d PID %d at time %d:%d\n", i, pid, clock_ptr->seconds, clock_ptr-> nanoseconds); 
						break; 
       				}
    			}
		}	
				for (int i = 0; i < num_processes; i++) {
   					msgrcv(msqid, &buf, sizeof(msgbuffer)- sizeof(long), 0, 0); 
						if (buf.intData == 0) {
							fprintf(fp, "OSS: Recieved termination message from worker %ld at %d: %d\n", buf.mtype, clock_ptr->seconds, clock_ptr-> nanoseconds);	
							fprintf(fp, "OSS: Planning to terminate worker %d PID %ld.\n", i, buf.mtype); 
							num_processes--;
						} else if (buf.intData == 1) {
							fprintf(fp, "OSS: Received message to continue from worker PID %ld at time %d:%d\n",  buf.mtype, clock_ptr->seconds, clock_ptr->nanoseconds); 
						}
					}

				if (num_processes == 0) { 
					printf("All children ended.\n"); 
					break;
				} 

				int i; 
				for (i = 0; i < 20; i++) {
					if (procctable[i].occupied && procctable[i].pid == buf.mtype) {
						break; 
					}
				}
				 
				if (i < 20) { 
					if (buf.intData == 0) { 
						procctable[i].occupied = 0; 
						printf("Process removed from PCB - %ld\n", buf.mtype);
					} else {
						printf("Process still running -%ld\n", buf.mtype);  
						procctable[i].startSeconds = clock_ptr->seconds; 
						procctable[i].startNano = clock_ptr-> nanoseconds; 
					}	
				printPCB(procctable, clock_ptr);
			} 
		}	
		}
if (num_processes == 0 && !shm_detached) {
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        exit(1);
    }
    if (shmdt(clock_ptr) == -1) {
        fprintf(stderr, "Detatching failiure\n");
        exit(1);
    }
    shmctl(shm_id, IPC_RMID, NULL);
    
	
	shm_detached = true;
} 
system("rm msg_key.txt");
fclose(fp); 
	
	return 0; 
} 
