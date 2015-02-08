

#ifndef ANDROID_AUDIOPOLICYCLIENTLEGACY_H
#define ANDROID_AUDIOPOLICYCLIENTLEGACY_H

#include <system/audio.h>
#include <system/audio_policy.h>
#include <hardware/audio_policy.h>

#include <hardware_legacy/AudioSystemLegacy.h>
#include <hardware_legacy/AudioPolicyInterface.h>

/************************************/
/* FOR BACKWARDS COMPATIBILITY ONLY */
/************************************/
namespace android_audio_legacy {

class AudioPolicyCompatClient : public AudioPolicyClientInterface {
public:
    AudioPolicyCompatClient(struct audio_policy_service_ops *serviceOps,
                            void *service) :
            mServiceOps(serviceOps) , mService(service) {}

    virtual audio_io_handle_t openOutput(uint32_t *pDevices,
                                         uint32_t *pSamplingRate,
                                         uint32_t *pFormat,
                                         uint32_t *pChannels,
                                         uint32_t *pLatencyMs,
                                         AudioSystem::output_flags flags);
    virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1,
                                                  audio_io_handle_t output2);
    virtual status_t closeOutput(audio_io_handle_t output);
    virtual status_t suspendOutput(audio_io_handle_t output);
    virtual status_t restoreOutput(audio_io_handle_t output);
    virtual audio_io_handle_t openInput(uint32_t *pDevices,
                                        uint32_t *pSamplingRate,
                                        uint32_t *pFormat,
                                        uint32_t *pChannels,
                                        uint32_t acoustics);
    virtual status_t closeInput(audio_io_handle_t input);
    virtual status_t setStreamOutput(AudioSystem::stream_type stream, audio_io_handle_t output);
    virtual status_t moveEffects(int session,
                                 audio_io_handle_t srcOutput,
                                 audio_io_handle_t dstOutput);

    virtual String8 getParameters(audio_io_handle_t ioHandle, const String8& keys);
    virtual void setParameters(audio_io_handle_t ioHandle,
                               const String8& keyValuePairs,
                               int delayMs = 0);
    virtual status_t setStreamVolume(AudioSystem::stream_type stream,
                                     float volume,
                                     audio_io_handle_t output,
                                     int delayMs = 0);
    virtual status_t startTone(ToneGenerator::tone_type tone, AudioSystem::stream_type stream);
    virtual status_t stopTone();
    virtual status_t setVoiceVolume(float volume, int delayMs = 0);

private:
    struct audio_policy_service_ops* mServiceOps;
    void*                            mService;
};

}; // namespace android_audio_legacy

#endif // ANDROID_AUDIOPOLICYCLIENTLEGACY_H
