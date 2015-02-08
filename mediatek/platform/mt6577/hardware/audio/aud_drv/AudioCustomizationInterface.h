


#ifndef ANDROID_AUDIO_CUSTOM_INTERFACE_H
#define ANDROID_AUDIO_CUSTOM_INTERFACE_H


#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>

#define AUDIO_EXP_DAC_BUF0 0x40
#define AUDIO_EXP_DAC_BUF1 0x80
#define EXT_DAC_I2S                0x1
#define EXT_DAC_LINEIN          0x2

namespace android {

//AudioCustomizeInterface  Interface
class AudioCustomizationInterface
{

public:
    virtual ~AudioCustomizationInterface() {}

    // indicate a change in device connection status
    virtual void EXT_DAC_Init(void) = 0;

    virtual void EXT_DAC_SetPlaybackFreq(unsigned int frequency) =0;

    virtual void EXT_DAC_TurnOnSpeaker(unsigned int source ,unsigned int speaker) =0;

    virtual void EXT_DAC_TurnOffSpeaker(unsigned int source,unsigned int speaker) =0;

    virtual void EXT_DAC_SetVolume(unsigned int speaker,unsigned int vol) =0;

};

};

#endif

