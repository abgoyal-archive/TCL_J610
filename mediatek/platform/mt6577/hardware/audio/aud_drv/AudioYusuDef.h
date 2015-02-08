

#ifndef _AUDIO_YUSU_DEF_H_
#define _AUDIO_YUSU_DEF_H_


#ifndef ASSERT
#include <assert.h>
#define ASSERT(x)    assert(x)
#endif


//#define PC_EMULATION
//#define FAKE_CCCI


//----------------log definition---------------------------

#define ENABLE_LOG_HARDWARE
#define ENABLE_LOG_STREAMOUT
#define ENABLE_LOG_STREAMIN
#define ENABLE_LOG_VOLUMECONTROLLER
#define ENABLE_LOG_STREAMHANDLER
//#define ENABLE_LOG_ANA
//#define ENABLE_LOG_AFE
#define ENABLE_LOG_AUDIOFLINGER
#define ENABLE_LOG_LAD

//#define ENABLE_LOG_AUDIOPOLICYSERVICE
//#define ENABLE_LOG_AUDIOPOLICYANAGER
//#define ENABLE_LOG_REAMPLERMTK
//#define ENABLE_LOG_A2DPINTERFACE
//#define ENABLE_LOG_AUDIO_MIXER
//#define ENABLE_LOG_AUDIOHEADSETMESSAGER
//#define ENABLE_LOG_I2S
//#define ENABLE_LOG_I2SSTREAMIN
#define TEMP_SOLUTION_VM

#if defined(PC_EMULATION)
#define YAD_LOGW   printf
#define YAD_LOGE   printf
#define YAD_LOGD   printf
#else
#ifdef  ENABLE_LOG_STREAMIN
   #define YAD_LOGV   LOGD
#else
   #define YAD_LOGV   LOGV
#endif
#define YAD_LOGW   LOGW
#define YAD_LOGE   LOGE
#define YAD_LOGD   LOGD
#endif



//--------------- Type definition ---------------------

#ifndef int8
typedef char int8;
#endif
#ifndef uint8
typedef unsigned char  uint8;
#endif

#ifndef int16
typedef short  int16;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef uint32
typedef unsigned int   uint32;
#endif

#ifndef uint32
typedef unsigned int   UINT32;
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef null
#define null 0
#endif

namespace android {

#ifndef int8
    typedef char int8;
#endif
#ifndef uint8
    typedef unsigned char  uint8;
#endif

#ifndef int16
typedef short  int16;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef uint32
typedef unsigned int   uint32;
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef null
    #define null 0
#endif

//---------------structure definition------------------------

struct rb {
   char *pBufBase;
   char *pRead;
   char *pWrite;
   int   bufLen;
};

#if defined(PC_EMULATION)

enum audio_mode {
   MODE_NORMAL = 0,
   MODE_RINGTONE = 1,
   MODE_IN_CALL = 2
};

class AudioSystem
{
public:

    enum audio_format {
        FORMAT_DEFAULT = 0,
        PCM_16_BIT,
        PCM_8_BIT,
        VM_FMT,
        INVALID_FORMAT
    };

    enum audio_mode {
        MODE_INVALID = -2,
        MODE_CURRENT = -1,
        MODE_NORMAL = 0,
        MODE_RINGTONE,
        MODE_IN_CALL,
        NUM_MODES  // not a valid entry, denotes end-of-list
    };
};
#endif



extern void AAD_Sleep_1ms(int ms);
extern int rb_getDataCount(rb *rb1);
extern void rb_copyToLinear(char *buf, rb *rb1, int count);
extern void rb_copyFromLinear(rb *rb1, char *buf, int count);
extern void rb_copyEmpty(rb *rbt, rb *rbs);
extern void rb_writeDataValue(rb *rb1, int value, int count);

extern void rb_copyFromLinearSRC(void *pSrcHdl, rb *rbt, int8 *buf, int32 num, int32 srt, int32 srs);
extern void rb_copyEmptySRC(void *pSrcHdl, rb *rbt, rb *rbs, int32 srt, int32 srs);
}; // namespace android

#endif   //_AUDIO_YUSU_DEF_H_

