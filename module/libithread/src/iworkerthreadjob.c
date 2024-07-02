#include <string.h>

#include "global.h"
#include "iworkerthread.h"
#include "iworkerthreadjob.h"

void IWorkerThreadJobFailed(IWorkerThreadJob * iwtj, char * message)
{
    if (!IWorkerThreadJobIsValid(iwtj) || iwtj->state != IThreadJobStateRunning || iwtj->state != IThreadJobStatePaused) return;
    iwtj->state = IThreadJobStateFailed;
    iwtj->failure_message = (char *) malloc(512);
    if (iwtj->failure_message) strncpy(iwtj->failure_message, message, 512);
    IWorkerThread * iwt = IWorkerThreadJobGetParentThread(iwtj);
    iwt->jobFailureCallbackFunction(iwtj);
}

void IWorkerThreadJobFree(IWorkerThreadJob * itj)
{
    if (!IWorkerThreadJobIsValid(itj)) return;
    itj->data = NULL;
    itj->start_time = itj->end_time = 0;
    if (itj->failure_message) {
        free(itj->failure_message);
        itj->failure_message = NULL;
    }
    itj->next_job = NULL;
    itj->state = IThreadJobStateUnusuable;
    itj->struct_id = 0;
    itj->id = 0;
    itj->worker_thread = NULL;
    free(itj);
}

void * IWorkerThreadJobGetData(IWorkerThreadJob * iwj)
{
    return IWorkerThreadJobIsValid(iwj) ? iwj->data : NULL;
}

size_t IWorkerThreadJobGetId(IWorkerThreadJob * iwtj)
{
    return !IWorkerThreadJobIsValid(iwtj) ? 0 : iwtj->id;
}

IWorkerThread * IWorkerThreadJobGetParentThread(IWorkerThreadJob * iwtj)
{
    return !IWorkerThreadJobIsValid(iwtj) ? NULL : iwtj->worker_thread;
}

bool IWorkerThreadJobIsValid(IWorkerThreadJob * iwtj)
{
    return iwtj && iwtj->struct_id == ITHREAD_DATA_STRUCT_ID && iwtj->state != IThreadJobStateUnusuable;
}