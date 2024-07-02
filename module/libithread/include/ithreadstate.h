#ifndef COM_PLUS_MEVANSPN_ITHREAD_STATE
#define COM_PLUS_MEVANSPN_ITHREAD_STATE

typedef enum _ithread_state {
    IThreadStateUnusable, 
    IThreadStateInitialised, 
    IThreadStateRunning, 
    IThreadStateStopRequested, 
    IThreadStateStopped, 
    IThreadStateKillRequested, 
    IThreadStateKilled,
    IThreadStateKilledNoResponse,
    IThreadStateDone
} IThreadState;

#endif