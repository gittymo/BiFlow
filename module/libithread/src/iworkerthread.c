#include "global.h"
#include "iworkerthread.h"
#include "iworkerthreadjob.h"
#include "iworkerthreadcontroller.h"

/// @brief This function defines how a worker thread is controlled.  If is passed to pthread_create then the 
///         worker thread is started (triggered by calling WorkerThreadControllerStart() ).
/// @param data Pointer to a valid worker thread data structure.
/// @return NULL.
void * IWorkerThreadRun(void * data)
{
    // Make sure the worker thread data structure is valid and in its initialised state.
    IWorkerThread * itd = (IWorkerThread *) data;
    if (!IWorkerThreadIsValid(itd) || itd->state != IThreadStateInitialised) return NULL; // It's not so exit immediately.
    else itd->state = IThreadStateRunning; // It is, so set the worker thread state to running.

    IWorkerThreadJobProvider * iwtjp = itd->controller->job_provider;

    // Perform processing on any jobs that have been allocated to the thread until it should exit.
    while (((itd->flag_exit_on_no_jobs && IWorkerThreadJobProviderHasJobs(iwtjp)) || !itd->flag_exit_on_no_jobs) && itd->state == IThreadStateRunning)
    {
        // Depending on the priority of the worker thread it can process one or more jobs at a time.
        for (int j = 0; j < itd->priority && IWorkerThreadJobProviderHasJobs(iwtjp) && itd->state == IThreadStateRunning; j++) {
            itd->current_job = IWorkerThreadJobProviderNextJob(iwtjp);
            if (!itd->current_job) continue;
            // Record the job processing start time.
            itd->current_job->start_time = time(NULL);
            // Process the job.
            itd->threadMainFunction(itd->current_job);
            // Record the job processing end time.
            itd->current_job->end_time = time(NULL);
            // Make the job as done.
            itd->current_job->state = IThreadJobStateDone;
            // Record the total time it took to process the job in the thread's job run time history (this is used for smart thread killing).
            itd->job_run_time_history[itd->jobs_run % 10] = itd->current_job->end_time - itd->current_job->start_time;
            if (itd->jobSuccessCallbackFunction) {
                itd->jobSuccessCallbackFunction(itd->current_job);
            }
            // Increment the number of jobs processed.
            itd->jobs_run++;
        }
        // have a little bit of a sleep to stop the thread hogging all the CPU core's processing time.
        IThreadSleep(20);
    }

    // Now the thread has done processing work (or a stop/kill request has been received), we can set it's state appropriately.
    switch (itd->state) {
        case IThreadStateStopRequested : itd->state = IThreadStateStopped; break;
        case IThreadStateKillRequested : itd->state = IThreadStateKilled; break;
        case IThreadStateRunning : itd->state = IThreadStateDone; break;
        default: {}
    }

    // Record the time the thread exited.
    itd->end_time = time(NULL);

    // Exit the thread and return NULL.
    pthread_exit(NULL);
    return NULL;
}

/// @brief Will try to create a worker thread data structure with pointers to functions used for processing jobs.
/// @param workFunction Pointer to a function that will do the main processing work for a job.
/// @param successFunction Pointer to a function that will handle a job after it has been processed (by workFunction).
/// @param failureFunction Pointer to a function that will handle a job then it has failed to process (by workFunction).
/// @param itc Pointer to the parent worker thread controller data structure.
/// @return Pointerto a worker thread data structure (IWorkerThread) or NULL if the data structure can't be created.
IWorkerThread * IWorkerThreadCreate(  void (* workFunction)(IWorkerThreadJob *),
                                void (*successFunction)(IWorkerThreadJob *),
                                void (*failureFunction)(IWorkerThreadJob *),
                                IWorkerThreadController * itc)
{
    // Make sure the calling function has passed in a valid worker thread controller pointer.  If not, exit immediately.
    if (!IWorkerThreadControllerIsValid(itc)) return NULL;

    // Try to reserve memory for a worker thread data structure.
    IWorkerThread * itd = (IWorkerThread *) malloc(sizeof(IWorkerThread));
    if (itd) {
        // We have managed to reserve memory, so initialise the data structure, recording the pointers to the
        // processing functions that have been passed in. 
        itd->struct_id = ITHREAD_DATA_STRUCT_ID;
        itd->start_time = itd->end_time = 0;
        itd->state = IThreadStateInitialised;
        itd->priority = IThreadPriorityNormal;
        itd->handle = 0;
        itd->threadMainFunction = workFunction;
        itd->jobFailureCallbackFunction = failureFunction;
        itd->jobSuccessCallbackFunction = successFunction;
        itd->id = _ithread_current_id++;
        itd->jobs_run = 0;
        itd->current_job = NULL;
        itd->controller = itc;
        itd->flag_exit_on_no_jobs = false;
    }

    // Return pointer to the worker thread data structure or NULL if we couldn't allocate memory for it.
    return itd;
}

