#define WORKER_THREAD_DATA_STRUCTURE_ID (('W' << 24) + ('T' << 16) + ('h' << 8) + 'r')

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "libthreadman.h"

int worker_thread_id = 0;

typedef struct worker_thread {
    int id;
    time_t start_time, end_time;
    bool killed, running, done, stop;
    time_t last_task_start_time;
    time_t average_task_time;
    int tasks_run;
    void * (* function_ptr)(void *);
    void * task_data_ptr;
    int ptid;
    pthread_t pt;
    time_t kill_request_time;
    struct worker_thread * next;
    bool soft_kill_recover;
} WorkerThread;

typedef struct worker_thread_manager {
    int id;
    WorkerThread * first_thread;
    bool running, stop, done;
    bool work_complete;
} WorkerThreadManager;

WorkerThread * WorkerThreadCreate(void *(* worker_thread_function)(void *), void * task_data_ptr)
{
    if (!worker_thread_function) return NULL;
    WorkerThread * w = (WorkerThread *) malloc(sizeof(WorkerThread));
    if (w) {
        w->id = WORKER_THREAD_DATA_STRUCTURE_ID;
        w->ptid = worker_thread_id++;
        w->start_time = w->end_time = w->last_task_start_time = w->average_task_time = 0;
        w->killed = w->running = w->done = w->stop = false;
        w->tasks_run = 0;
        w->function_ptr = worker_thread_function;
        w->task_data_ptr = task_data_ptr;
        w->kill_request_time = 0;
        w->next = NULL;
        w->soft_kill_recover = false;
    }
    return w;
}

bool WorkerThreadIsValid(WorkerThread * w)
{
    return w && w->id == WORKER_THREAD_DATA_STRUCTURE_ID && w->function_ptr;
}

int WorkerThreadGetId(WorkerThread * w)
{
    return WorkerThreadIsValid(w) ? w->ptid : -1;
}

bool WorkerThreadShouldKill(WorkerThread * w)
{
    if (!WorkerThreadIsValid(w)) return false;
    if (!w->running || w->done) return false;
    if (w->start_time > 0) {
        if (w->average_task_time > 0 && w->tasks_run > 0 &&
            time(NULL) - w->start_time >= WORKER_THREAD_TASK_TIME_KILL_MULTIPLIER * w->average_task_time)
        {
            return true;
        } else if (time(NULL) - w->start_time >= WORKER_FORCED_THREAD_KILL_TIMEOUT_SECONDS)
        {
            return true;
        }
    }
    return false;
}

bool WorkerThreadFree(WorkerThread * w)
{
    if (!WorkerThreadIsValid(w)) return false;
    if (w->running) pthread_cancel(w->pt);
    w->start_time = w->end_time = w->average_task_time = w->last_task_start_time = 0;
    w->done = w->killed = w->running = w->stop = false;
    w->tasks_run = 0;
    w->id = 0;
    w->function_ptr = NULL;
    w->next = NULL;
    w->task_data_ptr = NULL;
    w->kill_request_time = 0;
    free(w);
    return true;
}

bool WorkerThreadInterrupted(WorkerThread * w)
{
    return WorkerThreadIsValid(w) & (w->stop || w->killed);
}

bool WorkerThreadWasKilled(WorkerThread * w)
{
    return WorkerThreadIsValid(w) & w->killed;
}

bool WorkerThreadWasStopped(WorkerThread * w)
{
    return WorkerThreadIsValid(w) & w->stop;
}

bool WorkerThreadIsRunning(WorkerThread * w)
{
    return WorkerThreadIsValid(w) & w->running;
}

bool WorkerThreadIsDone(WorkerThread * w)
{
    return WorkerThreadIsValid(w) & w->done;
}

void * WorkerThreadGetTaskData(WorkerThread * w) 
{
    return WorkerThreadIsValid(w) ? w->task_data_ptr : NULL;
}

void * WorkerThreadExit(WorkerThread * w)
{
    if (WorkerThreadIsValid(w)) {
        w->done = true;
        w->running = false;
        if (w->soft_kill_recover) w->killed = false;
        pthread_exit(NULL);
    }
    return NULL;
}

WorkerThreadManager * WorkerThreadManagerCreate()
{
    WorkerThreadManager * wtm = (WorkerThreadManager *) malloc(sizeof(WorkerThreadManager));
    if (wtm) {
        wtm->id = WORKER_THREAD_DATA_STRUCTURE_ID;
        wtm->first_thread = NULL;
        wtm->done = wtm->running = wtm->stop = false;
        wtm->work_complete = false;
    }

    return wtm;
}

