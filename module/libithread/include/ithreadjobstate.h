#ifndef COM_PLUS_MEVANSPN_ITHREAD_JOB_STATE
#define COM_PLUS_MEVANSPN_ITHREAD_JOB_STATE

typedef enum _ithread_job_state {
    IThreadJobStateUnusuable,
    IThreadJobStateInitialised,
    IThreadJobStateRunning,
    IThreadJobStatePaused,
    IThreadJobStateStopped,
    IThreadJobStateFailed,
    IThreadJobStateDone
} IThreadJobState;

#endif