

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#define LOG_TAG "AudioYusuI2SStreamIn"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuI2SStreamIn.h"
#include "audio_custom_exp.h"

#include <assert.h>

#ifdef ENABLE_LOG_I2SSTREAMIN
#undef LOGV
#define LOGV(...) LOGD(__VA_ARGS__)
#endif

namespace android {

status_t AudioYusuI2SStreamIn::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
   bool ret = true;
   LOGD("AudioYusuI2SStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
   // check values


   if ((*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT) || (*pChannels != channels()) || (*pRate != sampleRate()) ) {
      LOGE("Error opening input Rate(%d), ch(%d)",sampleRate(),channels());
      goto SET_EXIT;
   }


    mSourceType = mI2SHandler->GetSourceType();

    mI2Sid = mI2SHandler->open();
    LOGV("AudioI2S mI2Sid = %d",mI2Sid);
    mI2SHandler->start(mI2Sid,(I2STYPE)mSourceType);
    LOGV("AudioYusuI2SStreamIn for I2S if work and return ok");
    return NO_ERROR;

SET_EXIT:
    // modify default paramters and let Audiorecord openagina for reampler.
   *pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT;
   *pChannels =  channels();
   *pRate = sampleRate ();
    return BAD_VALUE;

}

AudioYusuI2SStreamIn::AudioYusuI2SStreamIn()
{
    LOGV("AudioYusuI2SStreamIn Constructor");
    int ret =0;
    mI2Sid = NULL;
    // init with default pcm parameters
    mI2SHandler = AudioI2S::getInstance();


    mSampleRate = mI2SHandler->samplerate();
    mChannels = android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;
    LOGD("Constructor mSampleRate=%d, mChannels=%d",mSampleRate,mChannels);


    mReadBufferSize = mI2SHandler->GetReadBufferSize();
    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        LOGE("Failed to initialize  mReadMutex");
    mHw = NULL;
    mFd = -1;
    mSourceType=-1;
}

 status_t  AudioYusuI2SStreamIn::standby()
{
    LOGV("standby");
    return NO_ERROR;
}

AudioYusuI2SStreamIn::~AudioYusuI2SStreamIn()
{
    LOGV("~AudioYusuI2SStreamIn");
    if(mI2SHandler != NULL && mI2Sid != 0){
        /*
        if(mI2SHandler->GetState () == I2S0_INPUT_START){
            mI2SHandler->stop(mI2Sid,(I2STYPE)mSourceType);
        }
        */
        mI2SHandler->stop(mI2Sid,(I2STYPE)mSourceType);

        mI2SHandler->close(mI2Sid);
        mI2SHandler = NULL;
    }
}

ssize_t AudioYusuI2SStreamIn::read(void* buffer, ssize_t bytes)
{
   int readsize = 0;
   LOGD("before read bytes = %d readsize = %d",bytes,readsize);
   readsize = mI2SHandler->read(mI2Sid, buffer,mReadBufferSize);
   LOGD("after read bytes = %d readsize = %d",bytes,readsize);
   return readsize;
}

status_t AudioYusuI2SStreamIn::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioYusuStreamIn::dump\n");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannel count: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmAudioHardware: %p\n", mHw);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioYusuI2SStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    LOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuI2SStreamIn::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}


// ----------------------------------------------------------------------------
}

