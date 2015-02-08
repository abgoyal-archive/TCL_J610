


#ifndef ANDROID_AUDIO_CUSTOM_BASE_H
#define ANDROID_AUDIO_CUSTOM_BASE_H


#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include "AudioCustomizationInterface.h"

namespace android {

class AudioCustomizationBase : public AudioCustomizationInterface
{

public:
        AudioCustomizationBase();
        ~AudioCustomizationBase();

        // AudioCustomizeInterface
        void EXT_DAC_Init(void);
        void EXT_DAC_SetPlaybackFreq(unsigned int frequency);
        void EXT_DAC_TurnOnSpeaker(unsigned int source ,unsigned int speaker);
        void EXT_DAC_TurnOffSpeaker(unsigned int source,unsigned int speaker);
        void EXT_DAC_SetVolume(unsigned int speaker,unsigned int vol);
};

};

#endif

