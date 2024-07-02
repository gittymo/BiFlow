#include "global.h"
#include "iworkerthreadjob.h"
#include "iworkerthreadjobprovider.h"

static size_t _iworker_thread_job_id = 0;

IWorkerThreadJobProvider * IWorkerThreadJobProviderCreate()
{
    IWorkerThreadJobProvider * iwtjp = (IWorkerThreadJobProvider *) malloc(sizeof(IWorkerThreadJobProvider));
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

bool IWorkerThreadJobProviderIsValid(IWorkerThreadJobProvider * iwtjp)
{
    return iwtjp && iwtjp->struct_id == ITHREAD_DATA_STRUCT_ID;
}

bool IWorkerThreadJobProviderAddJob(IWorkerThreadJobProvider * iwtjp, void * job_data)
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

bool IWorkerThreadJobProviderFree(IWorkerThreadJobProvider * iwtjp)
{
    if (!IWorkerThreadJobProviderIsValid(iwtjp)) return false;
    if (iwtjp->jobs_count > 0) {
        for (size_t i = 0; i < iwtjp->jobs_count; i++) {
            if (iwtjp->jobs[i]) {
                IWorkerThreadJobFree(iwtjp->jobs[i]);
                iwtjp->jobs[i] = NULL;
            }
        }
        free(iwtjp->jobs);
        iwtjp->jobs = NULL;
        iwtjp->jobs_count = 0;
    }
    iwtjp->jobs_array_size = 0;
    iwtjp->next_job_index = 0;
    iwtjp->struct_id = 0;
    free(iwtjp);
}

bool IWorkerThreadJobProviderHasJobs(IWorkerThreadJobProvider * iwtjp)
{
    return IWorkerThreadJobProviderIsValid(iwtjp) && iwtjp->next_job_index < iwtjp->jobs_count - 1;
}

IWorkerThreadJob * IWorkerThreadJobProviderNextJob(IWorkerThreadJobProvider * iwtjp)
{
    return IWorkerThreadJobProviderIsValid(iwtjp) && iwtjp->next_job_index < iwtjp->jobs_count ? iwtjp->jobs[iwtjp->next_job_index++] : NULL;
}
