#include <time.h>

#include "global.h"

static size_t _ithread_current_id = 0;

/// @brief Makes the current thread of execution sleep for the given number of milliseconds.
/// @param milliseconds Milliseconds to sleep.  If value is negative, the absolute value will be used (i.e. -100 = 100).
void IThreadSleep(long milliseconds)
{
    // If the sleep period is zero milliseconds, we can return immediately.
    if (milliseconds == 0) return;

    // Take the absolute value of milliseconds if the original value was negative.
    if (milliseconds < 0) milliseconds = -milliseconds;
    
    // Create a timespec with the appropriate values for the sleep operation.
    struct timespec ts;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    ts.tv_sec = milliseconds / 1000;

    // This second timespec is never used, but it required be the nanosleep function.
    struct timespec rem;

    // Cause the current exection thread to sleep for the required number of milliseconds.
    nanosleep(&ts, &rem);
}