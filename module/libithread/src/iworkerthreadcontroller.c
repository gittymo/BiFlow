#include "ithread.h"
#include "iworkerthread.h"
#include "iworkerthreadjob.h"
#include "iworkerthreadcontroller.h"

/// @brief Creates and initialises a worker thread controller data structure, then passes back a pointer to it's data.
/// @return Pointer to the worker thread controller (IWorkerThreadController) data structure.
IWorkerThreadController * IWorkerThreadControllerCreate()
{
    // Try to reserve enough memory to hold the work thread controller data structure.
    IWorkerThreadController * itc = (IWorkerThreadController *) malloc(sizeof(IWorkerThreadController));
    if (itc) {
        // We were able to reserve enough memory for the data structure, so initialise it.
        itc->struct_id = ITHREAD_DATA_STRUCT_ID;    // Standard IThread identifier used to distinguish data structures created by this library.
        itc->threads_count = 0;                     // All controllers have no threads assigned to them at initialisation.
        itc->threads_buffer_size = 4;               // threads_buffer_size is used to dynamically allocate memory that holds the list of
                                                    // threads assigned to this data structure.  If threads_count == threads_buffer_size, the
                                                    // list will be resized to allow more additions.
        itc->threads = (IWorkerThread **) malloc(sizeof(IWorkerThread *) * itc->threads_buffer_size); // List of threads allocated to data structure.
        itc->stop = itc->running = false;           // Initially the controller should do nothing until it is asked to start.
    }
    return itc; // Return pointer to newly created IWorkerThreadController data structure or NULL if there was not enough memory.
}

/// @brief Frees any memory reserved for a worker thread controller data structure, if one can be found at the given pointer.
/// @param itc Pointer to the worker thread controller (IWorkerThreadController) data structure.
/// @return True if the pointer referenced a valid worker thread controller data structure or false otherwise.
bool IWorkerThreadControllerFree(IWorkerThreadController * itc)
{
    // Check to see if the given pointer references a valid worker thread controller data structure.  If not, return false.
    if (!IWorkerThreadControllerIsValid(itc)) return false;

    // Check to se eif the worker thread controller is running.  if so, stop it.
    if (itc->running) IWorkerThreadControllerStop(itc);
    
    // Reset any parameter values associated with the data structure to their defaults.
    itc->running = false;
    itc->stop = false;
    itc->struct_id = 0;
    if (itc->threads_count > 0) {
        // The data structure also references one or more worker thread data data structures, whose memory also needs to be freed.
        for (int t = 0; t < itc->threads_count; t++) {
            IWorkerThreadFree(itc->threads[t]);
            itc->threads[t] = NULL;
        }
        // Reset the controller thread counts back to zero.
        itc->threads_count = itc->threads_buffer_size = 0;
        // Free meomory related to the worker thread controller.
        free(itc->threads);
        // remove pointer to the list of threads.
        itc->threads = NULL;
    }
    // Free any remaining memory allocated to the IWorkerThreadController data structure.
    free(itc);
    // Return true to indicate the memory allocated to the worker thread controller data structure.
    return true;
}

