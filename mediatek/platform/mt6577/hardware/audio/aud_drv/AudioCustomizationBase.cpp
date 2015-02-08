

#define LOG_TAG "AudioCustomizationBase"
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#include <AudioCustomizationBase.h>

namespace android {


// ----------------------------------------------------------------------------
// AudioPolicyInterface implementation
// ----------------------------------------------------------------------------

// AudioCustomizeInterface

AudioCustomizationBase::AudioCustomizationBase()
{
    LOGD("AudioCustomizationBase constructor");
}

AudioCustomizationBase::~AudioCustomizationBase()
{
    LOGD("AudioCustomizationBase destructor");
}

void AudioCustomizationBase::EXT_DAC_Init()
{
    LOGD("AudioCustomizationBase EXT_DAC_Init");
}

void AudioCustomizationBase::EXT_DAC_SetPlaybackFreq(unsigned int frequency)
{
    LOGD("EXT_DAC_SetPlaybackFreq frequency = %d",frequency);
}

void AudioCustomizationBase::EXT_DAC_TurnOnSpeaker(unsigned int source , unsigned int speaker)
{
    LOGD("EXT_DAC_TurnOnSpeaker source = %d speaker = %d",source,speaker);

}

void AudioCustomizationBase::EXT_DAC_TurnOffSpeaker(unsigned int source,unsigned int speaker)
{
    LOGD("EXT_DAC_TurnOffSpeaker source = %d,speaker = %d",source,speaker);

}

void AudioCustomizationBase::EXT_DAC_SetVolume(unsigned int speaker,unsigned int vol)
{
    LOGD("EXT_DAC_SetVolume speaker = %d vol = %d",speaker,vol);
}


}; // namespace android
