#ifndef COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD
#define COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "global.h"

typedef struct _iworker_thread {
    int struct_id;
    int id;
    pthread_t handle;
    IThreadState state;
    IThreadPriority priority;
    void (* threadMainFunction)(struct _iworker_thread_job *);
    void (*jobSuccessCallbackFunction)(struct _iworker_thread_job *);
    void (*jobFailureCallbackFunction)(struct _iworker_thread_job *);
    time_t start_time, end_time;
    time_t job_run_time_history[10];
    size_t jobs_count, jobs_run;
    struct _iworker_thread_job * first_job, * last_job, * current_job;
    bool flag_exit_on_no_jobs;
    struct _iworker_thread_controller * controller;
    IThreadTimeout timeout;
} IWorkerThread;

void * IWorkerThreadRun(void * data);
IWorkerThread * IWorkerThreadCreate(  void (* workFunction)(IWorkerThreadJob *),
                                void (*successFunction)(IWorkerThreadJob *),
                                void (*failureFunction)(IWorkerThreadJob *),
                                IWorkerThreadController * itc);
IWorkerThreadJob * IWorkerThreadAddJob(IWorkerThread * itd, void * job_data);
time_t IWorkerThreadGetAverageJobTime(IWorkerThread * itd);
bool IWorkerThreadDone(IWorkerThread * iwt);
bool IWorkerThreadFree(IWorkerThread * itd);
void IWorkerThreadWaitForJobs(IWorkerThread * iwt, bool flag_wait_for_jobs);
int IWorkerThreadGetId(IWorkerThread * iwt);
bool IWorkerThreadIsValid(IWorkerThread * iwt);

#endif