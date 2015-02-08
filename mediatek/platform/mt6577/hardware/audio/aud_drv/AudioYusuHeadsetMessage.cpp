

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include "AudioYusuLad.h"
#include <utils/Log.h>
#include "AudioYusuLad.h"
#include "AudioYusuHeadsetMessage.h"
#include "AudioYusuHardware.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioHeadSetMessager"

#ifdef ENABLE_LOG_AUDIOHEADSETMESSAGER
    #define LOG_AudioHeadSetMessager LOGD
#else
    #define LOG_AudioHeadSetMessager LOGV
#endif

namespace android {

static int HeadsetFd =0;
static int headstatusFd =0;
#define BUF_LEN 1
static char rbuf[BUF_LEN] = {'\0'};
static char wbuf[BUF_LEN] = {'1'};
static char wbuf1[BUF_LEN] = {'2'};

bool AudioYusuHeadSetMessager::SetHeadInit()
{
    LOG_AudioHeadSetMessager("SetHeadInit");
    int ret =0;
    if(HeadsetFd <=0){
        // open headset device
        HeadsetFd = open(HEADSET_PATH, O_RDONLY);
        if(HeadsetFd <0){
            LOGE("open %s error fd = %d",HEADSET_PATH,HeadsetFd);
            return false;
        }
    }
    ret = ::ioctl(HeadsetFd,ACCDET_INIT,0);
    return true;
}

AudioYusuHeadSetMessager::AudioYusuHeadSetMessager(LAD *lad)
{
    LOG_AudioHeadSetMessager("AudioHeadSetMessager Contructor");
    int ret =0;
    if(lad != NULL){
        mLad = lad;
    }
    else{
        LOGD("AudioHeadSetMessager Contructor with no Lad");
    }
    // create thread
//    ret = pthread_create(&hHeadsetReadThread,NULL,HeadsetIoctl_function,(void*)mLad);
}

void AudioYusuHeadSetMessager::SetHeadSetState(int state)
{
    LOG_AudioHeadSetMessager("SetHeadSetState state = %d");
    int ret =0;
    if(HeadsetFd <=0){
        // open headset device
        HeadsetFd = open(HEADSET_PATH, O_RDONLY);
        if(HeadsetFd <0){
            LOGE("open %s error fd = %d",HEADSET_PATH,HeadsetFd);
            return;
        }
    }
    ret = ::ioctl(HeadsetFd,SET_CALL_STATE,state);
}

bool AudioYusuHeadSetMessager::Get_headset_info(void)
{
    headstatusFd = -1;
    headstatusFd = open(YUSUHEADSET_STAUTS_PATH, O_RDONLY,0);

    if(headstatusFd <0){
        LOGE("open %s error fd = %d",YUSUHEADSET_STAUTS_PATH,headstatusFd);
        return false;
    }

    if (read(headstatusFd, rbuf, BUF_LEN) == -1) {
       LOGD("Get_headset_info Can't read headset");
       close(headstatusFd);
       return false;
   }

    if (!strncmp(wbuf, rbuf, BUF_LEN)) {
       LOGD( "Get_headset_info Get_headset_info state  == 1" );
       close(headstatusFd);
       return  true;
   }

    if (!strncmp(wbuf, rbuf, BUF_LEN)) {
       LOGD( "Get_headset_info state  == 2" );
       close(headstatusFd);
       return true;
   }
   else {
       LOGD("Get_headset_info return  false" );
       close(headstatusFd);
       return  false;
   }
}


}


