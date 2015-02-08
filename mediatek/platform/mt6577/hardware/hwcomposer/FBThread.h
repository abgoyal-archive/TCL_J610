
#ifndef FBTHREAD_H
#define FBTHREAD_H


//--------------------------------------------------------------------------------------------------
#include <utils/threads.h>
#include <hardware/hwcomposer.h>

#include "MtkOverlay.h"

using namespace android;
using namespace MtkOverlay;


//--------------------------------------------------------------------------------------------------
//----------------------------------------------
// FBThread for HW invoked works
//----------------------------------------------
class FBThread : public Thread {
private:
    sem_t mEvent;
    Mutex mLock;

public:
    FBThread() {
        sem_init(&mEvent, 0, 0);
    }

    virtual void     onFirstRef();
    virtual status_t readyToRun();
    virtual bool     threadLoop();

    status_t trigger();
};


//--------------------------------------------------------------------------------------------------
#endif

