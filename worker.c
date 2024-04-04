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

typedef struct msgbuffer {
    long mtype;
    int intData;
} msgbuffer;

#define PERMS 0666

typedef struct Clock {
        int seconds;
        int nanoseconds;
} Clock;

#define SH_KEY 123456

int main(int argc, char** argv) {
        // Initialize shared memory space
        int shm_id = shmget(SH_KEY, sizeof(Clock), 0666);
        if (shm_id <= 0) {
                perror("Worker: Shared memory failiure");
                exit(1);
        }
        
        Clock *clock_ptr = (struct Clock*) shmat(shm_id, NULL, 0);
        if (clock_ptr == (void *) -1) {
                perror("Worker: Shared memoryfailiure");
                exit(1);
        }


        msgbuffer buf;
        buf.mtype = getpid();
        int msqid = 0;
        key_t msgkey;

       
        if ((msgkey = ftok("msg_key.txt", 1)) == -1) {

        exit(1);
    }
       
        if ((msqid = msgget(msgkey, PERMS)) == -1) {

                exit(1);
        }

        int t_seconds = atoi(argv[1]) + clock_ptr-> seconds;
        int t_nanoseconds = atoi(argv[2]) + clock_ptr-> nanoseconds;
        int elapsed_seconds = 0;
        int start_seconds = clock_ptr -> seconds;

        printf("WORKER PID:%d PPID:%d Called with oss: TermTimeS: %d TermTimeNano: %d\n", getpid(), getppid(), t_seconds, t_nanoseconds);
    printf("--Received message\n");


        while (true) {
                if (clock_ptr->seconds > t_seconds || (clock_ptr->seconds== t_seconds && clock_ptr->nanoseconds >= t_nanoseconds)) {
                        buf.intData = 0;
                } else {
                        buf.intData = 1;
                }


                buf.mtype = getppid();
                if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) == -1) {

                        exit(1);
                }
                if (buf.intData == 0) {
                        printf("Sending 0 to terminate child");
                break;
                }

                if (clock_ptr->seconds - start_seconds > elapsed_seconds) {
                elapsed_seconds = clock_ptr->seconds - start_seconds;
                printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n", getpid(), getppid(), clock_ptr->seconds, clock_ptr->nanoseconds, t_seconds, t_nanoseconds);
                printf("--%d seconds have passed since starting\n", elapsed_seconds);
        }
        }

        printf("WORKER: detaching shared memory\n");
        if (shmdt(clock_ptr) == -1) {
                fprintf(stderr, "Detatching process failed");
                exit(1);
        }
        return 0;
}
