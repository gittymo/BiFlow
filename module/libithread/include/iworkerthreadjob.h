#ifndef COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD_JOB
#define COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD_JOB

#include "global.h"
#include "ithreadjobstate.h"

typedef struct _iworker_thread_job {
    int struct_id;
    size_t id;
    time_t start_time;
    time_t end_time;
    IThreadJobState state;
    void * data;
    struct _iworker_thread_job * next_job;
    char * failure_message;
    struct _iworker_thread * worker_thread;
} IWorkerThreadJob;

IWorkerThreadJob * IWorkerThreadJobCreate(void * data);
void IWorkerThreadJobFailed(IWorkerThreadJob * iwtj, char * message);
void IWorkerThreadJobFree(IWorkerThreadJob * itj);
void * IWorkerThreadJobGetData(IWorkerThreadJob * iwj);
size_t IWorkerThreadJobGetId(IWorkerThreadJob * iwtj);
struct _iworker_thread * IWorkerThreadJobGetParentThread(IWorkerThreadJob * iwtj);
bool IWorkerThreadJobIsValid(IWorkerThreadJob * iwtj);

#endif