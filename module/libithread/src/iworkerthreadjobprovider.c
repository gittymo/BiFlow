#include "global.h"
#include "iworkerthreadjob.h"

static size_t _iworker_thread_job_id = 0;

typedef struct _iworker_thread_job_provider {
    int struct_id;
    IWorkerThreadJob ** jobs;
    size_t jobs_count;
    size_t next_job_index;
    size_t jobs_array_size;
} IWorkThreadJobProvider;

IWorkThreadJobProvider * IWorkerThreadJobProviderCreate()
{
    IWorkThreadJobProvider * iwtjp = (IWorkThreadJobProvider *) malloc(sizeof(IWorkThreadJobProvider));
    if (iwtjp) {
        iwtjp->jobs = (IWorkerThreadJob **) malloc(sizeof(IWorkerThreadJob *) * 256);
        if (!iwtjp->jobs) {
            free(iwtjp);
            return NULL;
        } else {
            iwtjp->struct_id = ITHREAD_DATA_STRUCT_ID;
            iwtjp->jobs_array_size = 256;
            iwtjp->jobs_count = 0;
            iwtjp->next_job_index = 0;
        }
    }
    return iwtjp;
}

bool IWorkerThreadJobProviderIsValid(IWorkThreadJobProvider * iwtjp)
{
    return iwtjp && iwtjp->struct_id == ITHREAD_DATA_STRUCT_ID;
}

bool IWorkerThreadJobProviderAddJob(IWorkThreadJobProvider * iwtjp, void * job_data)
{
    if (!job_data || !IWorkerThreadJobProviderIsValid(iwtjp)) return false;
    if (iwtjp->jobs_count == iwtjp->jobs_array_size) {
        void ** new_job_array = (IWorkerThreadJob **) realloc(iwtjp->jobs, sizeof(IWorkerThreadJob *) * (iwtjp->jobs_count + 256));
        if (!new_job_array) return false;
        else {
            iwtjp->jobs = new_job_array;
            iwtjp->jobs_array_size += 256;
        }
    }
    IWorkerThreadJob * iwtj = IWorkerThreadJobCreate(job_data);
    if (IWorkerThreadJobIsValid(iwtj)) {
        iwtj->id = _iworker_thread_job_id++;
        iwtjp->jobs[iwtjp->jobs_count++] = iwtj;
        return true;
    }

    return false;
}
