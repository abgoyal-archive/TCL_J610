

#ifndef ANDROID_AUDIO_USER_CASE_INTERFACE_H
#define ANDROID_AUDIO_USER_CASE_INTERFACE_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <AudioYusuStreamHandler.h>
#include <media/AudioSystem.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuDef.h"
#include "AudioIoctl.h"

namespace android {

// ----------------------------------------------------------------------------
class AudioUserCaseManagerInterFace {
public:
         virtual      ~AudioUserCaseManagerInterFace(){};
         virtual status_t    initCheck() =0;

         // tell UCM which mode is, when system boot up , default for normal mode/
         virtual status_t  SetPhoneMode(int32 mode)=0;

         // tell UCM which device is attached and remove.
         virtual status_t  SetInputDevice(uint32 device)=0;
         virtual status_t  SetOutputDevice(uint32 device)=0;

         // init stream index and volume
         virtual status_t InitStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, int indexMin , int indexMax)=0;

         // tell UCM which stream start or stop.
         virtual status_t StreamStart(android_audio_legacy::AudioSystem::stream_type stream)=0;
         virtual status_t StreamStop(android_audio_legacy::AudioSystem::stream_type stream)=0;


         // tell UCM when Record start / stop
         virtual status_t RecordStreamStart()=0;
         virtual status_t RecordStreamStop()=0;

         // tell UCM which stream start or stop.
         virtual status_t SetStreamLevel(android_audio_legacy::AudioSystem::stream_type stream, uint32 level)=0;

         // set and get parameter
         virtual status_t setParameters(const String8& keyValuePairs)=0;
         virtual String8 getParameters(const String8& keys)=0;

};

// ----------------------------------------------------------------------------
}; // namespace android

#endif;