/// @brief This function defines how a worker thread controller works.  Essentially, when a worker thread controller is started,
///         this function is passed to pthread_create, along with a pointer to the worker thread controller data structure.
/// @param data Pointer to a valid worker thread controller (IWorkerThreadController) data structure.
/// @return NULL.  
void * IWorkerThreadControllerRun(void * data)
{
    // Make sure the calling method provided a valid pointer value.  if not, exit the thread immediately.
    if (!data) return NULL;
    IWorkerThreadController * itc = (IWorkerThreadController *) data;

    // Check to make sure the 'data' pointer points to a worker thread controller data structure.  Exit the thread immediately if not.
    if (!IWorkerThreadControllerIsValid(itc)) return NULL;
    else itc->running = true;  // We have a got a valid worker thread controller data structure pointer, so set the controller state to 'running'.

    // Check to see if the controlling program has asked the worker thread controller to stop.
    if (!itc->stop) {
        // If not, we need to iterate through the list of worker thread data structures and make sure they're valid.
        for (int t = 0; t < itc->threads_count; t++) {
            IWorkerThread * itd = itc->threads[t];
            if (!IWorkerThreadIsValid(itd)) continue;
            // Also, at this current point in time, the worker thread data structure should be in its initialised state.  If not, kill it.
            if (itd->state != IThreadStateInitialised) itd->state = IThreadStateKilled;
            else pthread_create(&itd->handle, NULL, IWorkerThreadRun, itd); // The worker thread state is good, so we can create a pthread that
                                                                            // uses the worker thread data structure.
        }
    } else itc->running = false; // The controlling program has asked the worker thread controller to stop, so set a flag to indicate as such.
   
    // Whilst the worker thread controller isn't stopped and there's work to be done, we need to keep an eye on it's worker threads to make
    // sure they're behaving and not taking too long to do a job.
    while (!IWorkerThreadControllerChildThreadsDone(itc) && !itc->stop) {
        for (int t = 0; t < itc->threads_count; t++) {
            IWorkerThread * itd = itc->threads[t];
            if (!itd) continue;
            if (itd->current_job && itd->current_job->state == IThreadJobStateRunning) {
                // The controller's current worker thread is running and has an active job.  Find out how long the job has taken so far (in secnods).
                const time_t CURRENT_JOB_TIME = time(NULL) - itd->current_job->start_time;
                // Depending on the configuration of the worker thread, it can either wait for a job to finish indefinitely - or wait for one of
                // two time periods before the thread is killed (cancelled).  NOTE:  This is time allocated PER JOB, not for all jobs.
                // 1) If a timeout value is given that is greater than 0 seconds, the thread will be allowed to work for that amount of time
                //      on a job, before it is killed.
                // 2) Depending on how many jobs have been completed the thread will timeout after ITHREAD_DEFAULT_TIMEOUT_SEC (if only one job has run)
                //      or after double the average amount of time it has taken to run the last 'n' jobs (where n can be up to 10).
                bool kill_thread = false;
                switch (itd->timeout) {
                    case IThreadTimeoutSmart : {
                        if (itd->jobs_count < 3 && CURRENT_JOB_TIME > ITHREAD_DEFAULT_TIMEOUT_SEC) kill_thread = true;
                        else if (CURRENT_JOB_TIME > IWorkerThreadGetAverageJobTime(itd) * 2) kill_thread = true;
                    } break;
                    case IThreadTimeoutNone : break;
                    default : {
                        if (CURRENT_JOB_TIME > itd->timeout) kill_thread = true;
                    }
                }
                if (kill_thread) {
                    pthread_cancel(itd->handle);
                    IWorkerThreadJobFailed(itd->current_job, "Processing thread killed due to timeout.");
                    itd->state = IThreadStateKilledNoResponse;
                }
            }
        }
        // Sleep for a short period of time to stop the worker thread controller thread hogging a CPU core's processing time.
        IThreadSleep(20);
    }
    // If the controller has been stopped or there's no more work to do, flag the controller as no longer running.
    itc->running = false;

    // Return NULL
    pthread_exit(NULL);
}

/// @brief This function will create a new worker thread data structure and add it to the list of worker thread data structures
///         associated with the given worker thread controller.
/// @param itc Pointer to a valid worker thread controller data structure.
/// @param mainThreadFunction Reference to the main executable function used to process a worker thread's job data.
/// @param successCallbackFunction Reference to a function that will handle a worker thread's job data after it has been 
///                                 processed successfully by mainThreadFunction.
/// @param failureCallbackFunction Reference to a function that will handle a worker thread's job data after it has been
///                                 unsuccessfully processed by mainThreadFunction.
/// @param timeout Either a positive integer value greater than zero or one of two predefined values: IThreadTimeoutNone and
///                 IThreadTimeoutSmart.  These control how a thread is killed if a job is taking too long.
/// @return Pointer to a worker thread data structure or NULL if one could not be created (usually if worker thread controller pointer is invalid).
IWorkerThread * IWorkerThreadControllerAddWorkerThread(IWorkerThreadController * itc, 
                                                    void (*mainThreadFunction)(IWorkerThreadJob *), 
                                                    void (*successCallbackFunction)(IWorkerThreadJob *),
                                                    void (*failureCallbackFunction)(IWorkerThreadJob *),
                                                    IThreadTimeout timeout)
{
    // Make sure the calling method has passed valid worker thread controller data structure and mainThreadFunction pointers.
    if (!IWorkerThreadControllerIsValid(itc) || !mainThreadFunction) return NULL;

    // Attempt to create a worker thread data structure.
    IWorkerThread * itd = IWorkerThreadCreate(mainThreadFunction, successCallbackFunction, failureCallbackFunction, itc);
    if (itd) {
        // We have created a worker thread data structure (and it has been initialised), so set it's timeout method and add it to the
        // worker thread controller list of worker threads.
        itd->timeout = timeout < 0 ? IThreadTimeoutSmart : timeout;
        itc->threads[itc->threads_count++] = itd;
        // Make sure the worker thread controller worker thrad list isn't full.  If it is resize it.
        if (itc->threads_count == itc->threads_buffer_size) {
            itc->threads_buffer_size += 4;
            itc->threads = (IWorkerThread **) realloc(itc->threads, sizeof(IWorkerThread *) * itc->threads_buffer_size);
        }
    }
    // Return the pointer to the newly created worker thread data structure (or NULL if it couldn't be created.)
    return itd;
}

