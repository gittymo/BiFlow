#ifndef COM_PLUS_MEVANSPN_BIFLOW_THREADMAN
#define COM_PLUS_MEVANSPN_BIFLOW_THREADMAN

#define WORKER_FORCED_THREAD_KILL_TIMEOUT_SECONDS 30
#define WORKER_THREAD_TASK_TIME_KILL_MULTIPLIER 5

#include <stdbool.h>

typedef struct worker_thread WorkerThread;
typedef struct worker_thread_manager WorkerThreadManager;

WorkerThreadManager * WorkerThreadManagerCreate();
void WorkerThreadManagerStart(WorkerThreadManager * wtm);
bool WorkerThreadManagerIsRunning(WorkerThreadManager * wtm);
void WorkerThreadManagerFree(WorkerThreadManager * wtm);
bool WorkerThreadManagerAddWorkerThread(WorkerThreadManager * wtm, void * (* worker_thread_function)(void *), void * task_data);
bool WorkerThreadManagerTasksComplete(WorkerThreadManager * wtm);

void WorkerThreadStartTask(WorkerThread * w);
void WorkerThreadEndTask(WorkerThread * w);
void * WorkerThreadExit(WorkerThread * w);
bool WorkerThreadIsValid(WorkerThread * w);
int WorkerThreadGetId(WorkerThread * w);
bool WorkerThreadInterrupted(WorkerThread * w);
bool WorkerThreadWasKilled(WorkerThread * w);
bool WorkerThreadWasStopped(WorkerThread * w);
bool WorkerThreadIsRunning(WorkerThread * w);
bool WorkerThreadIsDone(WorkerThread * w);
void * WorkerThreadGetTaskData(WorkerThread * w);

#endif