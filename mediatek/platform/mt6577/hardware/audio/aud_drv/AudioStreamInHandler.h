

#ifndef ANDROID_AUDIO_APSTREAM_HANDLER_H
#define ANDROID_AUDIO_APSTREAM_HANDLER_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"
#include "AudioYusuStreamInInterface.h"
#include "AudioYusuStreamIn.h"
#include "AudioYusuApStreamIn.h"

//#define DUMP_STREAMIN
namespace android {
// ----------------------------------------------------------------------------
enum StreamInActiveType
{
    Active_None_StreamIn,
    Active_Modem_StreamIn,
    Active_Ap_StreamIn,
    Num_Of_StreamIn
};

class AudioYusuHardware;
class AudioAttribute;

class AudioStreamInHandler : public AudioStreamInInterface {
public:
         AudioStreamInHandler() ;
         AudioStreamInHandler(AudioYusuHardware *hw) ;
     ~AudioStreamInHandler();

    virtual status_t    set(
	AudioYusuHardware *hw,
	int mFd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

    void StoreStreamAttribute(
        uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	uint32_t  acoustic
	);

    virtual uint32_t    sampleRate() const ;
    virtual size_t      bufferSize() const ;
    virtual uint32_t    channels() const;
    virtual int         format() const;
    virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
    virtual ssize_t     read(void* buffer, ssize_t bytes);
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    standby();
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual unsigned int  getInputFramesLost() const { return 0; }

    virtual bool  IsStandby(void);
    virtual bool  RecOpen(void);
    virtual bool  RecClose(void);
    virtual void SetMicSource(int micsource);
    virtual bool MutexLock(void);
    virtual bool MutexUnlock(void);
    virtual void SetStreamInputSource(int Inputsource);
    virtual void getDataFromModem();

    virtual status_t addAudioEffect(effect_handle_t effect){return NO_ERROR;}
    virtual status_t removeAudioEffect(effect_handle_t effect){return NO_ERROR;}

    #ifdef DUMP_STREAMIN
    void openDumpStreamFile(void);
    static int record_num;
    #endif

    bool ModemRecordCondition(uint32_t RecordSampleRate,uint32_t devices);

    AudioYusuHardware *mHw;

private:
    mutable Mutex	mRecordHandlerLock;
    mutable Mutex	mReadLock;
    int      mFd;              /* file ID for write */
    //stream atteibute for set confing , when set is return success
    //save streamin setting attribute.
    AudioAttribute msetAttribute;
    //stream atteibute is used for current config
    AudioAttribute mAttribute;
    AudioStreamInInterface *mStreamInput;

    uint32   mDevice;
    uint32   mActiveStream;
    android_audio_legacy::AudioSystem::audio_in_acoustics mAcoustics;
    bool   mstandby;
    pthread_mutex_t mReadMutex;

    #ifdef DUMP_STREAMIN
    FILE *pOutFile;
    #endif
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
