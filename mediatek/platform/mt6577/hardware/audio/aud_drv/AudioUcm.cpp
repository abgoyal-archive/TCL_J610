

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

#define LOG_TAG "AudioYusuUCM"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioUcm.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include <CFG_AUDIO_File.h>

#include <assert.h>

#define AUDIOMAXVOLUME (4)
#define DUMP_UCM_STATUS

#ifdef ENABLE_LOG_AUDIOYUSUUCM
#undef LOGV
#define LOGV(...) LOGD(__VA_ARGS__)
#endif

namespace android {

bool AudioYusuUserCaseManager::HeadsetDeviceConnect()
{
    if(mOutputDevice&android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE ||
            mOutputDevice&android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET){
        return true;
    }
    else{
        return false;
    }
}

bool AudioYusuUserCaseManager::BluetoothscoDeviceConnect(){
    if(mOutputDevice&android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO ||
            mOutputDevice&android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
                mOutputDevice&android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT){
        return true;
    }
    else{
        return false;
    }
}

AudioYusuUserCaseManager::AudioYusuUserCaseManager(AudioYusuHardware *hw)
{
    LOGD("AudioYusuUserCaseManager constructor");
    mHw = hw;
    if(mHw == NULL){
        LOGE("mHw == NULL");
    }
    LOGD("mHw = %p",mHw);
    //set default device
    mOutputDevice =android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    mInputDevice =android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    pGainTable = NULL;
    pGainTable = (AUDIO_GAIN_TABLE_STRUCT *)malloc(sizeof(AUDIO_GAIN_TABLE_STRUCT));

    if(pGainTable){
        GetAudioGainTableParamFromNV(pGainTable);
    }

    DumpGainTable(VOICE_GAIN);
    DumpGainTable(SPEECH_GAIN);
    DumpGainTable(SIDETONE_GAIN);
    DumpGainTable(MICROPHNE_GAIN);
    DumpGainTable(RINGTONE_GAIN);
    DumpGainTable(MUSIC_GAIN);
    DumpGainTable(ALARM_GAIN);
    DumpGainTable(NOTIFICATION_GAIN);
    DumpGainTable(BLUETOOTH_SCO_GAIN);
    DumpGainTable(ENFORCEAUDIBLE_GAIN);
    DumpGainTable(FTMF_GAIN);
    DumpGainTable(TTS_GAIN);
    DumpGainTable(FM_GAIN);
    DumpGainTable(MATV_GAIN);

    mVoiceStreamstruct = &(pGainTable->Voice_Gain_table);
    mSystemStreamstruct = &(pGainTable->System_Gain_table);
    mRingToneStreamstruct = &(pGainTable->Ring_Gain_table);
    mMusicStreamstruct = &(pGainTable->Music_Gain_table);
    mAlarmStreamstruct = &(pGainTable->Alarm_Gain_table);
    mNotificationStreamstruct = &(pGainTable->Notification_Gain_table);
    mBluetoothscoStreamstruct = &(pGainTable->Bluetooth_sco_Gain_table);
    mEnforceStreamstruct = &(pGainTable->EnforceAudible_table);
    mDtmfStreamstruct = &(pGainTable->Dtmf_Gain_table);
    mTsStreamstruct = &(pGainTable->Tts_Gain_table);
    mFmStreamstruct = &(pGainTable->Fm_Gain_table);
    mMatvStreamstruct = &(pGainTable->Matv_Gain_table);
    mMicrphoneStreamstruct = &(pGainTable->Microphone_Gain_table);
    mSidetoneStreamstruct = &(pGainTable->Sidetone_Gain_table);
    mSpeechStreamstruct = &(pGainTable->Speech_Gain_table);
}

AudioYusuUserCaseManager::~AudioYusuUserCaseManager()
{
    if(pGainTable){
        free(pGainTable);
        pGainTable = NULL;
    }
}

status_t AudioYusuUserCaseManager::initCheck()
{
    LOGD("init cehck");
    return NO_ERROR;
}

bool AudioYusuUserCaseManager::StreamActive(int streamtype)
{
    if(mStreams[streamtype].mActiveCounter){
        return true;
    }
    else{
        return false;
    }

}

int AudioYusuUserCaseManager::PolicyForStreamNormalMode()
{
    if(StreamActive(RINGTONE_GAIN))
        return RINGTONE_GAIN;
    else if(StreamActive(NOTIFICATION_GAIN))
        return NOTIFICATION_GAIN;
    else if(StreamActive(ENFORCEAUDIBLE_GAIN))
        return ENFORCEAUDIBLE_GAIN;
    else if(StreamActive(MATV_GAIN))
        return MATV_GAIN;
    else if(StreamActive(MUSIC_GAIN))
        return MUSIC_GAIN;
    else
        return MUSIC_GAIN;
}

int AudioYusuUserCaseManager::PolicyForStreamRingtoneMode()
{
    return RINGTONE_GAIN;
}

int AudioYusuUserCaseManager::PolicyForStreamIncallMode(void)
{
    return VOICE_GAIN;
}

int AudioYusuUserCaseManager::PolicyForStreamComminicationMode(void)
{
    return VOICE_GAIN;
}


int AudioYusuUserCaseManager::PolicyForStream()
{

    switch(mMode){
        case android_audio_legacy::AudioSystem::MODE_NORMAL:
            return PolicyForStreamNormalMode();
            break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
            return PolicyForStreamRingtoneMode();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
            return PolicyForStreamIncallMode();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
            return PolicyForStreamComminicationMode();
            break;
        default:
            LOGE("policy for stream with mode = %d",mMode);
            return -1;
    }
    return -1;
}

status_t  AudioYusuUserCaseManager::NormalModeVolumeCheck()
{
    // here to check volume in normal mode , first decide which stream we  need to apply
    // and then call set analog gain
    int streamtype = PolicyForStream();
    LOGD("NormalModeVolumeCheck streamtype = %d",streamtype);
    if(streamtype < 0){
        return BAD_TYPE;
    }
    if(HeadsetDeviceConnect ()){
        SetAnalogGain (streamtype, STREAM_HEADSET, mStreams[streamtype].mIndexCur);
    }
    else{
        SetAnalogGain (streamtype, STREAM_SPEAKER, mStreams[streamtype].mIndexCur);
    }
    return NO_ERROR;
}

status_t  AudioYusuUserCaseManager::RingtoneModeVolumeCheck()
{
    // here to check volume in ringtone mode , first decide which stream we  need to apply
    // and then call set analog gain
    if(HeadsetDeviceConnect ()){
        SetAnalogGain (RINGTONE_GAIN, RING_MODE_HEADSET, mStreams[RINGTONE_GAIN].mIndexCur);
    }
    else{
        SetAnalogGain (RINGTONE_GAIN, RING_MODE_SPEAKER, mStreams[RINGTONE_GAIN].mIndexCur);
    }
    return NO_ERROR;
}

status_t  AudioYusuUserCaseManager::IncallModeVolumeCheck()
{
    // here to check volume in incall mode , first decide which stream we  need to apply
    // and then call set analog gain
    if(mOutputDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER){
        SetAnalogGain (SPEECH_GAIN, SPEECH_SPEAKER, mStreams[SPEECH_GAIN].mIndexCur);
    }
    else if(mOutputDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE){
        SetAnalogGain (SPEECH_GAIN, SPEECH_NORMAL, mStreams[SPEECH_GAIN].mIndexCur);
    }
    else if(HeadsetDeviceConnect ()){
        SetAnalogGain (SPEECH_GAIN, SPEECH_HEADSET, mStreams[SPEECH_GAIN].mIndexCur);
    }
    else if(BluetoothscoDeviceConnect ()){
        // here blue tooth can set digital gain to modem.
        SetAnalogGain (SPEECH_GAIN, SPEECH_BT, mStreams[SPEECH_GAIN].mIndexCur);
    }
    else{
        LOGD("mOutputDevice = %x",mOutputDevice);
    }
    return NO_ERROR;
}


status_t  AudioYusuUserCaseManager::CommunicationModeVolumeCheck()
{
    // here to check volume in incall mode , first decide which stream we  need to apply
    // and then call set analog gain
    if(mOutputDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER){
        SetAnalogGain (VOICE_GAIN, SIP_VOICE_SPEAKER, mStreams[VOICE_GAIN].mIndexCur);
    }
    else if(mOutputDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE){
        SetAnalogGain (VOICE_GAIN, SIP_VOICE_RECEIVER, mStreams[VOICE_GAIN].mIndexCur);
    }
    else if(HeadsetDeviceConnect ()){
        SetAnalogGain (VOICE_GAIN, SIP_VOICE_HEADSET, mStreams[VOICE_GAIN].mIndexCur);
    }
    else{
        LOGD("mOutputDevice = %x",mOutputDevice);
    }
    return NO_ERROR;
}


status_t  AudioYusuUserCaseManager::SetPhoneMode(int32 mode)
{
    LOGD("SetPhoneMode mode = %d",mode);
    mMode = mode;
    #ifdef DUMP_UCM_STATUS
    dumpStatus();
    #endif
    switch(mMode){
        case android_audio_legacy::AudioSystem::MODE_NORMAL:
            NormalModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
            RingtoneModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
            IncallModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
            CommunicationModeVolumeCheck();
            break;
    }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::SetInputDevice(uint32 device)
{
    LOGD("SetInputDevice device = %x",device);
    mInputDevice = device;
    #ifdef DUMP_UCM_STATUS
    dumpStatus();
    #endif
    return NO_ERROR;
}
status_t AudioYusuUserCaseManager::SetOutputDevice(uint32 device)
{
    LOGD("SetOutputDevice device = %x",device);
    // do set analog gain id device is cahnge
    if(mOutputDevice != device){
        switch(mMode){
            case android_audio_legacy::AudioSystem::MODE_NORMAL:
                NormalModeVolumeCheck();
                break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
                RingtoneModeVolumeCheck();
                break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
                IncallModeVolumeCheck();
                break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
                CommunicationModeVolumeCheck();
                break;
        }
    }
    mOutputDevice = device;
    #ifdef DUMP_UCM_STATUS
    dumpStatus();
    #endif
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::InitStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, int indexMin , int indexMax)
{
    LOGD("InitStreamLevel stream = %d indexMin = %d indexMax = %d mHw = %p"
        ,stream,indexMin,indexMax,mHw);
    mStreams[stream].mIndexMin = indexMin;
    mStreams[stream].mIndexMax = indexMax;
    if(stream == android_audio_legacy::AudioSystem::VOICE_CALL){
        mStreams[SPEECH_GAIN].mIndexMin = indexMin;
        mStreams[SPEECH_GAIN].mIndexMax = indexMax;
    }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::StreamStart(android_audio_legacy::AudioSystem::stream_type stream)
{
    LOGD("StreamStart stream = %d mHw = %p",stream,mHw);
    mStreams[stream].mActiveCounter++;
    #ifdef DUMP_UCM_STATUS
    dumpStatus();
    #endif

    switch(mMode){
        case android_audio_legacy::AudioSystem::MODE_NORMAL:
            NormalModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
            RingtoneModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
            // when incall , whenerver which stream start , speech gain is applied.
            //IncallModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
            CommunicationModeVolumeCheck();
            break;
    }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::StreamStop(android_audio_legacy::AudioSystem::stream_type stream)
{
    LOGD("StreamStop stream = %d",stream);
    mStreams[stream].mActiveCounter--;
    if(mStreams[stream].mActiveCounter<0){
        LOGE("mActiveCounter = %d  stream = %d",mStreams[stream].mActiveCounter,stream);
    }
   #ifdef DUMP_UCM_STATUS
    dumpStatus();
    #endif
    switch(mMode){
        case android_audio_legacy::AudioSystem::MODE_NORMAL:
            NormalModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
            RingtoneModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
            //IncallModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
            CommunicationModeVolumeCheck();
            break;
    }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::RecordStreamStart()
{
    LOGD("RecordStreamStart");
    mInputStream.mActiveCounter = 1;
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::RecordStreamStop()
{
    LOGD("RecordStreamStop");
    mInputStream.mActiveCounter = 0;
    return NO_ERROR;
}

// tell UCM which steam start or stop.
status_t AudioYusuUserCaseManager::SetStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, uint32 level)
{
    LOGD("SetStreamLevel stream = %d level = %d mHw = %p",stream,level,mHw);
    mStreams[stream].mIndexCur = level;

    if(stream == android_audio_legacy::AudioSystem::VOICE_CALL){
        mStreams[SPEECH_GAIN].mIndexCur = level;
    }

    uint32 ModeForStream = PolicyForStream();
    LOGD("SetStreamLevel ModeForStream = %d stream = %d",ModeForStream,stream);
    if(ModeForStream == stream){
        switch(mMode){
        case android_audio_legacy::AudioSystem::MODE_NORMAL:
            NormalModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_RINGTONE:
            RingtoneModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_CALL:
            IncallModeVolumeCheck();
            break;
        case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
            CommunicationModeVolumeCheck();
            break;
        }
    }
    return NO_ERROR;
}

void AudioYusuUserCaseManager::DumpVoiceGainTable()
{
    LOGD("DumpVoiceGainTable");
    STREAM_VOICE_GAIN_CONTROL_STRUCT *pVoiceGainTable = &(pGainTable->Voice_Gain_table);
    for(int i=0 ; i < MAX_VOICE_STREAM_TYPE; i++){
        for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
            AUDIO_GAIN_CONTROL_STRUCT *ptemp = &(pVoiceGainTable->Voice_Gain[i][j]);
            LOGD("level = %d gain = %x gain = %x",
                j,ptemp->u8Gain_digital,ptemp->u32Gain_PGA_Amp);
        }
    }
}
void AudioYusuUserCaseManager::DumpRingGainTable()
{
    LOGD("DumpRingGainTable");
    STREAM_RING_GAIN_CONTROL_STRUCT *pRingGainTable = &(pGainTable->Ring_Gain_table);
    for(int i=0 ; i < MAX_RING_TYPE; i++){
        for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
            LOGD("level = %d gain = %x gain = %x",
                j,pRingGainTable->Ring_Stream_Gain[i][j].u8Gain_digital,pRingGainTable->Ring_Stream_Gain[i][j].u32Gain_PGA_Amp);
        }
    }
}

void AudioYusuUserCaseManager::DumpStreamGainTable(int streamType)
{
    LOGD("DumpStreamGainTable streamType = %d",streamType);
    STREAM_GAIN_CONTROL_STRUCT *pStreamGainTable = NULL;
    switch(streamType){
       case SYSTEM_GAIN:
           pStreamGainTable = &(pGainTable->System_Gain_table);
           break;
       case MUSIC_GAIN:
           pStreamGainTable = &(pGainTable->Music_Gain_table);
           break;
       case ALARM_GAIN:
           pStreamGainTable = &(pGainTable->Alarm_Gain_table);
           break;
       case NOTIFICATION_GAIN:
           pStreamGainTable = &(pGainTable->Notification_Gain_table);
           break;
       case BLUETOOTH_SCO_GAIN:
           pStreamGainTable =&(pGainTable->Bluetooth_sco_Gain_table);
           break;
       case ENFORCEAUDIBLE_GAIN:
           pStreamGainTable =&(pGainTable->EnforceAudible_table);
           break;
       case FTMF_GAIN:
           pStreamGainTable =&(pGainTable->Dtmf_Gain_table);
           break;
       case TTS_GAIN:
           pStreamGainTable =&(pGainTable->Tts_Gain_table);
           break;
       case FM_GAIN:
           pStreamGainTable =&(pGainTable->Fm_Gain_table);
           break;
       case MATV_GAIN:
           pStreamGainTable =&(pGainTable->Matv_Gain_table);
           break;
    }
    for(int i=0 ; i < MAX_STREAM_TYPE; i++){
        for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
            AUDIO_GAIN_CONTROL_STRUCT *ptemp = &(pStreamGainTable->Stream_Gain[i][j]);
            LOGD("level = %d gain = %x gain = %x",
                j,ptemp->u8Gain_digital,ptemp->u32Gain_PGA_Amp);
        }
    }
}

void AudioYusuUserCaseManager::DumpMicroPhoneGainTable()
{
    LOGD("DumpMicroPhoneGainTable");
    STREAM_MICROPHONE_GAIN_CONTROL_STRUCT *pMicroPhoneGainTable = &(pGainTable->Microphone_Gain_table);
    for(int i=0 ; i < NUM_OF_MICGAIN; i+=2){
            LOGD("level = %d gain = %x level = %d gain = %x",
                i,pMicroPhoneGainTable->Mic_Gain[i],i+1,pMicroPhoneGainTable->Mic_Gain[i+1]);
     }
}

void AudioYusuUserCaseManager::DumpSidetoneGainTable()
{
    LOGD("DumpSidetoneGainTable");
    STREAM_SIDETONE_GAIN_CONTROL_STRUCT *pSidetoneGainTable = &(pGainTable->Sidetone_Gain_table);
    for(int i=0 ; i < NUM_OF_SIDETONEGAIN; i+=2){
            LOGD("level = %d gain = %x level = %d gain = %x",
                i,pSidetoneGainTable->SdieTone_Gain[i],i+1,pSidetoneGainTable->SdieTone_Gain[i+1]);
     }
}

void AudioYusuUserCaseManager::DumpspeechGainTable()
{
    LOGD("DumpspeechGainTable");
    STREAM_SPEECH_GAIN_CONTROL_STRUCT *pSpeechGainTable = &(pGainTable->Speech_Gain_table);
    for(int i=0 ; i < NUM_OF_SPEECHGAIN; i++){
         for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
             AUDIO_GAIN_CONTROL_STRUCT *ptemp = &(pSpeechGainTable->Speech_Gain[i][j]);
               LOGD("level = %d gain = %x gain = %x",
                   j,ptemp->u8Gain_digital,ptemp->u32Gain_PGA_Amp);
         }
     }

}

// tell UCM which stream start or stop.
void AudioYusuUserCaseManager::DumpGainTable(int streamgain)
{
    switch(streamgain){
        case VOICE_GAIN:
            DumpVoiceGainTable();
            break;
        case RINGTONE_GAIN:
            DumpRingGainTable();
            break;
        case SYSTEM_GAIN:
        case MUSIC_GAIN:
        case ALARM_GAIN:
        case NOTIFICATION_GAIN:
        case BLUETOOTH_SCO_GAIN:
        case ENFORCEAUDIBLE_GAIN:
        case FTMF_GAIN:
        case TTS_GAIN:
        case FM_GAIN:
        case MATV_GAIN:
            DumpStreamGainTable(streamgain);
            break;
        case MICROPHNE_GAIN:
            DumpMicroPhoneGainTable();
            break;
        case SIDETONE_GAIN:
            DumpSidetoneGainTable();
            break;
        case SPEECH_GAIN:
            DumpspeechGainTable();
            break;
        default:
            DumpVoiceGainTable();
            DumpRingGainTable();
            DumpStreamGainTable(NULL);
            DumpMicroPhoneGainTable();
            DumpSidetoneGainTable();
            DumpspeechGainTable();
    }
}


// tell UCM which stream start or stop.
void  AudioYusuUserCaseManager::dumpStatus()
{
    for(int i=0; i < android_audio_legacy::AudioSystem::NUM_STREAM_TYPES;i++){
        LOGD("Stream type %d  MaxLevel %d MinLevel %d Curlevel %d ActiveCounter %d",
            i,mStreams[i].mIndexMax,mStreams[i].mIndexMin,mStreams[i].mIndexCur,mStreams[i].mActiveCounter);
    }
    LOGD("OutputDevice = %x InputDevice = %x mode = %d",mOutputDevice,mInputDevice,mMode);
}


unsigned int  AudioYusuUserCaseManager::MapAudioVolume(unsigned int volume)
{
    LOGD("MapAudioVolume volume = %x",volume);
    volume = volume >> 24;
    volume =AUDIOMAXVOLUME + volume;
    return volume;
}


// tell UCM which stream start or stop.
status_t AudioYusuUserCaseManager::SetAnalogGain(int streamgaintype,int gaintype, uint32 level)
{
    LOGD("SetAnalogGain streamgaintype = %d gaintype = %d level = %d",streamgaintype,gaintype,level);
    if(mHw == NULL){
        LOGE("Setanalog gain but mhw == NULL");
        return BAD_TYPE;
    }

    switch(streamgaintype){
        case VOICE_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mVoiceStreamstruct->Voice_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case SYSTEM_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mSystemStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case RINGTONE_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(pGainTable->Ring_Gain_table.Ring_Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case MUSIC_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mMusicStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case ALARM_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mAlarmStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case NOTIFICATION_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mNotificationStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case BLUETOOTH_SCO_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mBluetoothscoStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case ENFORCEAUDIBLE_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mEnforceStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case FTMF_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mDtmfStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case TTS_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mTsStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case FM_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mFmStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case MATV_GAIN:
            mHw->SetAnalogGain (MapAudioVolume(mMatvStreamstruct->Stream_Gain[gaintype][level].u32Gain_PGA_Amp));
            break;
        case MICROPHNE_GAIN:
            break;
        case SIDETONE_GAIN:
            break;
        case SPEECH_GAIN:{
            int32 i4SpeechGain =0;
            i4SpeechGain = (mSpeechStreamstruct->Speech_Gain[gaintype][level].u32Gain_PGA_Amp) >> 24;
            i4SpeechGain = i4SpeechGain << 8;
            i4SpeechGain |= mSpeechStreamstruct->Speech_Gain[gaintype][level].u8Gain_digital;
            mHw->SetSpeechGaintableVolume (i4SpeechGain);
            break;
        }
        default:
            break;
   }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::setParameters(const String8& keyValuePairs)
{
    status_t status = NO_ERROR;
    return status;
}

String8 AudioYusuUserCaseManager::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8("true");

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)NULL);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}


// ----------------------------------------------------------------------------
}

