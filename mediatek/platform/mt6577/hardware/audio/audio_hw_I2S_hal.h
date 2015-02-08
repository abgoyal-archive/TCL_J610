

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <pthread.h>
#include <utils/threads.h>
#include <AudioI2S.h>

extern "C"
{

namespace android{

// wrapper for AudioI2S
AudioI2S* I2S_wp_getInstance();
void I2S_wp_Free_I2S_Instance();
int32_t I2S_wp_Audio_GetI2SCapability();

int32_t I2S_wp_Audio_GetState(AudioI2S* Handle);
int32_t I2S_wp_Audio_GetAudioHandle(AudioI2S* Handle);
uint32_t I2S_wp_Audio_samplerate(AudioI2S* Handle);
int32_t I2S_wp_Audio_GetSourceType(AudioI2S* Handle);
uint32_t I2S_wp_Audio_GetReadBufferSize(AudioI2S* Handle);

I2SClient* I2S_wp_Audio_GetI2SClient(AudioI2S* Handle,int index);
bool I2S_wp_Audio_FindFreeClient(AudioI2S* Handle,int32_t* ClientIndex);
bool I2S_wp_Audio_FindValidClient(AudioI2S* Handle,uint32_t Identity);
uint32_t I2S_wp_Audio_open(AudioI2S* Handle);
bool I2S_wp_Audio_close(AudioI2S* Handle,uint32_t Identity);

// weh nstart with I2STYPE ,should call set and then start
bool I2S_wp_Audio_start(AudioI2S* Handle,uint32_t Identity ,I2STYPE Type,uint32_t SampleRate);
bool I2S_wp_Audio_stop(AudioI2S* Handle,uint32_t Identity ,I2STYPE Type);
unsigned int I2S_wp_Audio_read(AudioI2S* Handle,uint32_t Identity,void* buffer, uint32_t buffersize);
bool I2S_wp_Audio_set(AudioI2S* Handle,uint32_t TYPE, int32_t samplerate);

   // state of I2S
bool I2S_wp_Audio_StateInIdle(AudioI2S* Handle);
bool I2S_wp_Audio_StateInI2S0InputStart(AudioI2S* Handle);
bool I2S_wp_Audio_StateInI2S0OutputStart(AudioI2S* Handle);
bool I2S_wp_Audio_StateInI2S1OutputStart(AudioI2S* Handle);
bool I2S_wp_Audio_StateChecking(AudioI2S* Handle,uint32_t state);

}

}

