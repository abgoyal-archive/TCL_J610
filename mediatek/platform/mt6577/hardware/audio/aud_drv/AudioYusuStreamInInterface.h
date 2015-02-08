

#ifndef ANDROID_AUDIO_YUSU_STREAM_ININTERFACE_H
#define ANDROID_AUDIO_YUSU_STREAM_ININTERFACE_H


#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <media/IAudioFlinger.h>

#include <AudioYusuStreamHandler.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"
#include <hardware_legacy/AudioHardwareBase.h>


namespace android {
// ----------------------------------------------------------------------------
class AudioYusuHardware;


class AudioStreamInInterface : public android_audio_legacy::AudioStreamIn{
public:

    virtual     ~AudioStreamInInterface(){;}

    virtual status_t	set(AudioYusuHardware *hw,
							 int mFd,
							 uint32_t devices,
							 int *pFormat,
							 uint32_t *pChannels,
							 uint32_t *pRate,
							 android_audio_legacy::AudioSystem::audio_in_acoustics acoustics) =0;

    /** return audio sampling rate in hz - eg. 44100 */
    virtual uint32_t    sampleRate() const = 0;

    /** return the input buffer size allowed by audio driver */
    virtual size_t      bufferSize() const = 0;

    /** return input channel mask */
    virtual uint32_t    channels() const = 0;

    /**
     * return audio format in 8bit or 16bit PCM format -
     * eg. AudioSystem:PCM_16_BIT
     */
    virtual int         format() const = 0;

    /**
     * return the frame size (number of bytes per sample).
     */
    uint32_t    frameSize() const { return android_audio_legacy::AudioSystem::popCount(channels())*((format()==android_audio_legacy::AudioSystem::PCM_16_BIT)?sizeof(int16_t):sizeof(int8_t)); }

    /** set the input gain for the audio driver. This method is for
     *  for future use */
    virtual status_t    setGain(float gain) = 0;

    /** read audio buffer in from audio driver */
    virtual ssize_t     read(void* buffer, ssize_t bytes) = 0;

    /** dump the state of the audio input device */
    virtual status_t dump(int fd, const Vector<String16>& args) = 0;

    /**
     * Put the audio hardware input into standby mode. Returns
     * status based on include/utils/Errors.h
     */
    virtual status_t    standby() = 0;

    // set/get audio input parameters. The function accepts a list of  parameters
    // key value pairs in the form: key1=value1;key2=value2;...
    // Some keys are reserved for standard parameters (See AudioParameter class).
    // If the implementation does not accept a parameter change while the output is
    // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
    // The audio flinger will put the input in standby and then change the parameter value.
    virtual status_t    setParameters(const String8& keyValuePairs) = 0;
    virtual String8     getParameters(const String8& keys) = 0;


    // Return the amount of input frames lost in the audio driver since the last call of this function.
    // Audio driver is expected to reset the value to 0 and restart counting upon returning the current value by this function call.
    // Such loss typically occurs when the user space process is blocked longer than the capacity of audio driver buffers.
    // Unit: the number of input audio frames
    virtual unsigned int  getInputFramesLost() const = 0;

    virtual void getDataFromModem()=0;

    virtual void  SetStreamInputSource(int source) =0;
    virtual bool  IsStandby() =0;
    virtual bool  RecOpen() =0;
    virtual bool  RecClose() =0;
    virtual bool  MutexLock() =0;
    virtual bool  MutexUnlock() =0;

};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