void WorkerThreadManagerFree(WorkerThreadManager * wtm)
{
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID) return;
    WorkerThread * w = wtm->first_thread;
    while (w) {
        WorkerThread * nw = w->next;
        if (w->running) WorkerThreadFree(w);
        w = nw;
    }
    wtm->id = 0;
    wtm->first_thread = NULL;
    free(wtm);
}

bool WorkerThreadManagerAddWorkerThread(WorkerThreadManager * wtm, void * (* worker_thread_function)(void *), void * task_data)
{
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID || !worker_thread_function) return false;
    WorkerThread * w = WorkerThreadCreate(worker_thread_function, task_data);
    if (wtm->first_thread == NULL) wtm->first_thread = w;
    else {
        WorkerThread * ew = wtm->first_thread;
        while (ew->next) ew = ew->next;
        ew->next = w;
    }
    return true;
}

bool WorkerThreadManagerWorkDone(WorkerThreadManager * wtm)
{
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID || !wtm->first_thread) return false;
    bool jobs_done = true;
    WorkerThread * w = wtm->first_thread;
    while (w && jobs_done) {
        WorkerThread * nw = w->next;
        jobs_done = w->done;
        w = nw;
    }
    return jobs_done;
}

void * WorkerThreadManagerControlFunction(void * wtm_ptr)
{
    WorkerThreadManager * wtm = (WorkerThreadManager *) wtm_ptr;
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID || !wtm->first_thread || wtm->done) pthread_exit(NULL);
    WorkerThread * w = wtm->first_thread;
    while (w) {
        WorkerThread * nw = w->next;
        if (!WorkerThreadIsValid(w)) {
            pthread_exit(NULL);
            return NULL;
        }
        if (w->running) {
            pthread_exit(NULL);
            return NULL;
        }
        if (w->done) {
            pthread_exit(NULL);
            return NULL;
        }
        if (w->stop) {
            pthread_exit(NULL);
            return NULL;
        }
        if (w->killed) {
            pthread_exit(NULL);
            return NULL;
        }
        w->running = true;
        pthread_create(&w->pt, NULL, w->function_ptr, w);
        w = nw;
    }
    while (!wtm->stop && !WorkerThreadManagerWorkDone(wtm)) {
        w = wtm->first_thread;
        while (w) {
            WorkerThread * nw = w->next;
            if (WorkerThreadShouldKill(w)) {
                if (!w->killed) {
                    w->killed = true;
                    w->kill_request_time = time(NULL);
                    w->soft_kill_recover = true;
                } else if (time(NULL) - w->kill_request_time >= WORKER_FORCED_THREAD_KILL_TIMEOUT_SECONDS) {
                    w->done = true;
                    w->running = false;
                    w->soft_kill_recover = false;
                    pthread_cancel(w->pt);
                }
            }
            w = nw;
        }
        sleep(1);
    }
    if (wtm->stop) {
        w = wtm->first_thread;
        while (w) {
            WorkerThread * nw = w->next;
            w->stop = true;
            w = nw;
        }
    } else {
        wtm->work_complete = true;
        w = wtm->first_thread;
        while (wtm->work_complete && w) {
            WorkerThread * nw = w->next;
            wtm->work_complete = w->done & !w->killed & !w->stop;
            w = nw;
        }
    }

    wtm->running = false;
    wtm->done = true;
    pthread_exit(NULL);
    return NULL;
}

bool WorkerThreadManagerTasksComplete(WorkerThreadManager * wtm)
{
    return wtm && wtm->id == WORKER_THREAD_DATA_STRUCTURE_ID && !wtm->running && wtm->done && wtm->work_complete;
}

void WorkerThreadStartTask(WorkerThread * w)
{
    w->last_task_start_time = time(NULL);
    w->start_time = w->last_task_start_time;
}

void WorkerThreadEndTask(WorkerThread * w)
{
    time_t task_time = time(NULL) - w->last_task_start_time;
    w->average_task_time = w->tasks_run > 0 ? (w->average_task_time + task_time) / 2 : task_time;
    w->tasks_run++;
}

void WorkerThreadManagerStart(WorkerThreadManager * wtm)
{
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID || !wtm->first_thread) return;
    wtm->running = true;
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, WorkerThreadManagerControlFunction, wtm);
}

bool WorkerThreadManagerIsRunning(WorkerThreadManager * wtm)
{
    if (!wtm || wtm->id != WORKER_THREAD_DATA_STRUCTURE_ID || !wtm->first_thread) return false;
    return wtm->running;
}