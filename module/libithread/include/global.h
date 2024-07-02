#ifndef COM_PLUS_MEVANSPN_ITHREAD_GLOBAL
#define COM_PLUS_MEVANSPN_ITHREAD_GLOBAL

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "ithreadstate.h"
#include "ithreadjobstate.h"
#include "ithreadpriority.h"

static size_t _ithread_current_id;

#define ITHREAD_DATA_STRUCT_ID (('I' << 24) + ('T' << 16) + ('h' << 8) + ('d'))

typedef int IThreadTimeout;

typedef struct _iworker_thread IWorkerThread;
typedef struct _iworker_thread_controller IWorkerThreadController;
typedef struct _iworker_thread_job IWorkerThreadJob;

#define IThreadTimeoutNone 0
#define IThreadTimeoutSmart -1

void IThreadSleep(long milliseconds);

#endif