/// @brief Indicates if all child worker threads have finished (or have been stopped/killed) and are no longer able to process any more jobs.
/// @param itc Pointer to worker thread controller data structure.
/// @return True if the data structure is valid and the child threads under it's control have finished.  False otherwise.
bool IWorkerThreadControllerChildThreadsDone(IWorkerThreadController * itc)
{
    bool controller_invalid = !IWorkerThreadControllerIsValid(itc);
    if (controller_invalid) return true;
    int threads_done = 0;
    for (int t = 0; t < itc->threads_count; t++) {
        IWorkerThread * itd = itc->threads[t];
        if (IWorkerThreadDone(itd)) threads_done++;
    }
    return threads_done == itc->threads_count;
}

/// @brief Given a valid worker thread controller data structure, this function will start the worker thread controller on its own thread.
/// @param itc Pointer to a worker thread controller data structure.
/// @return True if the data structure was valid, false otherwise.
bool IWorkerThreadControllerStart(IWorkerThreadController * itc)
{
    if (!IWorkerThreadControllerIsValid(itc) || itc->threads_count == 0) return false;
    pthread_create(&itc->handle, NULL, IWorkerThreadControllerRun, itc);
    return true;
}

/// @brief Requests that the worker thread controller stop.  If the controller thread does not stop in a sensible fashion, it will be killed.
/// @param itc Pointer to worker thread controller data structure.
void IWorkerThreadControllerStop(IWorkerThreadController * itc)
{
    // Make sure the calling function has provided a sensible worker thread controller data structure.
    if (!IWorkerThreadControllerIsValid(itc) || itc->threads_count == 0) return;

    // Set the stop flag for the controller thread.
    itc->stop = true;

    // Record when the stop request was made.
    const time_t REQUEST_CONTROLLER_STOP_TIME = time(NULL);

    // Wait for five seconds or until the controller has stopped running.
    while (itc->running && time(NULL) < REQUEST_CONTROLLER_STOP_TIME + 5) {
        IThreadSleep(20);
    }

    // Check to see if the controller is still running.
    if (itc->running) {
        // It is, so kill the controller thread.
        itc->running = false;
        pthread_cancel(itc->handle);
    }
}

/// @brief Indicates if the worker thread controller is running.
/// @param itc Pointer to worker thread controller data structure.
/// @return True if the pointer is valid and the thread is running, false otherwise.
bool IWorkerThreadControllerIsRunning(IWorkerThreadController * itc)
{
    bool is_valid = IWorkerThreadControllerIsValid(itc);
    bool is_running = is_valid ? !IWorkerThreadControllerChildThreadsDone(itc) : false;
    return is_running;
}

/// @brief Indicates if the given pointer points to a valid worker thread controller data structure (IWorkerThreadController).
/// @param iwtc Pointer to worker thread controller data structure.
/// @return True if pointer points to valid worker thread controller data structure, false otherwise.
bool IWorkerThreadControllerIsValid(IWorkerThreadController * iwtc)
{
    return iwtc && iwtc->struct_id == ITHREAD_DATA_STRUCT_ID;
}