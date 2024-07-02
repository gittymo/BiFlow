#ifndef COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD_CONTROLLER
#define COM_PLUS_MEVANSPN_ITHREAD_WORKER_THREAD_CONTROLLER

#include <pthread.h>

#include "global.h"

typedef struct _iworker_thread_controller {
    int struct_id;
    struct _iworker_thread ** threads;
    int threads_count;
    int threads_buffer_size;
    bool stop, running;
    pthread_t handle;
} IWorkerThreadController;

IWorkerThreadController * IWorkerThreadControllerCreate();
bool IWorkerThreadControllerFree(IWorkerThreadController * itc);
void * IWorkerThreadControllerRun(void * data);
IWorkerThread * IWorkerThreadControllerAddWorkerThread(IWorkerThreadController * itc, 
                                                    void (*mainThreadFunction)(IWorkerThreadJob *), 
                                                    void (*successCallbackFunction)(IWorkerThreadJob *),
                                                    void (*failureCallbackFunction)(IWorkerThreadJob *),
                                                    IThreadTimeout timeout);
bool IWorkerThreadControllerChildThreadsDone(IWorkerThreadController * itc);
bool IWorkerThreadControllerStart(IWorkerThreadController * itc);
void IWorkerThreadControllerStop(IWorkerThreadController * itc);
bool IWorkerThreadControllerIsRunning(IWorkerThreadController * itc);
bool IWorkerThreadControllerIsValid(IWorkerThreadController * iwtc);

#endif