/// @brief Gets the average time a job has taken to process.  The average is taken from the times accumulated over the past 'n'
///         jobs where 'n' is in the range 1..10.
/// @param itd Pointer to worker thread data structure.
/// @return time_t data structure holding the average processing time (or 0 if no jobs have been processed).
time_t IWorkerThreadGetAverageJobTime(IWorkerThread * itd)
{
    if (!IWorkerThreadIsValid(itd) || itd->state != IThreadStateRunning || itd->jobs_run == 0) return 0;
    time_t total_jobs_time = 0;
    const size_t MAX_JOB_INDEX = itd->jobs_run <= 10 ? itd->jobs_run : 10;
    for (size_t j = 0; j < MAX_JOB_INDEX; j++) total_jobs_time += itd->job_run_time_history[j];
    return total_jobs_time / MAX_JOB_INDEX;
}

/// @brief Indicates if a worker thread has finished.  At this stage, the thread can do no more work, either exiting naturally, or
///         having been stopped/killed by an external request.
/// @param iwt Pointer to worker thread data structure.
/// @return True if the thread has finished processing and have exited, or has been stopped or killed.
bool IWorkerThreadDone(IWorkerThread * iwt)
{
    // Make sure we've got a valid pointer to a worker thread data structure.
    if (!IWorkerThreadIsValid(iwt)) return true;

    // CHeck the state of the thread and return true if the thread can no longer process any more work.
    if (iwt->state == IThreadStateUnusable || iwt->state == IThreadStateDone ||
        iwt->state == IThreadStateKilled || iwt->state == IThreadStateStopped) return true;

    // Otherwise, return false, indicating the thread is still running.
    return false;
}

bool IWorkerThreadFree(IWorkerThread * itd)
{
    if (!IWorkerThreadIsValid(itd)) return false;
    itd->start_time = itd->end_time = 0;
    itd->handle = 0;
    itd->threadMainFunction = NULL;
    itd->jobFailureCallbackFunction = NULL;
    itd->jobSuccessCallbackFunction = NULL;
    itd->state = IThreadStateUnusable;
    itd->id = 0;
    itd->flag_exit_on_no_jobs = false;
    itd->jobs_run = 0;
    itd->current_job = NULL;
    itd->priority = IThreadPriorityNone;
    itd->controller = NULL;
    free(itd);
    return true;
}

void IWorkerThreadWaitForJobs(IWorkerThread * iwt, bool flag_wait_for_jobs)
{
    if (!IWorkerThreadIsValid(iwt)) return;
    iwt->flag_exit_on_no_jobs = !flag_wait_for_jobs;
}

int IWorkerThreadGetId(IWorkerThread * iwt)
{
    return (!IWorkerThreadIsValid(iwt)) ? -1 : iwt->id;
}

bool IWorkerThreadIsValid(IWorkerThread * iwt)
{
    return iwt && iwt->struct_id == ITHREAD_DATA_STRUCT_ID && iwt->state != IThreadStateUnusable;
}