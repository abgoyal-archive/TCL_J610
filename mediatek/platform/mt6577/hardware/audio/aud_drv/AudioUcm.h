

#ifndef ANDROID_AUDIO_YUSU_USER_CASE_H
#define ANDROID_AUDIO_YUSU_USER_CASE_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <AudioYusuStreamHandler.h>
#include <media/AudioSystem.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include <AudioUcmInterface.h>
#include "AudioYusuDef.h"
#include "AudioIoctl.h"

namespace android {
// ----------------------------------------------------------------------------

class AudioYusuUserCaseManager : public AudioUserCaseManagerInterFace
{
public:
	 AudioYusuUserCaseManager(AudioYusuHardware *hw);
	 ~AudioYusuUserCaseManager();
         virtual status_t    initCheck();

         // tell UCM which mode is, when system boot up , default init for normal mode
         virtual status_t  SetPhoneMode(int32 mode);

         // tell UCM which device is attached and remove.
         virtual status_t  SetInputDevice(uint32 device);
         virtual status_t  SetOutputDevice(uint32 device);

         // init stream index and volume
         virtual status_t InitStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, int indexMin , int indexMax);

         // tell UCM which stream start or stop.
         virtual status_t StreamStart(android_audio_legacy::AudioSystem::stream_type stream);
         virtual status_t StreamStop(android_audio_legacy::AudioSystem::stream_type stream);

        // tell UCM when Record start / stop
         virtual status_t RecordStreamStart();
         virtual status_t RecordStreamStop();

         // tell UCM which stream start or stop.
         virtual status_t SetStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, uint32 level);

         // set and get parameter
         virtual status_t setParameters(const String8& keyValuePairs);
         virtual String8 getParameters(const String8& keys);

         // for debug , dump all information
         void dumpStatus(void);
         void DumpGainTable(int streamgain);

         void DumpVoiceGainTable();
         void DumpRingGainTable();
         void DumpStreamGainTable(int streamgain);
         void DumpMicroPhoneGainTable();
         void DumpSidetoneGainTable();
         void DumpspeechGainTable();

         // set analog gain
         status_t  NormalModeVolumeCheck(void);
         status_t  RingtoneModeVolumeCheck(void);
         status_t IncallModeVolumeCheck(void);
         status_t  CommunicationModeVolumeCheck(void);

         int PolicyForStream(void);
         int PolicyForStreamNormalMode(void);
         int PolicyForStreamRingtoneMode(void);
         int PolicyForStreamIncallMode(void);
         int PolicyForStreamComminicationMode(void);
         bool StreamActive(int streamtype);
         status_t SetAnalogGain(int streamgaintype,int gaintype, uint32 level);
         unsigned int  MapAudioVolume(unsigned int volume);

         //status check function , basic function_table
         bool HeadsetDeviceConnect();
         bool BluetoothscoDeviceConnect();

protected:
	// stream descriptor used for volume control
	class UCMStreamDescriptor
	{
	public:
		UCMStreamDescriptor()
		:	mIndexMin(0), mIndexMax(1), mIndexCur(1), mCanBeMuted(true),mActiveCounter(0) {}
		void dump(char* buffer, size_t size);
		int mIndexMin;		// min volume index
		int mIndexMax;	// max volume index
		int mIndexCur;		// current volume index
		bool mCanBeMuted;	// true is the stream can be muted
		int mActiveCounter; // if stream start , counter will increase  1 ,when stop counter will decrease on.
	};

private:
        UCMStreamDescriptor mStreams[android_audio_legacy::AudioSystem::NUM_STREAM_TYPES+1];// stream descriptors for volume control
        UCMStreamDescriptor mInputStream;
	AudioYusuHardware* mHw;
	uint32 mOutputDevice;
	uint32 mInputDevice;
	uint32 mMode;

	AUDIO_GAIN_TABLE_STRUCT *pGainTable;
	STREAM_VOICE_GAIN_CONTROL_STRUCT *mVoiceStreamstruct ;
	STREAM_GAIN_CONTROL_STRUCT *mSystemStreamstruct;
	STREAM_RING_GAIN_CONTROL_STRUCT *mRingToneStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mMusicStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mAlarmStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mNotificationStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mBluetoothscoStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mEnforceStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mDtmfStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mTsStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mFmStreamstruct;
	STREAM_GAIN_CONTROL_STRUCT *mMatvStreamstruct;
	STREAM_MICROPHONE_GAIN_CONTROL_STRUCT *mMicrphoneStreamstruct;
	STREAM_SIDETONE_GAIN_CONTROL_STRUCT *mSidetoneStreamstruct;
	STREAM_SPEECH_GAIN_CONTROL_STRUCT*mSpeechStreamstruct;


};

// ----------------------------------------------------------------------------
}; // namespace android

#endif;
