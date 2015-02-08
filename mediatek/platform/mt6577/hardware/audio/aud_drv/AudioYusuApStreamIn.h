

#ifndef ANDROID_AUDIO_YUSU_APSTREAM_IN_H
#define ANDROID_AUDIO_YUSU_APSTREAM_IN_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"
#include "AudioYusuStreamInInterface.h"

#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"
extern "C" {
#include "bli_exp.h"
}

//#define DIGITAL_MIC
#define MUTE_APAUDIO_PCM_MS   400
#define MUTE_APAUDIO_PCM_MS_NORMAL   1000


namespace android {
// ----------------------------------------------------------------------------

class AudioYusuHardware;

class AudioYusuApStreamIn : public AudioStreamInInterface {
public:
         AudioYusuApStreamIn() ;
     ~AudioYusuApStreamIn();

    virtual status_t    set(
	AudioYusuHardware *hw,
	int mFd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

    uint32_t    sampleRate() const ;
    uint32     GetRecordSampleRate(uint32_t pRate);
    virtual size_t      bufferSize() const { return mReadBufferSize; }
    virtual uint32_t    channels() const;
    virtual int         format() const { return android_audio_legacy::AudioSystem::PCM_16_BIT; }
    virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
    virtual ssize_t     read(void* buffer, ssize_t bytes);
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    standby();
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual unsigned int  getInputFramesLost() const { return 0; }
    virtual void          openRecordDevicePower(void);
    virtual void          CloseRecordDevicePower(void);
    virtual void          getDataFromModem(void);
    virtual void SetDigitalMic(void);

    virtual bool  IsStandby(void);
    virtual bool  RecOpen(void);
    virtual bool  RecClose(void);
    virtual bool CanRecordFM(void);
    virtual bool MutexLock(void);
    virtual bool MutexUnlock(void);
    virtual void SetStreamInputSource(int Inputsource);

    bool SupportedSampleRate(uint32 SampleRate);
    void SetMicSource(int micsource);

    void StereoToMono(short *buffer , int length);

    uint32 GetSrcbufvalidSize(void);
    uint32 GetSrcbufFreeSize(void);

    uint32 CopySrcBuf(char *buffer,uint32 *bytes, char *SrcBuf , uint32 *length);
    void  Dump_StreamIn_PCM(void* buffer, int bytes);
    void CheckNeedMute(void* buffer, int bytes);

    virtual status_t addAudioEffect(effect_handle_t effect){return NO_ERROR;}
    virtual status_t removeAudioEffect(effect_handle_t effect){return NO_ERROR;}

    int StreamInApplyVolume(short *pcm , int count,unsigned short volume);

    AudioYusuHardware *mHw;

private:
    mutable Mutex	mLock;
    mutable Mutex	mRecordLock;
    int      mFd;              /* file ID for write */
    uint32      mSampleRate;      //samping rate running in this bit-stream
    int      mFormat;
    int      mChNum;
    int      mApInputSource;
    uint32   mInputSource;   // this for android audio_source
    uint32   mApSampleRate;
    uint32   mReadBufferSize;
    uint32   mDevice;
    android_audio_legacy::AudioSystem::audio_in_acoustics mAcoustics;
    bool   mstandby;
    int      mMutecount;
    // add for apply fm volume
    unsigned short  mFm_Volume;
    int mFm_Vlume_Index;
    int FM_Mapping_table[16];

    pthread_mutex_t mReadMutex;

    // for SRC
    int8*	 pSrcBuf;
    BLI_HANDLE *pSrcHdl;

    // for src , read buffer
    int8*     pInputbuf;
    uint32   pSrcReadSize;
    uint32   pInputbuflen;
    uint32   pInputSrcWrite;
    uint32   pInputSrcRead;

};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
