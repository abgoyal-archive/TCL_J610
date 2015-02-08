

#ifndef ANDROID_AUDIO_HW_HAL_I2S_H
#define ANDROID_AUDIO_HW_HAL_I2S_H

//#include "audio_hw_I2S_hal.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <utils/threads.h>
#include <AudioI2S.h>
#include <audio_hw_I2S_hal.h>

namespace android{


// wrapper for android::AudioI2S
AudioI2S* I2S_wp_getInstance()
{
    return AudioI2S::getInstance ();
}

void I2S_wp_Free_I2S_Instance()
{
    return AudioI2S::freeInstance ();
}

int32_t I2S_wp_Audio_GetI2SCapability()
{
    return AudioI2S::GetI2SCapability();
}

int I2S_wp_Audio_GetState(AudioI2S* handle)
{
    return handle->GetState ();
}

int32_t I2S_wp_Audio_GetAudioHandle(AudioI2S* handle)
{
    return handle->GetAudioHandle ();
}

uint32_t I2S_wp_Audio_samplerate(AudioI2S* handle)
{
    return handle->samplerate ();
}

int32_t I2S_wp_Audio_GetSourceType(AudioI2S* handle)
{
    return handle->GetSourceType ();
}

uint32_t I2S_wp_Audio_GetReadBufferSize(AudioI2S* handle)
{
    return handle->GetReadBufferSize ();
}

I2SClient* I2S_wp_Audio_GetI2SClient(AudioI2S* handle,int index)
{
    return handle->GetI2SClient (index);
}

bool I2S_wp_Audio_FindFreeClient(AudioI2S* handle,int32_t* ClientIndex)
{
    return handle->FindFreeClient (ClientIndex);
}

bool I2S_wp_Audio_FindValidClient(AudioI2S* handle,uint32_t Identity)
{
    return handle->FindValidClient (Identity);
}

uint32_t I2S_wp_Audio_open(AudioI2S* handle)
{
    return handle->open();
}

bool I2S_wp_Audio_close(AudioI2S* handle,uint32_t Identity)
{
    return handle->close (Identity);
}

// weh nstart with I2STYPE ,should call set and then start
bool I2S_wp_Audio_start(AudioI2S* handle,uint32_t Identity ,I2STYPE Type,uint32_t SampleRate)
{
     return handle->start(Identity,Type,SampleRate);
}

bool I2S_wp_Audio_stop(AudioI2S* handle,uint32_t Identity ,I2STYPE Type)
{
    return handle->stop(Identity,Type);
}

unsigned int I2S_wp_Audio_read(AudioI2S* handle,uint32_t Identity,void* buffer, uint32_t buffersize)
{
    return handle->read(Identity,buffer,buffersize);
}

bool I2S_wp_Audio_set(AudioI2S* handle,uint32_t TYPE, int32_t samplerate)
{
    return handle->set(TYPE, samplerate);
}

// state of I2S
bool I2S_wp_Audio_StateInIdle(AudioI2S* handle)
{
    return handle->StateInIdle();
}

bool I2S_wp_Audio_StateInI2S0InputStart(AudioI2S* handle)
{
    return handle->StateInI2S0InputStart();
}

bool I2S_wp_Audio_StateInI2S0OutputStart(AudioI2S* handle)
{
    return handle->StateInI2S0OutputStart();
}

bool I2S_wp_Audio_StateInI2S1OutputStart(AudioI2S* handle )
{
    return handle->StateInI2S1OutputStart();
}

bool I2S_wp_Audio_StateChecking(AudioI2S* handle,uint32_t state)
{
    return handle->StateChecking(state);
}

}

#endif

