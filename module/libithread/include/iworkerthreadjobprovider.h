#ifndef COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD_JOB_PROVIDER
#define COM_PLUS_MEVANSPN_ITHREAD_WORDER_THREAD_JOB_PROVIDER

#include "global.h"

typedef struct _iworker_thread_job_provider {
    int struct_id;
    IWorkerThreadJob ** jobs;
    size_t jobs_count;
    size_t next_job_index;
    size_t jobs_array_size;
} IWorkerThreadJobProvider;

IWorkerThreadJobProvider * IWorkerThreadJobProviderCreate();
bool IWorkerThreadJobProviderIsValid(IWorkerThreadJobProvider * iwtjp);
bool IWorkerThreadJobProviderAddJob(IWorkerThreadJobProvider * iwtjp, void * job_data);
bool IWorkerThreadJobProviderHasJobs(IWorkerThreadJobProvider * iwtjp);
bool IWorkerThreadJobProviderFree(IWorkerThreadJobProvider * iwtjp);
IWorkerThreadJob * IWorkerThreadJobProviderNextJob(IWorkerThreadJobProvider * iwtjp);

#endif