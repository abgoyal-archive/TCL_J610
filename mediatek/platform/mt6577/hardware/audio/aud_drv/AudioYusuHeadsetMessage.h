
#ifndef _AUDIO_HEADSET_MESSAGE_H_
#define _AUDIO_HEADSET_MESSAGE_H_

#include "AudioYusuDef.h"
#include <linux/ioctl.h>


#define ACCDET_IOC_MAGIC 'A'
#define ACCDET_INIT       _IO(ACCDET_IOC_MAGIC,0)  // call wehn first time
#define SET_CALL_STATE    _IO(ACCDET_IOC_MAGIC,1)  // when state is changing , tell headset driver.
#define GET_BUTTON_STATUS _IO(ACCDET_IOC_MAGIC,2)  // ioctl to get hook button state.

static const char *HEADSET_PATH= "/dev/accdet";
static const char *YUSUHEADSET_STAUTS_PATH ="/sys/class/switch/h2w/state";

namespace android {

class LAD;
class AudioYusuHeadSetMessager
{
public:
   AudioYusuHeadSetMessager(LAD *lad);
   void SetHeadSetState(int state);
   bool SetHeadInit(void);
   static  bool Get_headset_info(void);
private:
   LAD *mLad;
   pthread_t hHeadsetReadThread;
};

}; // namespace android

#endif

