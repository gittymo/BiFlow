#ifndef COM_PLUS_MEVANSPN_ITHREAD_PRIORITY
#define COM_PLUS_MEVANSPN_ITHREAD_PRIORITY

typedef enum _ithread_priority {
    IThreadPriorityNormal = 1,
    IThreadPriorityHigh = 2,
    IThreadPriorityHighest = 3,
    IThreadPriorityNone = 0
} IThreadPriority;

#endif