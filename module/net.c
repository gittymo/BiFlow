/*  net.c
    Network library for biflow module
    ©2024 Morgan Evans */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadman.h"

void * WorkerThreadWorkFunction(void * w_ptr);

int main(int argc, char ** argv)
{
    srand(time(NULL));

    WorkerThreadManager * wtm = WorkerThreadManagerCreate();
    for (int i = 0; i < 4; i++) {
        WorkerThreadManagerAddWorkerThread(wtm, WorkerThreadWorkFunction, NULL);
    }
    
    WorkerThreadManagerStart(wtm);
    while (WorkerThreadManagerIsRunning(wtm)) {
        printf("Main Process: Waiting for work to be done.\n");
        sleep(5);
    }
    if (WorkerThreadManagerTasksComplete(wtm)) {
        printf("Successfully performed all tasks.\n");
    } else {
        printf("Some tasks did not completely successfully.\n");
    }
    WorkerThreadManagerFree(wtm);
    exit(EXIT_SUCCESS);
}

void * WorkerThreadWorkFunction(void * w_ptr)
{
    WorkerThread * w = (WorkerThread *) w_ptr;
    int sleep_time = 5 * (1 + (rand() % 12));
    printf("Worker thread (id %i) sleeping for %i seconds.\n",WorkerThreadGetId(w),sleep_time);
    WorkerThreadStartTask(w);
    time_t end_sleep = time(NULL) + sleep_time;
    while (time(NULL) < end_sleep && !WorkerThreadInterrupted(w)) {
        sleep(1);
    }
    WorkerThreadEndTask(w);
    if (!WorkerThreadWasKilled(w)) {
        printf("Worker thread (id %i) exited normally.\n",WorkerThreadGetId(w));
    } else {
        printf("Worker thread (id %i) exited due to high level kill.\n", WorkerThreadGetId(w));
    }

    return WorkerThreadExit(w);
}