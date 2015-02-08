

#ifndef ANDROID_AUDIO_YUSU_STREAM_OUT_H
#define ANDROID_AUDIO_YUSU_STREAM_OUT_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"

#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"

extern "C" {
#include "bli_exp.h"
}

extern "C" {
#include <dc_removal_flt.h>
}

//#define DUMP_STREAM_OUT
//#define GEN1KTONE
#define ENABLE_DC_REMOVE


namespace android
{
// ----------------------------------------------------------------------------
class AudioYusuHardware;
class AudioYusuStreamHandler;
class LADPlayBuffer;
class AudioCompensationFilter;
class HeadphoneCompensationFilter;
class AudioAttribute;

#define ENABLE_VOIP_BTSCO

class AudioYusuStreamOut : public android_audio_legacy::AudioStreamOut
{
public:

    AudioYusuStreamOut() ;
    virtual    ~AudioYusuStreamOut();

    virtual status_t    set(AudioYusuHardware *hw,
                            int mFd,
                            uint32_t devices,
                            int *pFormat,
                            uint32_t *pChannels,
                            uint32_t *pRate);

    virtual bool        InitOutStream(int StreamIndex, int fd);
    virtual uint32_t    sampleRate() const;
    virtual size_t      bufferSize()  const;
    virtual uint32_t    channels() const ;
    virtual uint32_t    channels_t() const;  // return 1 for mono 2 for stereo
    virtual int         format() const ;
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual uint32_t    latency() const ;
    virtual status_t    setVolume(float left, float right);
    virtual ssize_t     write(const void* buffer, size_t bytes);
    virtual status_t    standby();
    virtual status_t    dump(int fd, const Vector<String16>& args);
	
    virtual uint32_t    GetInterrupttime(void);
    virtual void        ResetGainFlag(void);
    virtual void        DeinitLadplayerBuffer(void);
    virtual void        InitLadplayerBuffer(void);
    virtual int32       GetStreamMode(void);
    virtual status_t  SetOutSamplerate(uint32 samplerate);
    virtual status_t  RestoreOutSamplerate(void);
    virtual status_t  SetBTscoSamplerate(uint32 samplerate);
    virtual status_t  getRenderPosition(uint32_t *dspFrames);
    virtual status_t  SetACFPreviewParameter(void *ptr, int len);
    virtual status_t  SetHCFPreviewParameter(void *ptr, int len);
    virtual bool SetFillinZero(bool bEnable);
    
    virtual status_t  startACFProcess(bool bStart);
    virtual status_t  startHCFProcess(bool bStart);
private:
    class CompensationFilter
    {
    public:
        virtual ~CompensationFilter(){};
        virtual status_t init(int filterMode)=0;
        virtual status_t start(uint32 channel, uint32 samplerate)=0;
        virtual size_t process(const void * inbuffer, size_t bytes, void *outbuffer)=0;
        virtual status_t stop()=0;
        virtual void deInit()=0;
        virtual status_t setParameter(void * param, size_t len)=0;
        virtual bool isStarted()=0;
    };
    class ACFCompensationFilter : public CompensationFilter
    {
    public:
        enum{
        ACF_MODE_DEFAULT=3,
        ACF_MODE_DCR =3,
        ACF_MODE_FILTER =4,
        ACF_MODE_FILTERDCR =5
        };
        ACFCompensationFilter();
        virtual ~ACFCompensationFilter();
        virtual status_t init(int filterMode);
        virtual status_t start(uint32 channel, uint32 samplerate);
        virtual size_t process(const void * inbuffer, size_t bytes, void *outbuffer);
        virtual status_t stop();
        virtual void deInit();
        virtual status_t setParameter(void * param, size_t len);
        virtual bool isStarted();
    private:
        AudioCompensationFilter *mCompFlt;
        int mFilterMode;
        ACFCompensationFilter(const ACFCompensationFilter &);
        ACFCompensationFilter &operator=(const ACFCompensationFilter &);
        
        bool bStarted;
        Mutex mACFLock;
    };
    class HCFCompensationFilter : public CompensationFilter
    {
    public:
        enum{
        HCF_MODE_DEFAULT =4,
        HCF_MODE_FILTER =4
        };
        HCFCompensationFilter();
        virtual ~HCFCompensationFilter();
        virtual status_t init(int filterMode);
        virtual status_t start(uint32 channel, uint32 samplerate);
        virtual size_t process(const void * inbuffer, size_t bytes, void *outbuffer);
        virtual status_t stop();
        virtual void deInit();
        virtual status_t setParameter(void * param, size_t len);
        virtual bool isStarted();
    private:
        HeadphoneCompensationFilter *mCompFlt;
        int mFilterMode;
        HCFCompensationFilter(const HCFCompensationFilter &);
        HCFCompensationFilter &operator=(const HCFCompensationFilter &);
        
        bool bStarted;
        Mutex mHCFLock;
    };
    class DcRemove
    {
    public:
        enum{
        DCR_MODE_1 =0,
        DCR_MODE_2,
        DCR_MODE_3
        };
        DcRemove();
        ~DcRemove();
        status_t init(uint32 channel, uint32 samplerate,uint32 drcMode);
        size_t process(const void * inbuffer, size_t bytes, void *outbuffer);
        status_t close();
    private:
        DCRemove_Handle *mHandle;
        DcRemove(const DcRemove&);
        DcRemove & operator=(const DcRemove&);
    };
    class BliSrc
    {
    public:
        BliSrc();
        ~BliSrc();
        status_t initStatus();
        status_t init(uint32 inSamplerate,uint32 inChannel, uint32 OutSamplerate,uint32 OutChannel);
        size_t process(const void * inbuffer, size_t bytes, void *outbuffer);
        status_t close();
    private:
	    BLI_HANDLE *mHandle;
	    uint8 *mBuffer;
	    status_t mInitCheck;
	    BliSrc(const BliSrc&);
	    BliSrc & operator=(const BliSrc&);
    };
private:
    // write data
	int  padMuteDataToHw(void* buffer, uint32 timeMs);
    size_t stereoToMono(const void* buffer, size_t bytes, void * outbuf);
	size_t writePcmDataToModem (const void* buffer, size_t bytes);
	size_t writePcmDataToBtSco(void* buffer, size_t bytes);
	size_t writePcmDataToAudioHw(void* buffer, size_t bytes);

	//sip call to btsco
	void  startAudioSco();
	bool isAudioScoTurnoff();
	void turnOnAudioSco();
	// audio hw
	void prepareAudioHw(void);
	void startAudioHw();
	bool isAudioHwTurnOff();
	void turnOnAudioHw();

    uint32_t    calInterrupttime(void);
private:
	AudioYusuHardware   *mAudioHardware;
	bool                mStandby;
	int32               mFd;
    //below is stream data
    AudioAttribute      *mAttribute;
    AudioAttribute      *mBTscoAttribute;
    mutable uint32      mLatency;
    uint32              mInterruptCounter;

    uint32              mHwBufferSize;
    uint32              mGain;
    bool                mSetGainFlag;
    const int32         mOutStreamIndex;
    // ladplayer
    LADPlayBuffer       *mLADPlayBuffer;
    bool                mFillinZero;
	// filter
	CompensationFilter *mAudioCompFlt;
    CompensationFilter *mHeadphoneCompFlt;
	//DcRemove
	DcRemove *mDcRemove;
	BliSrc * mBliSrc;

	uint8 *mSwapBufferOne;
	uint8 *mSwapBufferTwo;

    //HDMI
    uint32              mMuteHwMs;;
    // stream in which mode
    int32               mStreamMode;
    uint32              mDevice;

	Mutex               mLock;


};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
