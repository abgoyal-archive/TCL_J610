

#ifndef _AUDIO_YUSU_STREAM_HANDLER_H_
#define _AUDIO_YUSU_STREAM_HANDLER_H_

#include <stdint.h>
#include <utils/threads.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include <sys/types.h>
#include <cutils/log.h>
#include "AudioI2S.h"
#include "AudioYusuStreamInInterface.h"

#include "AudioAfe.h"
#include "AudioIoctl.h"
#include "AudioAnalogAfe.h"


namespace android {
// ----------------------------------------------------------------------------
class AudioYusuHardware;
class AudioYusuStreamIn;
class AudioYusuI2SStreamIn;
class AudioYusuStreamOut;
//class AudioStreamOut;
//class AudioStreamIn;
class AudioAfe;
class AudioAnalog;
class LAD;
class AudioAttribute
{
public:
    AudioAttribute();
    ~AudioAttribute();
    int mFormat;
    unsigned int mSampleRate;
    unsigned int mChannelCount;
    unsigned int mInputSource;
    unsigned int mdevices;
    unsigned int macoustic;
};

class InputStreamAttribute
{
public:
    InputStreamAttribute();
    ~InputStreamAttribute();
    uint32_t mDevices;
    int mFormat;
    uint32_t mChannelCount;
    uint32_t mSampleRate;
    android_audio_legacy::AudioSystem::audio_in_acoustics mAcoustics;
};

class AudioStreamHandler
{
public:
   AudioStreamHandler(int Fd,AudioYusuHardware *hw, AudioAfe *AsmReg, AudioAnalog *AnaReg );
	~AudioStreamHandler();

   android_audio_legacy::AudioStreamOut* openOutputStream(uint32 devices,
                                    int *format,
                                    uint32_t *channelCount,
                                    uint32_t *sampleRate,
                                    status_t *status);

   void  closeOutputStream(android_audio_legacy::AudioStreamOut* out);
   void  SetOutputSamplerate(uint32 samplerate);

   android_audio_legacy::AudioStreamIn* openInputStream(uint32 devices,
                                    int *format,
                                    uint32_t *channelCount,
                                    uint32_t *sampleRate,
                                    status_t *status,
                                    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

   void  closeInputStream(android_audio_legacy::AudioStreamIn* out);

   void StoreInputStreamAttribute(uint32 devices,
                                        int *format,
                                        uint32_t *channelCount,
                                        uint32_t *sampleRate,
                                        android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

   int FindFreeOutputStream();
   int FindMatchOutputStream(android_audio_legacy::AudioStreamOut* out);
   int FindFreeInputStream();
   int FindFreeI2SInputStream();
   int FindMatchInputStream(android_audio_legacy::AudioStreamIn* in);
   int FindMatchInputI2SStream(android_audio_legacy::AudioStreamIn* in);
   void StartI2SOutput(void);
   void StopI2SOutput(void);
   void AudioStartRunning(unsigned int OutStreamIndex,unsigned int InterruptCounter);
   void AudioStopRunning();
   void SetSrcBlockRunning(bool Running);
   bool GetSrcBlockRunning(void);
   void AddSrcRunningNumber(void);
   void MinusSrcRunningNumber(void);
   AnalogAFE_Mux FindDeviceWithMux(int device);
   int GetSrcRunningNumber(void);
   void InputStreamLock(void);
   void InputStreamUnLock(void);
   void OutputStreamLock();
   void OutputStreamUnLock();
   void SetOutputStreamToBT(void);
   void RestoreOutputStream(void);

   void StreamHandlerLock();
   void StreamHandlerUnLock();

   // for Audiohardware change mode and set ladplayer
   void SetLadPlayer(int mMode);
   void ResetLadPlayer(int mMode);

   // for yusuStreamOut set ladplayer
   void SetLadPlayer(void);
   void ResetLadPlayer(void);
   void ForceSetLadPlayer(void);
   void ForceResetLadPlayer(int mMode);

   // for I2S setting
   bool SetI2SControl(bool bEnable,int type,uint32 SampleRate=0);

   // close and open record
   bool RecordClose(void);
   bool RecordOpen(void);


   bool IsRecordOn(void);
   bool SetCloseRec(int mPreviousMode,int mMode);
   bool SetOpenRec(int mPreviousMode,int mMode);
   int SetMicType(int InputSource);

   AudioYusuStreamOut   *mOutput[MAX_OUTPUT_STREAM_NUM];
   AudioStreamInInterface    *mInput[MAX_INPUT_STREAM_NUM];
   InputStreamAttribute   mInputAttribure[MAX_INPUT_STREAM_NUM];
   AudioYusuI2SStreamIn   *mI2SInput[MAX_INPUT_STREAM_NUM];
   InputStreamAttribute   mI2SInputAttribure[MAX_INPUT_STREAM_NUM];
   AudioYusuHardware *mAudioHardware;
   AudioAfe *mAfe_handle;
   AudioAnalog *mAnaReg;
   AudioI2S *mI2SInstance;
   uint32 I2SClient;

private:
   float mVolume;
   unsigned int mSrcBlockOccupy;
   int mSrcBlockRunningNum;
   bool mSrcBlockRunning;
   int mFd;
   int mLadPlayerMode;
   Mutex   mLock;
   Mutex   mStreamRecordLock;
   Mutex   mHandlerLock; //avoid ccci to use sreamin which has been released
   pthread_mutex_t mInputstreamMutex;
   pthread_mutex_t mOutputstreamMutex;
};


typedef struct
{
   unsigned int u4WriteIdx;          // Write Index.
   unsigned int u4DMAReadIdx;        // read index
   unsigned int u4DataRemained;      //playback data remaind
   unsigned int u4Write_Length;
} _Buffer_Status;

// ----------------------------------------------------------------------------
}; // namesp

#endif


