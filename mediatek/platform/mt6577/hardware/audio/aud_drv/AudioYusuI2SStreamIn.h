

#ifndef ANDROID_AUDIO_YUSU_I2SSTREAM_IN_H
#define ANDROID_AUDIO_YUSU_I2SSTREAM_IN_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>


#include <AudioYusuStreamHandler.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"


#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"
#include "AudioI2S.h"

namespace android {
// ----------------------------------------------------------------------------

class AudioYusuHardware;
class AudioYusuStreamHandler;
class AudioI2S;

class AudioYusuI2SStreamIn : public android_audio_legacy::AudioStreamIn {
public:
                        AudioYusuI2SStreamIn() ;
         virtual      ~AudioYusuI2SStreamIn();

    virtual status_t    set(
	AudioYusuHardware *hw,
	int mFd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

    uint32_t    sampleRate() const { return mSampleRate; }
    virtual size_t      bufferSize() const { return mReadBufferSize; }
//    virtual uint32_t    channels() const {return android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;}
    virtual uint32_t    channels() const {return mChannels;}

    virtual int         format() const { return android_audio_legacy::AudioSystem::PCM_16_BIT; }
    virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
    virtual ssize_t     read(void* buffer, ssize_t bytes);
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    standby();
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual unsigned int  getInputFramesLost() const { return 0; }
    virtual status_t addAudioEffect(effect_handle_t effect){return NO_ERROR;}
    virtual status_t removeAudioEffect(effect_handle_t effect){return NO_ERROR;}


    AudioYusuHardware *mHw;
    int mSourceType;

private:
    int      mFd;              /* file ID for write */
    int      mSampleRate;      //samping rate running in this bit-stream
    int      mFormat;
    int      mChNum;
    int      mChannels;
    uint32   mReadBufferSize;
    uint32   mI2Sid;
    uint32_t mDevice;
    AudioI2S*  mI2SHandler;
    pthread_mutex_t mReadMutex;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
