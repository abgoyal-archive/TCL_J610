

#define LOG_TAG "AudioYusuPolicyManager"
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#ifndef ANDROID_DEFAULT_CODE
#include "AudioYusuPolicyManager.h"
#include <media/mediarecorder.h>
#include <AudioYusuDef.h>
#include "AudioYusuVolumeController.h"
#include "audio_custom_exp.h"
#else
#include <hardware_legacy/AudioPolicyManagerBase.h>
#include <hardware/audio_effect.h>
#endif //#ifndef ANDROID_DEFAULT_CODE

//PR-277961-Zhengyong.Wang-001 Modified begin
#include <cutils/properties.h>
//PR-277961-Zhengyong.Wang-001 Modified end
#include <math.h>

#ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
#include <AudioCustParam.h>
#endif

#ifndef ANDROID_DEFAULT_CODE
#include "AudioIoctl.h"

#define MUSIC_WAIT_TIME (1000*200)

#ifndef BOOT_ANIMATION_VOLUME
#define  BOOT_ANIMATION_VOLUME (0.25)
#endif

#ifdef ENABLE_LOG_AUDIOPOLICYANAGER
#undef LOGV
#define LOGV(...) LOGW(__VA_ARGS__)
#endif
#endif //#ifndef ANDROID_DEFAULT_CODE

namespace android_audio_legacy
{

static const float volume_Mapping_Step = 256.0f;
static const float Policy_Voume_Max = 255.0f;
static const int Custom_Voume_Step = 6.0;

#ifndef ANDROID_DEFAULT_CODE

static const uint16 GainMap[] =
{
	0xf1ad, // 0: -0.50 dB
	0xe429, // 1: -1.00 dB
	0xd765, // 2: -1.50 dB
	0xcb59, // 3: -2.00 dB
	0xbff9, // 4: -2.50 dB
	0xb53b, // 5: -3.00 dB
	0xab18, // 6: -3.50 dB
	0xa186, // 7: -4.00 dB
	0x987d, // 8: -4.50 dB
	0x8ff5, // 9: -5.00 dB
	0x87e8, // 10: -5.50 dB
	0x804d, // 11: -6.00 dB
	0x7920, // 12: -6.50 dB
	0x7259, // 13: -7.00 dB
	0x6bf4, // 14: -7.50 dB
	0x65ea, // 15: -8.00 dB
	0x6036, // 16: -8.50 dB
	0x5ad5, // 17: -9.00 dB
	0x55c0, // 18: -9.50 dB
	0x50f4, // 19: -10.00 dB
	0x4c6d, // 20: -10.50 dB
	0x4826, // 21: -11.00 dB
	0x441d, // 22: -11.50 dB
	0x404d, // 23: -12.00 dB
	0x3cb5, // 24: -12.50 dB
	0x394f, // 25: -13.00 dB
	0x361a, // 26: -13.50 dB
	0x3314, // 27: -14.00 dB
	0x3038, // 28: -14.50 dB
	0x2d86, // 29: -15.00 dB
	0x2afa, // 30: -15.50 dB
	0x2892, // 31: -16.00 dB
	0x264d, // 32: -16.50 dB
	0x2429, // 33: -17.00 dB
	0x2223, // 34: -17.50 dB
	0x203a, // 35: -18.00 dB
	0x1e6c, // 36: -18.50 dB
	0x1cb9, // 37: -19.00 dB
	0x1b1d, // 38: -19.50 dB
	0x1999, // 39: -20.00 dB
	0x182a, // 40: -20.50 dB
	0x16d0, // 41: -21.00 dB
	0x158a, // 42: -21.50 dB
	0x1455, // 43: -22.00 dB
	0x1332, // 44: -22.50 dB
	0x121f, // 45: -23.00 dB
	0x111c, // 46: -23.50 dB
	0x1027, // 47: -24.00 dB
	0x0f3f, // 48: -24.50 dB
	0x0e65, // 49: -25.00 dB
	0x0d97, // 50: -25.50 dB
	0x0cd4, // 51: -26.00 dB
	0x0c1c, // 52: -26.50 dB
	0x0b6f, // 53: -27.00 dB
	0x0acb, // 54: -27.50 dB
	0x0a31, // 55: -28.00 dB
	0x099f, // 56: -28.50 dB
	0x0915, // 57: -29.00 dB
	0x0893, // 58: -29.50 dB
	0x0818, // 59: -30.00 dB
	0x07a4, // 60: -30.50 dB
	0x0737, // 61: -31.00 dB
	0x06cf, // 62: -31.50 dB
	0x066e, // 63: -32.00 dB
	0x0612, // 64: -32.50 dB
	0x05bb, // 65: -33.00 dB
	0x0569, // 66: -33.50 dB
	0x051b, // 67: -34.00 dB
	0x04d2, // 68: -34.50 dB
	0x048d, // 69: -35.00 dB
	0x044c, // 70: -35.50 dB
	0x040e, // 71: -36.00 dB
	0x03d4, // 72: -36.50 dB
	0x039d, // 73: -37.00 dB
	0x0369, // 74: -37.50 dB
	0x0339, // 75: -38.00 dB
	0x030a, // 76: -38.50 dB
	0x02df, // 77: -39.00 dB
	0x02b6, // 78: -39.50 dB
	0x028f, // 79: -40.00 dB
	0x026a, // 80: -40.50 dB
	0x0248, // 81: -41.00 dB
	0x0227, // 82: -41.50 dB
	0x0208, // 83: -42.00 dB
	0x01eb, // 84: -42.50 dB
	0x01cf, // 85: -43.00 dB
	0x01b6, // 86: -43.50 dB
	0x019d, // 87: -44.00 dB
	0x0186, // 88: -44.50 dB
	0x0170, // 89: -45.00 dB
	0x015b, // 90: -45.50 dB
	0x0148, // 91: -46.00 dB
	0x0136, // 92: -46.50 dB
	0x0124, // 93: -47.00 dB
	0x0114, // 94: -47.50 dB
	0x0104, // 95: -48.00 dB
	0x00f6, // 96: -48.50 dB
	0x00e8, // 97: -49.00 dB
	0x00db, // 98: -49.50 dB
	0x00cf, // 99: -50.00 dB
	0x00c3, // 100: -50.50 dB
	0x00b8, // 101: -51.00 dB
	0x00ae, // 102: -51.50 dB
	0x00a4, // 103: -52.00 dB
	0x009b, // 104: -52.50 dB
	0x0092, // 105: -53.00 dB
	0x008a, // 106: -53.50 dB
	0x0082, // 107: -54.00 dB
	0x007b, // 108: -54.50 dB
	0x0074, // 109: -55.00 dB
	0x006e, // 110: -55.50 dB
	0x0067, // 111: -56.00 dB
	0x0062, // 112: -56.50 dB
	0x005c, // 113: -57.00 dB
	0x0057, // 114: -57.50 dB
	0x0052, // 115: -58.00 dB
	0x004d, // 116: -58.50 dB
	0x0049, // 117: -59.00 dB
	0x0045, // 118: -59.50 dB
	0x0041, // 119: -60.00 dB
	0x003d, // 120: -60.50 dB
	0x003a, // 121: -61.00 dB
	0x0037, // 122: -61.50 dB
	0x0034, // 123: -62.00 dB
	0x0031, // 124: -62.50 dB
	0x002e, // 125: -63.00 dB
	0x002b, // 126: -63.50 dB
	0x0029, // 127: -64.00 dB
	0x0027, // 128: -64.50 dB
	0x0024, // 129: -65.00 dB
	0x0022, // 130: -65.50 dB
	0x0020, // 131: -66.00 dB
	0x001f, // 132: -66.50 dB
	0x001d, // 133: -67.00 dB
	0x001b, // 134: -67.50 dB
	0x001a, // 135: -68.00 dB
	0x0018, // 136: -68.50 dB
	0x0017, // 137: -69.00 dB
	0x0015, // 138: -69.50 dB
	0x0014, // 139: -70.00 dB
	0x0013, // 140: -70.50 dB
	0x0012, // 141: -71.00 dB
	0x0011, // 142: -71.50 dB
	0x0010, // 143: -72.00 dB
	0x000f, // 144: -72.50 dB
	0x000e, // 145: -73.00 dB
	0x000d, // 146: -73.50 dB
	0x000d, // 147: -74.00 dB
	0x000c, // 148: -74.50 dB
	0x000b, // 149: -75.00 dB
	0x000b, // 150: -75.50 dB
	0x000a, // 151: -76.00 dB
	0x0009, // 152: -76.50 dB
	0x0009, // 153: -77.00 dB
	0x0008, // 154: -77.50 dB
	0x0008, // 155: -78.00 dB
	0x0007, // 156: -78.50 dB
	0x0007, // 157: -79.00 dB
	0x0006, // 158: -79.50 dB
	0x0006, // 159: -80.00 dB
	0x0006, // 160: -80.50 dB
	0x0005, // 161: -81.00 dB
	0x0005, // 162: -81.50 dB
	0x0005, // 163: -82.00 dB
	0x0004, // 164: -82.50 dB
	0x0004, // 165: -83.00 dB
	0x0004, // 166: -83.50 dB
	0x0004, // 167: -84.00 dB
	0x0003, // 168: -84.50 dB
	0x0003, // 169: -85.00 dB
	0x0003, // 170: -85.50 dB
	0x0003, // 171: -86.00 dB
	0x0003, // 172: -86.50 dB
	0x0002, // 173: -87.00 dB
	0x0002, // 174: -87.50 dB
	0x0002, // 175: -88.00 dB
	0x0002, // 176: -88.50 dB
	0x0002, // 177: -89.00 dB
	0x0002, // 178: -89.50 dB
	0x0002, // 179: -90.00 dB
	0x0001, // 180: -90.50 dB
	0x0001, // 181: -91.00 dB
	0x0001, // 182: -91.50 dB
	0x0001, // 183: -92.00 dB
	0x0001, // 184: -92.50 dB
	0x0001, // 185: -93.00 dB
	0x0001, // 186: -93.50 dB
	0x0001, // 187: -94.00 dB
	0x0001, // 188: -94.50 dB
	0x0001, // 189: -95.00 dB
	0x0001, // 190: -95.50 dB
	0x0001, // 191: -96.00 dB
	0x0000, // 192: -96.50 dB
	0x0000, // 193: -97.00 dB
	0x0000, // 194: -97.50 dB
	0x0000, // 195: -98.00 dB
	0x0000, // 196: -98.50 dB
	0x0000, // 197: -99.00 dB
	0x0000, // 198: -99.50 dB
	0x0000, // 199: -100.00 dB
};

AUDIO_VOLUME_CUSTOM_STRUCT AudioYusuPolicyManager::Audio_Custom_Volume;

#ifdef WITH_A2DP
//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
void AudioYusuPolicyManager::setA2dpEnabled(AudioSystem::device_connection_state state)
{
    LOGV("setA2dpEnabled state == %d",state);
    if(state == AudioSystem::DEVICE_STATE_UNAVAILABLE)
    {
        for (int StreamType=0 ;StreamType< AudioSystem::NUM_STREAM_TYPES ;StreamType++)
        {
            if(routedToA2dpOutput(StreamType) == true)
                mpClientInterface->setStreamOutput ((AudioSystem::stream_type)StreamType , mHardwareOutput );
        }

    }
    else if(state == AudioSystem::DEVICE_STATE_AVAILABLE)
    {
        for (int StreamType=0 ;StreamType< AudioSystem::NUM_STREAM_TYPES ;StreamType++)
        {
            if(routedToA2dpOutput(StreamType) == true)
                mpClientInterface->setStreamOutput ((AudioSystem::stream_type)StreamType , mA2dpOutput );
        }
    }
}

bool AudioYusuPolicyManager::routedToA2dpOutput(int StreamType)
{
    switch(StreamType) {
    case AudioSystem::MUSIC:
    case AudioSystem::VOICE_CALL:
    case AudioSystem::BLUETOOTH_SCO:
    case AudioSystem::SYSTEM:
        return true;
    default:
        return false;
    }
}
#endif
//----------------------------------------------------------------------------
// check Function
//----------------------------------------------------------------------------
void AudioYusuPolicyManager::CheckMaxMinValue(int min, int max)
{
    if(min > max){
        LOGE("CheckMaxMinValue min = %d > max = %d",min,max);
    }
}

static int mapping_vol(float &vol, float unitstep)
{
    if(vol < unitstep){
        return 1;
    }
    if(vol < unitstep*2){
        vol -= unitstep;
        return 2;
    }
    else if(vol < unitstep*3){
        vol -= unitstep*2;
        return 3;
    }
    else if(vol < unitstep*4){
        vol -= unitstep*3;
        return 4;
    }
    else if(vol < unitstep*5){
        vol -= unitstep*4;
        return 5;
    }
    else if(vol < unitstep*6){
        vol -= unitstep*5;
        return 6;
    }
    else if(vol < unitstep*7){
        vol -= unitstep*6;
        return 7;
    }
    else{
        LOGW("vole = %f unitstep = %f",vol,unitstep);
        return 0;
    }
}


#ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
float AudioYusuPolicyManager::MapGaintableVoltoCustomVol(AUDIO_GAIN_CONTROL_STRUCT array[], int volmin, int volmax, int &vol, int VolumeMaxLevel)
{
    float volume =0.0;
    CheckMaxMinValue(volmin,volmax);
    LOGD("MapGaintableVoltoCustomVol volmin = %d volmax = %d",volmin,volmax);
     if(vol == 100){
         vol = volmax;
         //LOGV("MapVoltoCustomVol vol = volmax = %d",vol);
     }
     // map volume value to custom volume
     else{
         float unitstep = 100.0 /(VolumeMaxLevel-1);           // percentage of each step.
         int Index = (float)vol / unitstep;                            // find the current volume belong to which level.
         float Remind = (float)vol - (unitstep *Index);       //remidn value to use linear interpolate.
         vol = array[Index].u8Gain_digital +  (array[Index+1].u8Gain_digital -array[Index].u8Gain_digital) * (Remind/ unitstep) ;
         LOGD("MapGaintableVoltoCustomVol unitstep = %f Index = %d Remind = %f vol = %d",unitstep,Index,Remind,vol);
     }

     // because volume step may not be full range , so need to comare with 0~255
     volume = (float)(vol *100.0)/255.0 +0.5;
     vol = (float)(vol *100.0)/255.0 +0.5;
     LOGD("MapGaintableVoltoCustomVol volume = %f vol = %d",volume,vol);
     return volume;
}
#endif


// this function will map vol 0~100 , base on customvolume amp to 0~255 , and do linear calculation to set mastervolume
float AudioYusuPolicyManager::MapVoltoCustomVol(unsigned char array[], int volmin, int volmax,float &vol , int stream)
{
    float volume =0.0;
    StreamDescriptor &streamDesc = mStreams[stream];
    CheckMaxMinValue(volmin,volmax);
    if (vol == 0){
        volume = vol;
        return 0;
    }
    // map volume value to custom volume
    else{
        float unitstep = volume_Mapping_Step/(Custom_Voume_Step);
        if(vol <= streamDesc.mIndexRange){
            volume = array[0];
            vol = volume;
            return volume;
        }
        else{
            vol -= streamDesc.mIndexRange;
            vol *= volume_Mapping_Step/(volume_Mapping_Step-streamDesc.mIndexRange);
        }
        int Index = mapping_vol(vol, unitstep);
        mapping_vol(vol, unitstep);
        float Remind = (1.0 - (float)vol/unitstep) ;
        volume = (array[Index]  - (array[Index] - array[Index-1]) * Remind);
        //LOGD("MapVoltoCustomVol unitstep = %f Index = %d Remind = %f vol = %f mIndexRange = %f",unitstep,Index,Remind,vol,streamDesc.mIndexRange);
    }

    if( volume > 252.0){
        volume = volume_Mapping_Step;
    }
    else if( volume <= array[0]){
        volume = array[0];
    }

     vol = volume;
     //LOGD("MapVoltoCustomVol volume = %f vol = %f",volume,vol);
     return volume;
}

// this function will map vol 0~100 , base on customvolume amp to 0~255 , and do linear calculation to set mastervolume
float AudioYusuPolicyManager::MapVoiceVoltoCustomVol(unsigned char array[], int volmin, int volmax, float &vol)
{
    vol = (int)vol;
    float volume =0.0;
    StreamDescriptor &streamDesc = mStreams[AudioSystem::VOICE_CALL];
    //LOGD("MapVoiceVoltoCustomVol vol = %f",vol);
    CheckMaxMinValue(volmin,volmax);
    if (vol == 0){
        volume = array[0];
    }
    else
    {
         if(vol >= volume_Mapping_Step){
             volume = array[6];
             //LOGD("Volume Max  volume = %f vol = %f",volume,vol);
         }
         else{
             double unitstep = volume_Mapping_Step /(6);  // per 42  ==> 1 step
             int Index = mapping_vol(vol, unitstep);
             // boundary for array
             if(Index >= 6){
                 Index = 6;
             }
             float Remind = (1.0 - (float)vol/unitstep) ;
             volume = (array[Index]  - (array[Index] - array[Index- 1]) * Remind);
             //LOGD("MapVoiceVoltoCustomVol volume = %f vol = %f Index = %d Remind = %f",volume,vol,Index,Remind);
         }
     }

     if( volume > VOICE_VOLUME_MAX){
         volume = VOICE_VOLUME_MAX;
     }
     else if( volume <= array[0]){
         volume = array[0];
     }


     vol = volume;
     float degradeDb = (VOICE_VOLUME_MAX-vol)/VOICE_ONEDB_STEP;
     vol = volume_Mapping_Step - (degradeDb*4);
     LOGD("MapVoltoCustomVol volume = %f vol = %f degradeDb = %f",volume,vol,degradeDb);
     return volume;
}


int AudioYusuPolicyManager::FindCustomVolumeIndex(unsigned char array[], int volInt)
{
    int volumeindex =0;
    for(int i=0;i <Custom_Voume_Step ; i++){
        LOGV("FindCustomVolumeIndex array[%d] = %d",i,array[i]);
    }
    for(volumeindex =Custom_Voume_Step-1 ; volumeindex >0 ; volumeindex--){
        if(array[volumeindex] < volInt){
            break;
        }
    }
    return volumeindex;
}

#endif //#ifndef ANDROID_DEFAULT_CODE
// ----------------------------------------------------------------------------
// AudioPolicyInterface implementation
// ----------------------------------------------------------------------------


status_t AudioYusuPolicyManager::setDeviceConnectionState(AudioSystem::audio_devices device,
                                                  AudioSystem::device_connection_state state,
                                                  const char *device_address)
{

    LOGD("setDeviceConnectionState() device: %x, state %d, address %s", device, state, device_address);

    // connect/disconnect only 1 device at a time
    if (AudioSystem::popCount(device) != 1) return BAD_VALUE;

    if (strlen(device_address) >= MAX_DEVICE_ADDRESS_LEN) {
        LOGE("setDeviceConnectionState() invalid address: %s", device_address);
        return BAD_VALUE;
    }

    // handle output devices
    if (AudioSystem::isOutputDevice(device)) {
#ifndef ANDROID_DEFAULT_CODE
        AudioParameter outputCmd = AudioParameter();
#endif

#ifndef WITH_A2DP
        if (AudioSystem::isA2dpDevice(device)) {
            LOGE("setDeviceConnectionState() invalid device: %x", device);
            return BAD_VALUE;
        }
#endif
        LOGV("setDeviceConnectionState() connecting device %x", device);

        switch (state)
        {
        // handle output device connection
        case AudioSystem::DEVICE_STATE_AVAILABLE:
            if (mAvailableOutputDevices & device) {
                LOGW("setDeviceConnectionState() device already connected: %x", device);
                return INVALID_OPERATION;
            }
            // register new device as available
            mAvailableOutputDevices |= device;
#ifndef ANDROID_DEFAULT_CODE
           // LOGD("setDeviceConnectionState mAvailableOutputDevices = %x",mAvailableOutputDevices);
            // handle outputDevice Route setParameters with 0 means set ot AudioHardware
           // outputCmd.addInt(String8(keyAddOutputDevice),device);
           // mpClientInterface->setParameters(0, outputCmd.toString());
#endif

#ifdef WITH_A2DP
            // handle A2DP device connection
            if (AudioSystem::isA2dpDevice(device)) {
                status_t status = handleA2dpConnection(device, device_address);
                if (status != NO_ERROR) {
                    mAvailableOutputDevices &= ~device;
                    return status;
                }
            } else
#endif
            {
                if (AudioSystem::isBluetoothScoDevice(device)) {
                    LOGV("setDeviceConnectionState() BT SCO  device, address %s", device_address);
                    // keep track of SCO device address
                    mScoDeviceAddress = String8(device_address, MAX_DEVICE_ADDRESS_LEN);
                }
            }
            break;
        // handle output device disconnection
        case AudioSystem::DEVICE_STATE_UNAVAILABLE: {
            if (!(mAvailableOutputDevices & device)) {
                LOGW("setDeviceConnectionState() device not connected: %x", device);
                return INVALID_OPERATION;
            }

            // remove device from available output devices
            mAvailableOutputDevices &= ~device;
#ifndef ANDROID_DEFAULT_CODE
            LOGD("setDeviceConnectionState mAvailableOutputDevices = %x",mAvailableOutputDevices);
            // handle outputDevice Route setParameters with 0 means set of AudioYusuPolicyManager
            outputCmd.addInt(String8(keyRemoveOutputDevice),device);
            mpClientInterface->setParameters(0, outputCmd.toString());
#endif

#ifdef WITH_A2DP
            // handle A2DP device disconnection
            if (AudioSystem::isA2dpDevice(device)) {
                status_t status = handleA2dpDisconnection(device, device_address);
                if (status != NO_ERROR) {
                    mAvailableOutputDevices |= device;
                    return status;
                }
            } else
#endif
            {
                if (AudioSystem::isBluetoothScoDevice(device)) {
                    mScoDeviceAddress = "";
                }
            }
            } break;

        default:
            LOGE("setDeviceConnectionState() invalid state: %x", state);
            return BAD_VALUE;
        }

        // request routing change if necessary
        uint32_t newDevice = getNewDevice(mHardwareOutput, false);
#ifdef WITH_A2DP
        checkA2dpSuspend();
        checkOutputForAllStrategies();
        // A2DP outputs must be closed after checkOutputForAllStrategies() is executed
        if (state == AudioSystem::DEVICE_STATE_UNAVAILABLE && AudioSystem::isA2dpDevice(device)) {
            closeA2dpOutputs();
        }
#endif
        updateDeviceForStrategy();
        setOutputDevice(mHardwareOutput, newDevice);
#ifndef ANDROID_DEFAULT_CODE
        if(state == AudioSystem::DEVICE_STATE_AVAILABLE)
        {
            LOGD("setDeviceConnectionState add device = %x",device);
            // handle outputDevice Route setParameters with 0 means set ot AudioHardware
            outputCmd.addInt(String8(keyAddOutputDevice),device);
            mpClientInterface->setParameters(0, outputCmd.toString());
        }
#endif
        if (device == AudioSystem::DEVICE_OUT_WIRED_HEADSET) {
            device = AudioSystem::DEVICE_IN_WIRED_HEADSET;
        } else if (device == AudioSystem::DEVICE_OUT_BLUETOOTH_SCO ||
                   device == AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
                   device == AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
            device = AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        } else {
            return NO_ERROR;
        }
    }
    // handle input devices
    if (AudioSystem::isInputDevice(device)) {
#ifndef ANDROID_DEFAULT_CODE
        AudioParameter outputCmd = AudioParameter();
#endif
        switch (state)
        {
        // handle input device connection
        case AudioSystem::DEVICE_STATE_AVAILABLE: {
            if (mAvailableInputDevices & device) {
                LOGW("setDeviceConnectionState() device already connected: %d", device);
                return INVALID_OPERATION;
            }
            mAvailableInputDevices |= device;
#ifndef ANDROID_DEFAULT_CODE
            LOGD("setDeviceConnectionState mAvailableInputDevices = %x",mAvailableInputDevices);
            // handle InputDevice Route setParameters with 0 means set ot AudioHardware
            outputCmd.addInt(String8(keyAddIntputDevice),device);
            mpClientInterface->setParameters(0, outputCmd.toString());
#endif
            }
            break;

        // handle input device disconnection
        case AudioSystem::DEVICE_STATE_UNAVAILABLE: {
            if (!(mAvailableInputDevices & device)) {
                LOGW("setDeviceConnectionState() device not connected: %d", device);
                return INVALID_OPERATION;
            }
            mAvailableInputDevices &= ~device;
#ifndef ANDROID_DEFAULT_CODE
            LOGD("setDeviceConnectionState mAvailableInputDevices = %x",mAvailableInputDevices);
            // handle InputDevice Route setParameters with 0 means set ot AudioHardware
            outputCmd.addInt(String8(keyRemoveIntputDevice),device);
            mpClientInterface->setParameters(0, outputCmd.toString());
#endif
            } break;

        default:
            LOGE("setDeviceConnectionState() invalid state: %x", state);
            return BAD_VALUE;
        }

        audio_io_handle_t activeInput = getActiveInput();
        if (activeInput != 0) {
            AudioInputDescriptor *inputDesc = mInputs.valueFor(activeInput);
            uint32_t newDevice = getDeviceForInputSource(inputDesc->mInputSource);
            if (newDevice != inputDesc->mDevice) {
                LOGV("setDeviceConnectionState() changing device from %x to %x for input %d",
                        inputDesc->mDevice, newDevice, activeInput);
                inputDesc->mDevice = newDevice;
                AudioParameter param = AudioParameter();
                param.addInt(String8(AudioParameter::keyRouting), (int)newDevice);
                mpClientInterface->setParameters(activeInput, param.toString());
            }
        }

        return NO_ERROR;
    }

    LOGW("setDeviceConnectionState() invalid device: %x", device);
    return BAD_VALUE;
}

AudioSystem::device_connection_state AudioYusuPolicyManager::getDeviceConnectionState(AudioSystem::audio_devices device,
                                                  const char *device_address)
{
    AudioSystem::device_connection_state state = AudioSystem::DEVICE_STATE_UNAVAILABLE;
    String8 address = String8(device_address);
    if (AudioSystem::isOutputDevice(device)) {
        if (device & mAvailableOutputDevices) {
#ifdef WITH_A2DP
            if (AudioSystem::isA2dpDevice(device) &&
                address != "" && mA2dpDeviceAddress != address) {
                return state;
            }
#endif
            if (AudioSystem::isBluetoothScoDevice(device) &&
                address != "" && mScoDeviceAddress != address) {
                return state;
            }
            state = AudioSystem::DEVICE_STATE_AVAILABLE;
        }
    } else if (AudioSystem::isInputDevice(device)) {
        if (device & mAvailableInputDevices) {
            state = AudioSystem::DEVICE_STATE_AVAILABLE;
        }
    }

    return state;
}

void AudioYusuPolicyManager::setPhoneState(int state)
{
    LOGD("setPhoneState() state %d", state);
    uint32_t newDevice = 0;
    if (state < 0 || state >= AudioSystem::NUM_MODES) {
        LOGW("setPhoneState() invalid state %d", state);
        return;
    }

    if (state == mPhoneState ) {
        LOGW("setPhoneState() setting same state %d", state);
        return;
    }

    // if leaving call state, handle special case of active streams
    // pertaining to sonification strategy see handleIncallSonification()
    if (isInCall()) {
        LOGD("setPhoneState() in call state management: new state is %d", state);
        for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
            handleIncallSonification(stream, false, true);
        }
    }

    // store previous phone state for management of sonification strategy below
    int oldState = mPhoneState;
    mPhoneState = state;
    LOGV("AudioYusuPolicyManager::setPhoneState oldState = %d mPhoneState = %d",oldState,mPhoneState);
    bool force = false;

    // are we entering or starting a call
    if (!isStateInCall(oldState) && isStateInCall(state)) {
        LOGV("  Entering call in setPhoneState()");
        // force routing command to audio hardware when starting a call
        // even if no device change is needed
        force = true;
    } else if (isStateInCall(oldState) && !isStateInCall(state)) {
        LOGV("  Exiting call in setPhoneState()");
        // force routing command to audio hardware when exiting a call
        // even if no device change is needed
        force = true;
    } else if (isStateInCall(state) && (state != oldState)) {
        LOGV("  Switching between telephony and VoIP in setPhoneState()");
        // force routing command to audio hardware when switching between telephony and VoIP
        // even if no device change is needed
        force = true;
    }

    // check for device and output changes triggered by new phone state
    newDevice = getNewDevice(mHardwareOutput, false);
#ifdef WITH_A2DP
    checkA2dpSuspend();
    checkOutputForAllStrategies();
#endif
    updateDeviceForStrategy();

    AudioOutputDescriptor *hwOutputDesc = mOutputs.valueFor(mHardwareOutput);

    // force routing command to audio hardware when ending call
    // even if no device change is needed
    if (isStateInCall(oldState) && newDevice == 0) {
        newDevice = hwOutputDesc->device();
    }

    // when changing from ring tone to in call mode, mute the ringing tone
    // immediately and delay the route change to avoid sending the ring tone
    // tail into the earpiece or headset.
    int delayMs = 0;
    if (isStateInCall(state) && oldState == AudioSystem::MODE_RINGTONE) {
        // delay the device change command by twice the output latency to have some margin
        // and be sure that audio buffers not yet affected by the mute are out when
        // we actually apply the route change
        delayMs = hwOutputDesc->mLatency*2;
        setStreamMute(AudioSystem::RING, true, mHardwareOutput);
    }

    // change routing is necessary
    setOutputDevice(mHardwareOutput, newDevice, force, delayMs);

    // if entering in call state, handle special case of active streams
    // pertaining to sonification strategy see handleIncallSonification()
    if (isStateInCall(state)) {
        LOGD("setPhoneState() in call state management: new state is %d", state);
        // unmute the ringing tone after a sufficient delay if it was muted before
        // setting output device above
        if (oldState == AudioSystem::MODE_RINGTONE) {
            setStreamMute(AudioSystem::RING, false, mHardwareOutput, MUTE_TIME_MS);
        }
        for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
            handleIncallSonification(stream, true, true);
        }
    }

    // Flag that ringtone volume must be limited to music volume until we exit MODE_RINGTONE
    if (state == AudioSystem::MODE_RINGTONE &&
        isStreamActive(AudioSystem::MUSIC, SONIFICATION_HEADSET_MUSIC_DELAY)) {
        mLimitRingtoneVolume = true;
    } else {
        mLimitRingtoneVolume = false;
    }
}

void AudioYusuPolicyManager::setRingerMode(uint32_t mode, uint32_t mask)
{
    LOGD("setRingerMode() mode %x, mask %x", mode, mask);

    mRingerMode = mode;
}

void AudioYusuPolicyManager::setForceUse(AudioSystem::force_use usage, AudioSystem::forced_config config)
{
    LOGD("setForceUse() usage %d, config %d, mPhoneState %d", usage, config, mPhoneState);

    bool forceVolumeReeval = false;
    switch(usage) {
    case AudioSystem::FOR_COMMUNICATION:
        if (config != AudioSystem::FORCE_SPEAKER && config != AudioSystem::FORCE_BT_SCO &&
            config != AudioSystem::FORCE_NONE) {
            LOGW("setForceUse() invalid config %d for FOR_COMMUNICATION", config);
            return;
        }
        forceVolumeReeval = true;
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_MEDIA:
        if (config != AudioSystem::FORCE_HEADPHONES && config != AudioSystem::FORCE_BT_A2DP &&
            config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_ANALOG_DOCK &&
            config != AudioSystem::FORCE_DIGITAL_DOCK && config != AudioSystem::FORCE_NONE
#ifndef ANDROID_DEFAULT_CODE
            && config != AudioSystem::FORCE_SPEAKER
#endif
            ) {
            LOGW("setForceUse() invalid config %d for FOR_MEDIA", config);
            return;
        }
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_RECORD:
        if (config != AudioSystem::FORCE_BT_SCO && config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_NONE) {
            LOGW("setForceUse() invalid config %d for FOR_RECORD", config);
            return;
        }
        mForceUse[usage] = config;
        break;
    case AudioSystem::FOR_DOCK:
        if (config != AudioSystem::FORCE_NONE && config != AudioSystem::FORCE_BT_CAR_DOCK &&
            config != AudioSystem::FORCE_BT_DESK_DOCK &&
            config != AudioSystem::FORCE_WIRED_ACCESSORY &&
            config != AudioSystem::FORCE_ANALOG_DOCK &&
            config != AudioSystem::FORCE_DIGITAL_DOCK) {
            LOGW("setForceUse() invalid config %d for FOR_DOCK", config);
        }
        forceVolumeReeval = true;
        mForceUse[usage] = config;
        break;
    default:
        LOGW("setForceUse() invalid usage %d", usage);
        break;
    }

    // check for device and output changes triggered by new phone state
    uint32_t newDevice = getNewDevice(mHardwareOutput, false);
#ifdef WITH_A2DP
    checkA2dpSuspend();
    checkOutputForAllStrategies();
#endif
    updateDeviceForStrategy();
    setOutputDevice(mHardwareOutput, newDevice);
    if (forceVolumeReeval) {
        applyStreamVolumes(mHardwareOutput, newDevice, 0, true);
    }

#ifndef ANDROID_DEFAULT_CODE
    if(AudioSystem::popCount(newDevice) != 2 )
    {
        LOGD("setForceUse newDevice=0x%x",newDevice);
        AudioParameter outputCmd = AudioParameter();
        uint32_t value = (uint32_t)usage<<16 |config;
        if(usage == AudioSystem::FOR_COMMUNICATION){
            outputCmd.addInt(String8(keyAddtForceusePhone),value);
        }
        else{
            outputCmd.addInt(String8(keyAddtForceuseNormal),value);
        }

        if(usage == AudioSystem::FOR_MEDIA&& config ==  AudioSystem::FORCE_NONE){
            mpClientInterface->setParameters(0, outputCmd.toString(),500);
        }
        else{
            mpClientInterface->setParameters(0, outputCmd.toString());
        }
    }
#endif
    audio_io_handle_t activeInput = getActiveInput();
    if (activeInput != 0) {
        AudioInputDescriptor *inputDesc = mInputs.valueFor(activeInput);
        newDevice = getDeviceForInputSource(inputDesc->mInputSource);
        if (newDevice != inputDesc->mDevice) {
            LOGV("setForceUse() changing device from %x to %x for input %d",
                    inputDesc->mDevice, newDevice, activeInput);
            inputDesc->mDevice = newDevice;
            AudioParameter param = AudioParameter();
            param.addInt(String8(AudioParameter::keyRouting), (int)newDevice);
            mpClientInterface->setParameters(activeInput, param.toString());
        }
    }
}

AudioSystem::forced_config AudioYusuPolicyManager::getForceUse(AudioSystem::force_use usage)
{
    return mForceUse[usage];
}

void AudioYusuPolicyManager::setSystemProperty(const char* property, const char* value)
{
    LOGD("setSystemProperty() property %s, value %s", property, value);
    if (strcmp(property, "ro.camera.sound.forced") == 0) {
        if (atoi(value)) {
            LOGV("ENFORCED_AUDIBLE cannot be muted");
            mStreams[AudioSystem::ENFORCED_AUDIBLE].mCanBeMuted = false;
        } else {
            //PR-277691-Zhengyong.Wang-002 Modified begin
            char soundPlayValue[PROPERTY_VALUE_MAX];
            property_get("Def.shuttersound.enable", soundPlayValue, "true");
            if(!strncmp(soundPlayValue, "true", 4)) {
                LOGV("ENFORCED_AUDIBLE can not be muted");
                mStreams[AudioSystem::ENFORCED_AUDIBLE].mCanBeMuted = false;
            } else {
                LOGV("ENFORCED_AUDIBLE can be muted");
                mStreams[AudioSystem::ENFORCED_AUDIBLE].mCanBeMuted = true;
            }
            //PR-277691-Zhengyong.Wang-002 Modified end
        }
    }
}

audio_io_handle_t AudioYusuPolicyManager::getOutput(AudioSystem::stream_type stream,
                                    uint32_t samplingRate,
                                    uint32_t format,
                                    uint32_t channels,
                                    AudioSystem::output_flags flags)
{
    audio_io_handle_t output = 0;
    uint32_t latency = 0;
    routing_strategy strategy = getStrategy((AudioSystem::stream_type)stream);
    uint32_t device = getDeviceForStrategy(strategy);
    LOGV("getOutput() stream %d, samplingRate %d, format %d, channels %x, flags %x", stream, samplingRate, format, channels, flags);

#ifdef AUDIO_POLICY_TEST
    if (mCurOutput != 0) {
        LOGV("getOutput() test output mCurOutput %d, samplingRate %d, format %d, channels %x, mDirectOutput %d",
                mCurOutput, mTestSamplingRate, mTestFormat, mTestChannels, mDirectOutput);

        if (mTestOutputs[mCurOutput] == 0) {
            LOGV("getOutput() opening test output");
            AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor();
            outputDesc->mDevice = mTestDevice;
            outputDesc->mSamplingRate = mTestSamplingRate;
            outputDesc->mFormat = mTestFormat;
            outputDesc->mChannels = mTestChannels;
            outputDesc->mLatency = mTestLatencyMs;
            outputDesc->mFlags = (AudioSystem::output_flags)(mDirectOutput ? AudioSystem::OUTPUT_FLAG_DIRECT : 0);
            outputDesc->mRefCount[stream] = 0;
            mTestOutputs[mCurOutput] = mpClientInterface->openOutput(&outputDesc->mDevice,
                                            &outputDesc->mSamplingRate,
                                            &outputDesc->mFormat,
                                            &outputDesc->mChannels,
                                            &outputDesc->mLatency,
                                            outputDesc->mFlags);
            if (mTestOutputs[mCurOutput]) {
                AudioParameter outputCmd = AudioParameter();
                outputCmd.addInt(String8("set_id"),mCurOutput);
                mpClientInterface->setParameters(mTestOutputs[mCurOutput],outputCmd.toString());
                addOutput(mTestOutputs[mCurOutput], outputDesc);
            }
        }
        return mTestOutputs[mCurOutput];
    }
#endif //AUDIO_POLICY_TEST

    // open a direct output if required by specified parameters
    if (needsDirectOuput(stream, samplingRate, format, channels, flags, device)) {

        LOGV("getOutput() opening direct output device %x", device);
        AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor();
        outputDesc->mDevice = device;
        outputDesc->mSamplingRate = samplingRate;
        outputDesc->mFormat = format;
        outputDesc->mChannels = channels;
        outputDesc->mLatency = 0;
        outputDesc->mFlags = (AudioSystem::output_flags)(flags | AudioSystem::OUTPUT_FLAG_DIRECT);
        outputDesc->mRefCount[stream] = 0;
        outputDesc->mStopTime[stream] = 0;
        output = mpClientInterface->openOutput(&outputDesc->mDevice,
                                        &outputDesc->mSamplingRate,
                                        &outputDesc->mFormat,
                                        &outputDesc->mChannels,
                                        &outputDesc->mLatency,
                                        outputDesc->mFlags);

        // only accept an output with the requeted parameters
        if (output == 0 ||
            (samplingRate != 0 && samplingRate != outputDesc->mSamplingRate) ||
            (format != 0 && format != outputDesc->mFormat) ||
            (channels != 0 && channels != outputDesc->mChannels)) {
            LOGV("getOutput() failed opening direct output: samplingRate %d, format %d, channels %d",
                    samplingRate, format, channels);
            if (output != 0) {
                mpClientInterface->closeOutput(output);
            }
            delete outputDesc;
            return 0;
        }
        addOutput(output, outputDesc);
        return output;
    }

    if (channels != 0 && channels != AudioSystem::CHANNEL_OUT_MONO &&
        channels != AudioSystem::CHANNEL_OUT_STEREO) {
        return 0;
    }
    // open a non direct output

    // get which output is suitable for the specified stream. The actual routing change will happen
    // when startOutput() will be called
    uint32_t a2dpDevice = device & AudioSystem::DEVICE_OUT_ALL_A2DP;
    if (AudioSystem::popCount((AudioSystem::audio_devices)device) == 2) {
#ifdef WITH_A2DP
        if (a2dpUsedForSonification() && a2dpDevice != 0) {
            // if playing on 2 devices among which one is A2DP, use duplicated output
            LOGV("getOutput() using duplicated output");
            LOGW_IF((mA2dpOutput == 0), "getOutput() A2DP device in multiple %x selected but A2DP output not opened", device);
            output = mDuplicatedOutput;
        } else
#endif
        {
            // if playing on 2 devices among which none is A2DP, use hardware output
            output = mHardwareOutput;
        }
        LOGV("getOutput() using output %d for 2 devices %x", output, device);
    } else {
#ifdef WITH_A2DP
        if (a2dpDevice != 0) {
            // if playing on A2DP device, use a2dp output
            LOGW_IF((mA2dpOutput == 0), "getOutput() A2DP device %x selected but A2DP output not opened", device);
            output = mA2dpOutput;
        } else
#endif
        {
            // if playing on not A2DP device, use hardware output
            output = mHardwareOutput;
        }
    }


    LOGW_IF((output ==0), "getOutput() could not find output for stream %d, samplingRate %d, format %d, channels %x, flags %x",
                stream, samplingRate, format, channels, flags);

    return output;
}
#ifndef ANDROID_DEFAULT_CODE
bool AudioYusuPolicyManager::ForcedAudioNeedMute(audio_io_handle_t output)
{
    if(mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)
        return true;
    else if(mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET)
        return true;
    else
        return false;
}


void AudioYusuPolicyManager::SetStreamMuteForceSpeaker(bool enable ,int mHardwareOutput, int delay)
{
    for(int i=0; i < AudioSystem::NUM_STREAM_TYPES ; i++){
        if(streamMuteInForceSpeaker(i) == true)
            setStreamMute (i, enable, mHardwareOutput,delay);
    }
}

void AudioYusuPolicyManager::SetStreamStart_UCM(audio_io_handle_t output,
                                                                                        AudioSystem::stream_type stream)
{
    LOGD("SetStreamStart_UCM output = %d stream = %d",output,stream);
    // tell audiohardware stream indexMax
    AudioParameter outputCmd = AudioParameter();
    outputCmd.addInt(String8(keySetStreamStart),(int)stream);
    mpClientInterface->setParameters(0, outputCmd.toString ());
}

void AudioYusuPolicyManager::SetStreamStop_UCM(audio_io_handle_t output,
                                                                                        AudioSystem::stream_type stream, int delay)
{
    LOGD("SetStreamStop_UCM output = %d stream = %d",output,stream);
    // tell audiohardware stream indexMax
    AudioParameter outputCmd = AudioParameter();
    outputCmd.addInt(String8(keySetStreamStop),(int)stream);
    mpClientInterface->setParameters(0, outputCmd.toString (),delay);
}
#endif

status_t AudioYusuPolicyManager::startOutput(audio_io_handle_t output,
                                             AudioSystem::stream_type stream,
                                             int session)
{
    LOGD("startOutput() output %d, stream %d, session %d", output, stream, session);
    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        LOGW("startOutput() unknow output %d", output);
        return BAD_VALUE;
    }

    AudioOutputDescriptor *outputDesc = mOutputs.valueAt(index);
    routing_strategy strategy = getStrategy((AudioSystem::stream_type)stream);

#ifdef WITH_A2DP
    if (mA2dpOutput != 0  && !a2dpUsedForSonification() &&
            (strategy == STRATEGY_SONIFICATION || strategy == STRATEGY_ENFORCED_AUDIBLE)) {
        setStrategyMute(STRATEGY_MEDIA, true, mA2dpOutput);
    }
#endif

    // incremenent usage count for this stream on the requested output:
    // NOTE that the usage count is the same for duplicated output and hardware output which is
    // necassary for a correct control of hardware output routing by startOutput() and stopOutput()
    outputDesc->changeRefCount(stream, 1);

    uint32_t prevDevice = outputDesc->mDevice;
    setOutputDevice(output, getNewDevice(output));

    // handle special case for sonification while in call
    if (isInCall()) {
        handleIncallSonification(stream, true, false);
    }

    // apply volume rules for current stream and device if necessary
    checkAndSetVolume(stream, mStreams[stream].mIndexCur, output, outputDesc->device());
#ifndef ANDROID_DEFAULT_CODE
    #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
    SetStreamStart_UCM(output,stream);
    #endif
#endif

#ifdef ANDROID_DEFAULT_CODE
    // FIXME: need a delay to make sure that audio path switches to speaker before sound
    // starts. Should be platform specific?
    if (stream == AudioSystem::ENFORCED_AUDIBLE &&
            prevDevice != outputDesc->mDevice) {
        usleep(outputDesc->mLatency*4*1000);
    }
#endif

    return NO_ERROR;
}

status_t AudioYusuPolicyManager::stopOutput(audio_io_handle_t output,
                                            AudioSystem::stream_type stream,
                                            int session)
{
    LOGD("stopOutput() output %d, stream %d, session %d", output, stream, session);
    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        LOGW("stopOutput() unknow output %d", output);
        return BAD_VALUE;
    }

    AudioOutputDescriptor *outputDesc = mOutputs.valueAt(index);
    routing_strategy strategy = getStrategy((AudioSystem::stream_type)stream);

    // handle special case for sonification while in call
    if (isInCall()) {
        handleIncallSonification(stream, false, false);
    }

    if (outputDesc->mRefCount[stream] > 0) {
        // decrement usage count of this stream on the output
        outputDesc->changeRefCount(stream, -1);
        // store time at which the stream was stopped - see isStreamActive()
        outputDesc->mStopTime[stream] = systemTime();

#ifndef ANDROID_DEFAULT_CODE
        if (stream == AudioSystem::MUSIC) {
            mMusicStopTime = systemTime();
        }
#endif
setOutputDevice(output, getNewDevice(output), false, outputDesc->mLatency*2);

#ifdef WITH_A2DP
        if (mA2dpOutput != 0 && !a2dpUsedForSonification() &&
                (strategy == STRATEGY_SONIFICATION || strategy == STRATEGY_ENFORCED_AUDIBLE)) {
            setStrategyMute(STRATEGY_MEDIA,
                            false,
                            mA2dpOutput,
                            mOutputs.valueFor(mHardwareOutput)->mLatency*2);
        }
#endif
        if (output != mHardwareOutput) {
            setOutputDevice(mHardwareOutput, getNewDevice(mHardwareOutput), true);
        }
#ifndef ANDROID_DEFAULT_CODE
        #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
        SetStreamStop_UCM(0,stream);
        #endif
#endif

        return NO_ERROR;
    } else {
        LOGW("stopOutput() refcount is already 0 for output %d", output);
#ifndef ANDROID_DEFAULT_CODE
        #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
        SetStreamStop_UCM(0,stream);
        #endif
#endif
        return INVALID_OPERATION;
    }
}


void AudioYusuPolicyManager::releaseOutput(audio_io_handle_t output)
{
    LOGD("releaseOutput() %d", output);
    ssize_t index = mOutputs.indexOfKey(output);
    if (index < 0) {
        LOGW("releaseOutput() releasing unknown output %d", output);
        return;
    }

#ifdef AUDIO_POLICY_TEST
    int testIndex = testOutputIndex(output);
    if (testIndex != 0) {
        AudioOutputDescriptor *outputDesc = mOutputs.valueAt(index);
        if (outputDesc->refCount() == 0) {
            mpClientInterface->closeOutput(output);
            delete mOutputs.valueAt(index);
            mOutputs.removeItem(output);
            mTestOutputs[testIndex] = 0;
        }
        return;
    }
#endif //AUDIO_POLICY_TEST

    if (mOutputs.valueAt(index)->mFlags & AudioSystem::OUTPUT_FLAG_DIRECT) {
        mpClientInterface->closeOutput(output);
        delete mOutputs.valueAt(index);
        mOutputs.removeItem(output);
    }
}

audio_io_handle_t AudioYusuPolicyManager::getInput(int inputSource,
                                    uint32_t samplingRate,
                                    uint32_t format,
                                    uint32_t channels,
                                    AudioSystem::audio_in_acoustics acoustics)
{
    audio_io_handle_t input = 0;
    uint32_t device = getDeviceForInputSource(inputSource);

    LOGD("getInput() inputSource %d, samplingRate %d, format %d, channels %x, acoustics %x", inputSource, samplingRate, format, channels, acoustics);

    if (device == 0) {
        return 0;
    }

    // adapt channel selection to input source
    switch(inputSource) {
    case AUDIO_SOURCE_VOICE_UPLINK:
        channels = AudioSystem::CHANNEL_IN_VOICE_UPLINK;
        break;
    case AUDIO_SOURCE_VOICE_DOWNLINK:
        channels = AudioSystem::CHANNEL_IN_VOICE_DNLINK;
        break;
    case AUDIO_SOURCE_VOICE_CALL:
        channels = (AudioSystem::CHANNEL_IN_VOICE_UPLINK | AudioSystem::CHANNEL_IN_VOICE_DNLINK);
        break;
    default:
        break;
    }

    AudioInputDescriptor *inputDesc = new AudioInputDescriptor();

    inputDesc->mInputSource = inputSource;
    inputDesc->mDevice = device;
    inputDesc->mSamplingRate = samplingRate;
    inputDesc->mFormat = format;
    inputDesc->mChannels = channels;
    inputDesc->mAcoustics = acoustics;
    inputDesc->mRefCount = 0;
#ifndef ANDROID_DEFAULT_CODE

    for(size_t count =0; count <mInputs.size();count++ ){
        AudioInputDescriptor *inputTemp = mInputs.valueAt(count);
        if(inputTemp->mDevice == inputDesc->mDevice){
            if ((samplingRate != inputTemp->mSamplingRate) ||(format != inputTemp->mFormat)||(channels != inputTemp->mChannels)) {
                LOGD("getInput() failed opening input: samplingRate %d, format %d, channels %d",samplingRate, format, channels);
                delete inputDesc;
                return 0;
           }
            if(inputTemp->mRefCount >0){
                LOGD("getInput() inputTemp->mRefCount =  %d",inputTemp->mRefCount);
                delete inputDesc;
                return 0;
            }
           else{
               LOGD("getInput return mInputs.keyAt(count) == %d",mInputs.keyAt(count));
               delete inputDesc;
               return mInputs.keyAt(count);
           }
        }
    }
#endif
    input = mpClientInterface->openInput(&inputDesc->mDevice,
                                    &inputDesc->mSamplingRate,
                                    &inputDesc->mFormat,
                                    &inputDesc->mChannels,
                                    inputDesc->mAcoustics);

    // only accept input with the exact requested set of parameters
    if (input == 0 ||
        (samplingRate != inputDesc->mSamplingRate) ||
        (format != inputDesc->mFormat) ||
        (channels != inputDesc->mChannels)) {
        LOGV("getInput() failed opening input: samplingRate %d, format %d, channels %d",
                samplingRate, format, channels);
        if (input != 0) {
            mpClientInterface->closeInput(input);
        }
        delete inputDesc;
        return 0;
    }
    mInputs.add(input, inputDesc);
    LOGD("mInputs add input == %d",input);
    return input;
}

status_t AudioYusuPolicyManager::startInput(audio_io_handle_t input)
{
    LOGD("startInput() input %d", input);
    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        LOGW("startInput() unknow input %d", input);
        return BAD_VALUE;
    }
    AudioInputDescriptor *inputDesc = mInputs.valueAt(index);

#ifdef AUDIO_POLICY_TEST
    if (mTestInput == 0)
#endif //AUDIO_POLICY_TEST
    {
        // refuse 2 active AudioRecord clients at the same time
        if (getActiveInput() != 0) {
            LOGW("startInput() input %d failed: other input already started", input);
            return INVALID_OPERATION;
        }
    }

    AudioParameter param = AudioParameter();
    param.addInt(String8(AudioParameter::keyRouting), (int)inputDesc->mDevice);

    param.addInt(String8(AudioParameter::keyInputSource), (int)inputDesc->mInputSource);
    LOGV("AudioPolicyManager::startInput() input source = %d", inputDesc->mInputSource);

    mpClientInterface->setParameters(input, param.toString());

    inputDesc->mRefCount = 1;
    return NO_ERROR;
}

status_t AudioYusuPolicyManager::stopInput(audio_io_handle_t input)
{
    LOGD("stopInput() input %d", input);
    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        LOGW("stopInput() unknow input %d", input);
        return BAD_VALUE;
    }
    AudioInputDescriptor *inputDesc = mInputs.valueAt(index);

    if (inputDesc->mRefCount == 0) {
        LOGW("stopInput() input %d already stopped", input);
        return INVALID_OPERATION;
    } else {
        AudioParameter param = AudioParameter();
        param.addInt(String8(AudioParameter::keyRouting), 0);
        mpClientInterface->setParameters(input, param.toString());
        inputDesc->mRefCount = 0;
        return NO_ERROR;
    }
}

void AudioYusuPolicyManager::releaseInput(audio_io_handle_t input)
{
    LOGD("releaseInput() %d", input);
    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        LOGW("releaseInput() releasing unknown input %d", input);
        return;
    }
    mpClientInterface->closeInput(input);
    delete mInputs.valueAt(index);
    mInputs.removeItem(input);
    LOGV("releaseInput() exit");
}

#ifndef ANDROID_DEFAULT_CODE
void AudioYusuPolicyManager::InitVolumeIndex_UCM(AudioSystem::stream_type stream,
                                            int indexMin,
                                            int indexMax)
{
    LOGD("InitVolumeIndex_UCM() stream %d, min %d, max %d", stream , indexMin, indexMax);
    // tell audiohardware stream indexMax
    AudioParameter outputCmd = AudioParameter();
    int StreamVolume =0;
    StreamVolume|= stream;
    StreamVolume = StreamVolume << 4;
    StreamVolume|= indexMin;
    StreamVolume = StreamVolume << 4;
    StreamVolume|= indexMax;
    outputCmd.addInt(String8(keyInitVoume),StreamVolume);
    mpClientInterface->setParameters(0, outputCmd.toString ());
}
#endif


void AudioYusuPolicyManager::initStreamVolume(AudioSystem::stream_type stream,
                                            int indexMin,
                                            int indexMax)
{
    LOGD("initStreamVolume() stream %d, min %d, max %d", stream , indexMin, indexMax);
    if (indexMin < 0 || indexMin >= indexMax) {
        LOGW("initStreamVolume() invalid index limits for stream %d, min %d, max %d", stream , indexMin, indexMax);
        return;
    }
    mStreams[stream].mIndexMin = indexMin;
    mStreams[stream].mIndexMax = indexMax;
    mStreams[stream].mIndexRange = (float)volume_Mapping_Step/indexMax;
#ifndef ANDROID_DEFAULT_CODE
    if(stream == AudioSystem::TTS) //makesure setparameter keyDetectHeadSet only once.
    {
        AudioParameter outputCmd = AudioParameter();
        outputCmd.addInt(String8(keyDetectHeadset),1);
        mpClientInterface->setParameters(0, outputCmd.toString());
    }
    #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
    InitVolumeIndex_UCM(stream,indexMin,indexMax);
    #endif
#endif
}

status_t AudioYusuPolicyManager::setStreamVolumeIndex(AudioSystem::stream_type stream, int index)
{

    if ((index < mStreams[stream].mIndexMin) || (index > mStreams[stream].mIndexMax)) {
        return BAD_VALUE;
    }

    // Force max volume if stream cannot be muted
    if (!mStreams[stream].mCanBeMuted) index = mStreams[stream].mIndexMax;

    LOGV("setStreamVolumeIndex() stream %d, index %d", stream, index);
    mStreams[stream].mIndexCur = index;

    // compute and apply stream volume on all outputs according to connected device
    status_t status = NO_ERROR;
    for (size_t i = 0; i < mOutputs.size(); i++) {
#ifndef ANDROID_DEFAULT_CODE
        status_t volStatus;
        //<----weiguo  alps00053099  adjust matv volume  when record sound stoping, matv sound will out through speaker.
        //   delay  -1 to put this volume command at the end of command queue in audiopolicy service
        if(stream ==AudioSystem::MATV  &&
            (mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET ||
             mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE))
        {
            volStatus =checkAndSetVolume(stream, index, mOutputs.keyAt(i), mOutputs.valueAt(i)->device(),-1);
        } //weiguo---->
        else
        {
            volStatus =checkAndSetVolume(stream, index, mOutputs.keyAt(i), mOutputs.valueAt(i)->device());
        }
#else
        status_t volStatus = checkAndSetVolume(stream, index, mOutputs.keyAt(i), mOutputs.valueAt(i)->device());
#endif
        if (volStatus != NO_ERROR) {
            status = volStatus;
        }
    }
        return status;
}

status_t AudioYusuPolicyManager::getStreamVolumeIndex(AudioSystem::stream_type stream, int *index)
{
    if (index == 0) {
        return BAD_VALUE;
    }
    LOGV("getStreamVolumeIndex() stream %d", stream);
    *index =  mStreams[stream].mIndexCur;
    return NO_ERROR;
}

audio_io_handle_t AudioYusuPolicyManager::getOutputForEffect(effect_descriptor_t *desc)
{
    LOGV("getOutputForEffect()");
    // apply simple rule where global effects are attached to the same output as MUSIC streams
    return getOutput(AudioSystem::MUSIC);
}

status_t AudioYusuPolicyManager::registerEffect(effect_descriptor_t *desc,
                                audio_io_handle_t io,
                                uint32_t strategy,
                                int session,
                                int id)
{
    ssize_t index = mOutputs.indexOfKey(io);
    if (index < 0) {
        index = mInputs.indexOfKey(io);
        if (index < 0) {
            LOGW("registerEffect() unknown io %d", io);
            return INVALID_OPERATION;
        }
    }

    if (mTotalEffectsMemory + desc->memoryUsage > getMaxEffectsMemory()) {
        LOGW("registerEffect() memory limit exceeded for Fx %s, Memory %d KB",
                desc->name, desc->memoryUsage);
        return INVALID_OPERATION;
    }
    mTotalEffectsMemory += desc->memoryUsage;
    LOGV("registerEffect() effect %s, io %d, strategy %d session %d id %d",
            desc->name, io, strategy, session, id);
    LOGV("registerEffect() memory %d, total memory %d", desc->memoryUsage, mTotalEffectsMemory);

    EffectDescriptor *pDesc = new EffectDescriptor();
    memcpy (&pDesc->mDesc, desc, sizeof(effect_descriptor_t));
    pDesc->mIo = io;
    pDesc->mStrategy = (routing_strategy)strategy;
    pDesc->mSession = session;
    pDesc->mEnabled = false;

    mEffects.add(id, pDesc);

    return NO_ERROR;
}

status_t AudioYusuPolicyManager::unregisterEffect(int id)
{
    ssize_t index = mEffects.indexOfKey(id);
    if (index < 0) {
        LOGW("unregisterEffect() unknown effect ID %d", id);
        return INVALID_OPERATION;
    }

    EffectDescriptor *pDesc = mEffects.valueAt(index);

    setEffectEnabled(pDesc, false);

    if (mTotalEffectsMemory < pDesc->mDesc.memoryUsage) {
        LOGW("unregisterEffect() memory %d too big for total %d",
                pDesc->mDesc.memoryUsage, mTotalEffectsMemory);
        pDesc->mDesc.memoryUsage = mTotalEffectsMemory;
    }
    mTotalEffectsMemory -= pDesc->mDesc.memoryUsage;
    LOGV("unregisterEffect() effect %s, ID %d, memory %d total memory %d",
            pDesc->mDesc.name, id, pDesc->mDesc.memoryUsage, mTotalEffectsMemory);

    mEffects.removeItem(id);
    delete pDesc;

    return NO_ERROR;
}

status_t AudioYusuPolicyManager::setEffectEnabled(int id, bool enabled)
{
    ssize_t index = mEffects.indexOfKey(id);
    if (index < 0) {
        LOGW("unregisterEffect() unknown effect ID %d", id);
        return INVALID_OPERATION;
    }

    return setEffectEnabled(mEffects.valueAt(index), enabled);
}

status_t AudioYusuPolicyManager::setEffectEnabled(EffectDescriptor *pDesc, bool enabled)
{
    if (enabled == pDesc->mEnabled) {
        LOGV("setEffectEnabled(%s) effect already %s",
             enabled?"true":"false", enabled?"enabled":"disabled");
        return INVALID_OPERATION;
    }

    if (enabled) {
        if (mTotalEffectsCpuLoad + pDesc->mDesc.cpuLoad > getMaxEffectsCpuLoad()) {
            LOGW("setEffectEnabled(true) CPU Load limit exceeded for Fx %s, CPU %f MIPS",
                 pDesc->mDesc.name, (float)pDesc->mDesc.cpuLoad/10);
            return INVALID_OPERATION;
        }
        mTotalEffectsCpuLoad += pDesc->mDesc.cpuLoad;
        LOGV("setEffectEnabled(true) total CPU %d", mTotalEffectsCpuLoad);
    } else {
        if (mTotalEffectsCpuLoad < pDesc->mDesc.cpuLoad) {
            LOGW("setEffectEnabled(false) CPU load %d too high for total %d",
                    pDesc->mDesc.cpuLoad, mTotalEffectsCpuLoad);
            pDesc->mDesc.cpuLoad = mTotalEffectsCpuLoad;
        }
        mTotalEffectsCpuLoad -= pDesc->mDesc.cpuLoad;
        LOGV("setEffectEnabled(false) total CPU %d", mTotalEffectsCpuLoad);
    }
    pDesc->mEnabled = enabled;
    return NO_ERROR;
}

bool AudioYusuPolicyManager::isStreamActive(int stream, uint32_t inPastMs) const
{
    nsecs_t sysTime = systemTime();
    for (size_t i = 0; i < mOutputs.size(); i++) {
        if (mOutputs.valueAt(i)->mRefCount[stream] != 0 ||
            ns2ms(sysTime - mOutputs.valueAt(i)->mStopTime[stream]) < inPastMs) {
            return true;
        }
    }
    return false;
}

status_t AudioYusuPolicyManager::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "\nAudioPolicyManager Dump: %p\n", this);
    result.append(buffer);
    snprintf(buffer, SIZE, " Hardware Output: %d\n", mHardwareOutput);
    result.append(buffer);
#ifdef WITH_A2DP
    snprintf(buffer, SIZE, " A2DP Output: %d\n", mA2dpOutput);
    result.append(buffer);
    snprintf(buffer, SIZE, " Duplicated Output: %d\n", mDuplicatedOutput);
    result.append(buffer);
    snprintf(buffer, SIZE, " A2DP device address: %s\n", mA2dpDeviceAddress.string());
    result.append(buffer);
#endif
    snprintf(buffer, SIZE, " SCO device address: %s\n", mScoDeviceAddress.string());
    result.append(buffer);
    snprintf(buffer, SIZE, " Output devices: %08x\n", mAvailableOutputDevices);
    result.append(buffer);
    snprintf(buffer, SIZE, " Input devices: %08x\n", mAvailableInputDevices);
    result.append(buffer);
    snprintf(buffer, SIZE, " Phone state: %d\n", mPhoneState);
    result.append(buffer);
    snprintf(buffer, SIZE, " Ringer mode: %d\n", mRingerMode);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for communications %d\n", mForceUse[AudioSystem::FOR_COMMUNICATION]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for media %d\n", mForceUse[AudioSystem::FOR_MEDIA]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for record %d\n", mForceUse[AudioSystem::FOR_RECORD]);
    result.append(buffer);
    snprintf(buffer, SIZE, " Force use for dock %d\n", mForceUse[AudioSystem::FOR_DOCK]);
    result.append(buffer);
    write(fd, result.string(), result.size());

    snprintf(buffer, SIZE, "\nOutputs dump:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mOutputs.size(); i++) {
        snprintf(buffer, SIZE, "- Output %d dump:\n", mOutputs.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mOutputs.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nInputs dump:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mInputs.size(); i++) {
        snprintf(buffer, SIZE, "- Input %d dump:\n", mInputs.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mInputs.valueAt(i)->dump(fd);
    }

    snprintf(buffer, SIZE, "\nStreams dump:\n");
    write(fd, buffer, strlen(buffer));
    snprintf(buffer, SIZE, " Stream  Index Min  Index Max  Index Cur  Can be muted\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        snprintf(buffer, SIZE, " %02d", i);
        mStreams[i].dump(buffer + 3, SIZE);
        write(fd, buffer, strlen(buffer));
    }

    snprintf(buffer, SIZE, "\nTotal Effects CPU: %f MIPS, Total Effects memory: %d KB\n",
            (float)mTotalEffectsCpuLoad/10, mTotalEffectsMemory);
    write(fd, buffer, strlen(buffer));

    snprintf(buffer, SIZE, "Registered effects:\n");
    write(fd, buffer, strlen(buffer));
    for (size_t i = 0; i < mEffects.size(); i++) {
        snprintf(buffer, SIZE, "- Effect %d dump:\n", mEffects.keyAt(i));
        write(fd, buffer, strlen(buffer));
        mEffects.valueAt(i)->dump(fd);
    }


    return NO_ERROR;
}

// ----------------------------------------------------------------------------
// AudioYusuPolicyManager
// ----------------------------------------------------------------------------

AudioYusuPolicyManager::AudioYusuPolicyManager(AudioPolicyClientInterface *clientInterface)
    :
#ifdef AUDIO_POLICY_TEST
    Thread(false),
#endif //AUDIO_POLICY_TEST
    mPhoneState(AudioSystem::MODE_NORMAL), mRingerMode(0),
    mLimitRingtoneVolume(false), mLastVoiceVolume(-1.0f),
    mTotalEffectsCpuLoad(0), mTotalEffectsMemory(0),
    mA2dpSuspended(false)
{
    mpClientInterface = clientInterface;
#ifndef ANDROID_DEFAULT_CODE
    LoadCustomVolume();
    //GetAudioCustomParamFromNV (&Audio_Custom_Volume);

    #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
    android::GetAudioGainTableParamFromNV(&Audio_gaintable_Custom_Volume);
    #endif

    for(int i=0; i < MAX_VOL_CATE; i++)
       for(int k=0;k< MAX_VOL_TYPE ; k++){
           LOGV("audiovolume_fmr Audio_Custom_Volume.[%d][%d] =%d ",i,k,Audio_Custom_Volume.audiovolume_fmr[i][k]);
    }

    for(int i=0; i < MAX_VOL_CATE; i++)
       for(int k=0;k< MAX_VOL_TYPE ; k++){
           LOGV("audiovolume_media  Audio_Custom_Volume.[%d][%d] =%d ",i,k,Audio_Custom_Volume.audiovolume_media[i][k]);
    }
    for(int i=0; i < MAX_VOL_CATE; i++)
       for(int k=0;k< MAX_VOL_TYPE ; k++){
           LOGV("audiovolume_matv  Audio_Custom_Volume.[%d][%d] =%d ",i,k,Audio_Custom_Volume.audiovolume_matv[i][k]);
    }
    for(int i=0; i < MAX_VOL_CATE; i++)
        for(int k=0;k< MAX_VOL_TYPE ; k++){
        LOGV("audiovolume_key  Audio_Custom_Volume.[%d][%d] =%d ",i,k,Audio_Custom_Volume.audiovolume_key[i][k]);
    }
#endif

    for (int i = 0; i < AudioSystem::NUM_FORCE_USE; i++) {
        mForceUse[i] = AudioSystem::FORCE_NONE;
    }

    initializeVolumeCurves();

    // devices available by default are speaker, ear piece and microphone
    mAvailableOutputDevices = AudioSystem::DEVICE_OUT_EARPIECE |
                        AudioSystem::DEVICE_OUT_SPEAKER;
    mAvailableInputDevices = AudioSystem::DEVICE_IN_BUILTIN_MIC;

#ifdef WITH_A2DP
    mA2dpOutput = 0;
    mDuplicatedOutput = 0;
    mA2dpDeviceAddress = String8("");
#endif
    mScoDeviceAddress = String8("");

    // open hardware output
    AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor();
    outputDesc->mDevice = (uint32_t)AudioSystem::DEVICE_OUT_SPEAKER;
    mHardwareOutput = mpClientInterface->openOutput(&outputDesc->mDevice,
                                    &outputDesc->mSamplingRate,
                                    &outputDesc->mFormat,
                                    &outputDesc->mChannels,
                                    &outputDesc->mLatency,
                                    outputDesc->mFlags);

    if (mHardwareOutput == 0) {
        LOGE("Failed to initialize hardware output stream, samplingRate: %d, format %d, channels %d",
                outputDesc->mSamplingRate, outputDesc->mFormat, outputDesc->mChannels);
    } else {
        addOutput(mHardwareOutput, outputDesc);
        setOutputDevice(mHardwareOutput, (uint32_t)AudioSystem::DEVICE_OUT_SPEAKER, true);
        //TODO: configure audio effect output stage here
    }

    updateDeviceForStrategy();
#ifdef AUDIO_POLICY_TEST
    if (mHardwareOutput != 0) {
        AudioParameter outputCmd = AudioParameter();
        outputCmd.addInt(String8("set_id"), 0);
        mpClientInterface->setParameters(mHardwareOutput, outputCmd.toString());

        mTestDevice = AudioSystem::DEVICE_OUT_SPEAKER;
        mTestSamplingRate = 44100;
        mTestFormat = AudioSystem::PCM_16_BIT;
        mTestChannels =  AudioSystem::CHANNEL_OUT_STEREO;
        mTestLatencyMs = 0;
        mCurOutput = 0;
        mDirectOutput = false;
        for (int i = 0; i < NUM_TEST_OUTPUTS; i++) {
            mTestOutputs[i] = 0;
        }

        const size_t SIZE = 256;
        char buffer[SIZE];
        snprintf(buffer, SIZE, "AudioPolicyManagerTest");
        run(buffer, ANDROID_PRIORITY_AUDIO);
    }
#endif //AUDIO_POLICY_TEST
#ifndef ANDROID_DEFAULT_CODE

    mForcedSpeakerCount =0;
    mForcedSpeakerMuteCount =0;
    mForcedAudibleCount =0;
    mFmForceSpeakerState= false;


    ActiveStream =0;
    PreActiveStream =-1;
#endif
}

AudioYusuPolicyManager::~AudioYusuPolicyManager()
{
#ifdef AUDIO_POLICY_TEST
    exit();
#endif //AUDIO_POLICY_TEST
   for (size_t i = 0; i < mOutputs.size(); i++) {
        mpClientInterface->closeOutput(mOutputs.keyAt(i));
        delete mOutputs.valueAt(i);
   }
   mOutputs.clear();
   for (size_t i = 0; i < mInputs.size(); i++) {
        mpClientInterface->closeInput(mInputs.keyAt(i));
        delete mInputs.valueAt(i);
   }
   mInputs.clear();
}

status_t AudioYusuPolicyManager::initCheck()
{
    return (mHardwareOutput == 0) ? NO_INIT : NO_ERROR;
}

#ifdef AUDIO_POLICY_TEST
bool AudioYusuPolicyManager::threadLoop()
{
    LOGV("entering threadLoop()");
    while (!exitPending())
    {
        String8 command;
        int valueInt;
        String8 value;

        Mutex::Autolock _l(mLock);
        mWaitWorkCV.waitRelative(mLock, milliseconds(50));

        command = mpClientInterface->getParameters(0, String8("test_cmd_policy"));
        AudioParameter param = AudioParameter(command);

        if (param.getInt(String8("test_cmd_policy"), valueInt) == NO_ERROR &&
            valueInt != 0) {
            LOGV("Test command %s received", command.string());
            String8 target;
            if (param.get(String8("target"), target) != NO_ERROR) {
                target = "Manager";
            }
            if (param.getInt(String8("test_cmd_policy_output"), valueInt) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_output"));
                mCurOutput = valueInt;
            }
            if (param.get(String8("test_cmd_policy_direct"), value) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_direct"));
                if (value == "false") {
                    mDirectOutput = false;
                } else if (value == "true") {
                    mDirectOutput = true;
                }
            }
            if (param.getInt(String8("test_cmd_policy_input"), valueInt) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_input"));
                mTestInput = valueInt;
            }

            if (param.get(String8("test_cmd_policy_format"), value) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_format"));
                int format = AudioSystem::INVALID_FORMAT;
                if (value == "PCM 16 bits") {
                    format = AudioSystem::PCM_16_BIT;
                } else if (value == "PCM 8 bits") {
                    format = AudioSystem::PCM_8_BIT;
                } else if (value == "Compressed MP3") {
                    format = AudioSystem::MP3;
                }
                if (format != AudioSystem::INVALID_FORMAT) {
                    if (target == "Manager") {
                        mTestFormat = format;
                    } else if (mTestOutputs[mCurOutput] != 0) {
                        AudioParameter outputParam = AudioParameter();
                        outputParam.addInt(String8("format"), format);
                        mpClientInterface->setParameters(mTestOutputs[mCurOutput], outputParam.toString());
                    }
                }
            }
            if (param.get(String8("test_cmd_policy_channels"), value) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_channels"));
                int channels = 0;

                if (value == "Channels Stereo") {
                    channels =  AudioSystem::CHANNEL_OUT_STEREO;
                } else if (value == "Channels Mono") {
                    channels =  AudioSystem::CHANNEL_OUT_MONO;
                }
                if (channels != 0) {
                    if (target == "Manager") {
                        mTestChannels = channels;
                    } else if (mTestOutputs[mCurOutput] != 0) {
                        AudioParameter outputParam = AudioParameter();
                        outputParam.addInt(String8("channels"), channels);
                        mpClientInterface->setParameters(mTestOutputs[mCurOutput], outputParam.toString());
                    }
                }
            }
            if (param.getInt(String8("test_cmd_policy_sampleRate"), valueInt) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_sampleRate"));
                if (valueInt >= 0 && valueInt <= 96000) {
                    int samplingRate = valueInt;
                    if (target == "Manager") {
                        mTestSamplingRate = samplingRate;
                    } else if (mTestOutputs[mCurOutput] != 0) {
                        AudioParameter outputParam = AudioParameter();
                        outputParam.addInt(String8("sampling_rate"), samplingRate);
                        mpClientInterface->setParameters(mTestOutputs[mCurOutput], outputParam.toString());
                    }
                }
            }

            if (param.get(String8("test_cmd_policy_reopen"), value) == NO_ERROR) {
                param.remove(String8("test_cmd_policy_reopen"));

                mpClientInterface->closeOutput(mHardwareOutput);
                delete mOutputs.valueFor(mHardwareOutput);
                mOutputs.removeItem(mHardwareOutput);

                AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor();
                outputDesc->mDevice = (uint32_t)AudioSystem::DEVICE_OUT_SPEAKER;
                mHardwareOutput = mpClientInterface->openOutput(&outputDesc->mDevice,
                                                &outputDesc->mSamplingRate,
                                                &outputDesc->mFormat,
                                                &outputDesc->mChannels,
                                                &outputDesc->mLatency,
                                                outputDesc->mFlags);
                if (mHardwareOutput == 0) {
                    LOGE("Failed to reopen hardware output stream, samplingRate: %d, format %d, channels %d",
                            outputDesc->mSamplingRate, outputDesc->mFormat, outputDesc->mChannels);
                } else {
                    AudioParameter outputCmd = AudioParameter();
                    outputCmd.addInt(String8("set_id"), 0);
                    mpClientInterface->setParameters(mHardwareOutput, outputCmd.toString());
                    addOutput(mHardwareOutput, outputDesc);
                }
            }


            mpClientInterface->setParameters(0, String8("test_cmd_policy="));
        }
    }
    return false;
}

void AudioYusuPolicyManager::exit()
{
    {
        AutoMutex _l(mLock);
        requestExit();
        mWaitWorkCV.signal();
    }
    requestExitAndWait();
}

int AudioYusuPolicyManager::testOutputIndex(audio_io_handle_t output)
{
    for (int i = 0; i < NUM_TEST_OUTPUTS; i++) {
        if (output == mTestOutputs[i]) return i;
    }
    return 0;
}
#endif //AUDIO_POLICY_TEST

// ---

void AudioYusuPolicyManager::addOutput(audio_io_handle_t id, AudioOutputDescriptor *outputDesc)
{
    outputDesc->mId = id;
    mOutputs.add(id, outputDesc);
}


#ifdef WITH_A2DP
status_t AudioYusuPolicyManager::handleA2dpConnection(AudioSystem::audio_devices device,
                                                 const char *device_address)
{
    // when an A2DP device is connected, open an A2DP and a duplicated output
    LOGD("opening A2DP output for device %s", device_address);
    AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor();
    outputDesc->mDevice = device;
    mA2dpOutput = mpClientInterface->openOutput(&outputDesc->mDevice,
                                            &outputDesc->mSamplingRate,
                                            &outputDesc->mFormat,
                                            &outputDesc->mChannels,
                                            &outputDesc->mLatency,
                                            outputDesc->mFlags);
    if (mA2dpOutput) {
        // add A2DP output descriptor
        addOutput(mA2dpOutput, outputDesc);

        //TODO: configure audio effect output stage here

        // set initial stream volume for A2DP device
        applyStreamVolumes(mA2dpOutput, device);
        if (a2dpUsedForSonification()) {
            mDuplicatedOutput = mpClientInterface->openDuplicateOutput(mA2dpOutput, mHardwareOutput);
        }
        if (mDuplicatedOutput != 0 ||
            !a2dpUsedForSonification()) {
            // If both A2DP and duplicated outputs are open, send device address to A2DP hardware
            // interface
            AudioParameter param;
            param.add(String8("a2dp_sink_address"), String8(device_address));
            mpClientInterface->setParameters(mA2dpOutput, param.toString());
            mA2dpDeviceAddress = String8(device_address, MAX_DEVICE_ADDRESS_LEN);

            if (a2dpUsedForSonification()) {
                // add duplicated output descriptor
                AudioOutputDescriptor *dupOutputDesc = new AudioOutputDescriptor();
                dupOutputDesc->mOutput1 = mOutputs.valueFor(mHardwareOutput);
                dupOutputDesc->mOutput2 = mOutputs.valueFor(mA2dpOutput);
                dupOutputDesc->mSamplingRate = outputDesc->mSamplingRate;
                dupOutputDesc->mFormat = outputDesc->mFormat;
                dupOutputDesc->mChannels = outputDesc->mChannels;
                dupOutputDesc->mLatency = outputDesc->mLatency;
                addOutput(mDuplicatedOutput, dupOutputDesc);
                applyStreamVolumes(mDuplicatedOutput, device);
            }
        } else {
            LOGW("getOutput() could not open duplicated output for %d and %d",
                    mHardwareOutput, mA2dpOutput);
            mpClientInterface->closeOutput(mA2dpOutput);
            mOutputs.removeItem(mA2dpOutput);
            mA2dpOutput = 0;
            delete outputDesc;
            return NO_INIT;
        }
    } else {
        LOGW("setDeviceConnectionState() could not open A2DP output for device %x", device);
        delete outputDesc;
        return NO_INIT;
    }
    AudioOutputDescriptor *hwOutputDesc = mOutputs.valueFor(mHardwareOutput);

    if (!a2dpUsedForSonification()) {
        // mute music on A2DP output if a notification or ringtone is playing
        uint32_t refCount = hwOutputDesc->strategyRefCount(STRATEGY_SONIFICATION);
        refCount += hwOutputDesc->strategyRefCount(STRATEGY_ENFORCED_AUDIBLE);
        for (uint32_t i = 0; i < refCount; i++) {
            setStrategyMute(STRATEGY_MEDIA, true, mA2dpOutput);
        }
    }

    mA2dpSuspended = false;

    return NO_ERROR;
}

status_t AudioYusuPolicyManager::handleA2dpDisconnection(AudioSystem::audio_devices device,
                                                    const char *device_address)
{
    if (mA2dpOutput == 0) {
        LOGW("setDeviceConnectionState() disconnecting A2DP and no A2DP output!");
        return INVALID_OPERATION;
    }

    if (mA2dpDeviceAddress != device_address) {
        LOGW("setDeviceConnectionState() disconnecting unknow A2DP sink address %s", device_address);
        return INVALID_OPERATION;
    }

    // mute media strategy to avoid outputting sound on hardware output while music stream
    // is switched from A2DP output and before music is paused by music application
    setStrategyMute(STRATEGY_MEDIA, true, mHardwareOutput);
    setStrategyMute(STRATEGY_MEDIA, false, mHardwareOutput, MUTE_TIME_MS);

    if (!a2dpUsedForSonification()) {
        // unmute music on A2DP output if a notification or ringtone is playing
        uint32_t refCount = mOutputs.valueFor(mHardwareOutput)->strategyRefCount(STRATEGY_SONIFICATION);
        refCount += mOutputs.valueFor(mHardwareOutput)->strategyRefCount(STRATEGY_ENFORCED_AUDIBLE);
        for (uint32_t i = 0; i < refCount; i++) {
            setStrategyMute(STRATEGY_MEDIA, false, mA2dpOutput);
        }
    }
    mA2dpDeviceAddress = "";
    mA2dpSuspended = false;
    return NO_ERROR;
}

void AudioYusuPolicyManager::closeA2dpOutputs()
{

    LOGD("setDeviceConnectionState() closing A2DP and duplicated output!");

    if (mDuplicatedOutput != 0) {
        AudioOutputDescriptor *dupOutputDesc = mOutputs.valueFor(mDuplicatedOutput);
        AudioOutputDescriptor *hwOutputDesc = mOutputs.valueFor(mHardwareOutput);
        // As all active tracks on duplicated output will be deleted,
        // and as they were also referenced on hardware output, the reference
        // count for their stream type must be adjusted accordingly on
        // hardware output.
        for (int i = 0; i < (int)AudioSystem::NUM_STREAM_TYPES; i++) {
            int refCount = dupOutputDesc->mRefCount[i];
            hwOutputDesc->changeRefCount((AudioSystem::stream_type)i,-refCount);
        }

        mpClientInterface->closeOutput(mDuplicatedOutput);
        delete mOutputs.valueFor(mDuplicatedOutput);
        mOutputs.removeItem(mDuplicatedOutput);
        mDuplicatedOutput = 0;
    }
    if (mA2dpOutput != 0) {
        AudioParameter param;
        param.add(String8("closing"), String8("true"));
        mpClientInterface->setParameters(mA2dpOutput, param.toString());

        mpClientInterface->closeOutput(mA2dpOutput);
        delete mOutputs.valueFor(mA2dpOutput);
        mOutputs.removeItem(mA2dpOutput);
        mA2dpOutput = 0;
    }
}

void AudioYusuPolicyManager::checkOutputForStrategy(routing_strategy strategy)
{
    uint32_t prevDevice = getDeviceForStrategy(strategy);
    uint32_t curDevice = getDeviceForStrategy(strategy, false);
    bool a2dpWasUsed = AudioSystem::isA2dpDevice((AudioSystem::audio_devices)(prevDevice & ~AudioSystem::DEVICE_OUT_SPEAKER));
    bool a2dpIsUsed = AudioSystem::isA2dpDevice((AudioSystem::audio_devices)(curDevice & ~AudioSystem::DEVICE_OUT_SPEAKER));
    audio_io_handle_t srcOutput = 0;
    audio_io_handle_t dstOutput = 0;

    if (a2dpWasUsed && !a2dpIsUsed) {
        bool dupUsed = a2dpUsedForSonification() && a2dpWasUsed && (AudioSystem::popCount(prevDevice) == 2);
        dstOutput = mHardwareOutput;
        if (dupUsed) {
            LOGV("checkOutputForStrategy() moving strategy %d from duplicated", strategy);
            srcOutput = mDuplicatedOutput;
        } else {
            LOGV("checkOutputForStrategy() moving strategy %d from a2dp", strategy);
            srcOutput = mA2dpOutput;
        }
    }
    if (a2dpIsUsed && !a2dpWasUsed) {
        bool dupUsed = a2dpUsedForSonification() && a2dpIsUsed && (AudioSystem::popCount(curDevice) == 2);
        srcOutput = mHardwareOutput;
        if (dupUsed) {
            LOGV("checkOutputForStrategy() moving strategy %d to duplicated", strategy);
            dstOutput = mDuplicatedOutput;
        } else {
            LOGV("checkOutputForStrategy() moving strategy %d to a2dp", strategy);
            dstOutput = mA2dpOutput;
        }
    }

    if (srcOutput != 0 && dstOutput != 0) {
#ifndef ANDROID_DEFAULT_CODE
#else
        // Move effects associated to this strategy from previous output to new output
        for (size_t i = 0; i < mEffects.size(); i++) {
            EffectDescriptor *desc = mEffects.valueAt(i);
            if (desc->mSession != AudioSystem::SESSION_OUTPUT_STAGE &&
                    desc->mStrategy == strategy &&
                    desc->mIo == srcOutput) {
                LOGV("checkOutputForStrategy() moving effect %d to output %d", mEffects.keyAt(i), dstOutput);
                mpClientInterface->moveEffects(desc->mSession, srcOutput, dstOutput);
                desc->mIo = dstOutput;
            }
        }
#endif
        // Move tracks associated to this strategy from previous output to new output
        for (int i = 0; i < (int)AudioSystem::NUM_STREAM_TYPES; i++) {
            if (getStrategy((AudioSystem::stream_type)i) == strategy) {
                mpClientInterface->setStreamOutput((AudioSystem::stream_type)i, dstOutput);
            }
        }
#ifndef ANDROID_DEFAULT_CODE
// Move effects associated to this strategy from previous output to new output
        for (size_t i = 0; i < mEffects.size(); i++) {
            EffectDescriptor *desc = mEffects.valueAt(i);
            if (desc->mSession != AudioSystem::SESSION_OUTPUT_STAGE &&
                    desc->mStrategy == strategy &&
                    desc->mIo == srcOutput) {
                LOGV("checkOutputForStrategy() moving effect %d to output %d", mEffects.keyAt(i), dstOutput);
                mpClientInterface->moveEffects(desc->mSession, srcOutput, dstOutput);
                desc->mIo = dstOutput;
            }
        }
#endif
    }
}

void AudioYusuPolicyManager::checkOutputForAllStrategies()
{
    checkOutputForStrategy(STRATEGY_ENFORCED_AUDIBLE);
    checkOutputForStrategy(STRATEGY_PHONE);
    checkOutputForStrategy(STRATEGY_SONIFICATION);
    checkOutputForStrategy(STRATEGY_MEDIA);
    checkOutputForStrategy(STRATEGY_DTMF);
#ifndef ANDROID_DEFAULT_CODE
    checkOutputForStrategy(STRATEGY_PROPRIETARY);
#endif
}

void AudioYusuPolicyManager::checkA2dpSuspend()
{
    // suspend A2DP output if:
    //      (NOT already suspended) &&
    //      ((SCO device is connected &&
    //       (forced usage for communication || for record is SCO))) ||
    //      (phone state is ringing || in call)
    //
    // restore A2DP output if:
    //      (Already suspended) &&
    //      ((SCO device is NOT connected ||
    //       (forced usage NOT for communication && NOT for record is SCO))) &&
    //      (phone state is NOT ringing && NOT in call)
    //
    if (mA2dpOutput == 0) {
        return;
    }

    if (mA2dpSuspended) {
        if (((mScoDeviceAddress == "") ||
             ((mForceUse[AudioSystem::FOR_COMMUNICATION] != AudioSystem::FORCE_BT_SCO) &&
              (mForceUse[AudioSystem::FOR_RECORD] != AudioSystem::FORCE_BT_SCO))) &&
             ((mPhoneState != AudioSystem::MODE_IN_CALL) &&
              (mPhoneState != AudioSystem::MODE_RINGTONE))) {

            mpClientInterface->restoreOutput(mA2dpOutput);
            mA2dpSuspended = false;
        }
    } else {
        if (((mScoDeviceAddress != "") &&
             ((mForceUse[AudioSystem::FOR_COMMUNICATION] == AudioSystem::FORCE_BT_SCO) ||
              (mForceUse[AudioSystem::FOR_RECORD] == AudioSystem::FORCE_BT_SCO))) ||
             ((mPhoneState == AudioSystem::MODE_IN_CALL) ||
              (mPhoneState == AudioSystem::MODE_RINGTONE))) {

            mpClientInterface->suspendOutput(mA2dpOutput);
            mA2dpSuspended = true;
        }
    }
}


#endif

uint32_t AudioYusuPolicyManager::getNewDevice(audio_io_handle_t output, bool fromCache)
{
    uint32_t device = 0;

    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    // check the following by order of priority to request a routing change if necessary:
    // 1: the strategy enforced audible is active on the output:
    //      use device for strategy enforced audible
    // 2: we are in call or the strategy phone is active on the output:
    //      use device for strategy phone
    // 3: the strategy sonification is active on the output:
    //      use device for strategy sonification
    // 4: the strategy media is active on the output:
    //      use device for strategy media
    // 5: the strategy DTMF is active on the output:
    //      use device for strategy DTMF
    if (outputDesc->isUsedByStrategy(STRATEGY_ENFORCED_AUDIBLE)) {
        device = getDeviceForStrategy(STRATEGY_ENFORCED_AUDIBLE, fromCache);
    } else if (isInCall() ||
                    outputDesc->isUsedByStrategy(STRATEGY_PHONE)) {
        device = getDeviceForStrategy(STRATEGY_PHONE, fromCache);
    } else if (outputDesc->isUsedByStrategy(STRATEGY_SONIFICATION)) {
        device = getDeviceForStrategy(STRATEGY_SONIFICATION, fromCache);
    } else if (outputDesc->isUsedByStrategy(STRATEGY_MEDIA)) {
        device = getDeviceForStrategy(STRATEGY_MEDIA, fromCache);
    } else if (outputDesc->isUsedByStrategy(STRATEGY_DTMF)) {
        device = getDeviceForStrategy(STRATEGY_DTMF, fromCache);
#ifndef ANDROID_DEFAULT_CODE
    }else if (outputDesc->isUsedByStrategy(STRATEGY_PROPRIETARY)) {
        device = getDeviceForStrategy(STRATEGY_PROPRIETARY, fromCache);
#endif
    }

    LOGV("getNewDevice() selected device %x output= %d", device,output);
    return device;
}

uint32_t AudioYusuPolicyManager::getStrategyForStream(AudioSystem::stream_type stream) {
    return (uint32_t)getStrategy(stream);
}

uint32_t AudioYusuPolicyManager::getDevicesForStream(AudioSystem::stream_type stream) {
    uint32_t devices;
    // By checking the range of stream before calling getStrategy, we avoid
    // getStrategy's behavior for invalid streams.  getStrategy would do a LOGE
    // and then return STRATEGY_MEDIA, but we want to return the empty set.
    if (stream < (AudioSystem::stream_type) 0 || stream >= AudioSystem::NUM_STREAM_TYPES) {
        devices = 0;
    } else {
        AudioYusuPolicyManager::routing_strategy strategy = getStrategy(stream);
        devices = getDeviceForStrategy(strategy, true);
    }
    return devices;
}
AudioYusuPolicyManager::routing_strategy AudioYusuPolicyManager::getStrategy(
        AudioSystem::stream_type stream) {
    // stream to strategy mapping
    switch (stream) {
    case AudioSystem::VOICE_CALL:
    case AudioSystem::BLUETOOTH_SCO:
        return STRATEGY_PHONE;
    case AudioSystem::RING:
    case AudioSystem::NOTIFICATION:
    case AudioSystem::ALARM:
        return STRATEGY_SONIFICATION;
    case AudioSystem::DTMF:
        return STRATEGY_DTMF;
#ifndef ANDROID_DEFAULT_CODE
    // FM and MATV strategy
    case AudioSystem::FM:
        return STRATEGY_PROPRIETARY;
    case AudioSystem::MATV:
    #ifdef MATV_AUDIO_LINEIN_PATH
        return STRATEGY_PROPRIETARY;
    #else
        return STRATEGY_MEDIA;
    #endif
#endif //#ifndef ANDROID_DEFAULT_CODE
    default:
        LOGE("unknown stream type");
    case AudioSystem::SYSTEM:
        // NOTE: SYSTEM stream uses MEDIA strategy because muting music and switching outputs
        // while key clicks are played produces a poor result
    case AudioSystem::TTS:
    case AudioSystem::MUSIC:
#ifndef ANDROID_DEFAULT_CODE
    case AudioSystem::BOOT:
#endif
        return STRATEGY_MEDIA;
    case AudioSystem::ENFORCED_AUDIBLE:
        return STRATEGY_ENFORCED_AUDIBLE;
     }
}

#ifndef ANDROID_DEFAULT_CODE
uint32 AudioYusuPolicyManager::GetMapVolumeIndex(uint32 volume){
    int Start_index =0;
    int End_index = VOLUME_STEP_NUMBER;
    int Search_Index =(End_index - Start_index)/2;
    int Search_Num;
    while(Start_index != End_index){
        if(volume < GainMap[Search_Index]){
            Start_index= Search_Index;
            Search_Num = (End_index - Start_index)/2;
            Search_Index = Start_index +Search_Num;
        }
        else{
            End_index = Search_Index;
            Search_Num = (End_index - Start_index)/2;
            Search_Index = Start_index +Search_Num;
        }
        if(End_index - Start_index <=1){
            break;
        }
    }
    return Start_index;
}
#endif

uint32_t AudioYusuPolicyManager::getDeviceForStrategy(routing_strategy strategy, bool fromCache)
{
    uint32_t device = 0;
    LOGV("getDeviceForStrategy() from cache strategy %d, device %x mAvailableOutputDevices = %x" , strategy, mDeviceForStrategy[strategy],mAvailableOutputDevices);

    if (fromCache) {
        LOGV("getDeviceForStrategy() from cache strategy %d, device %x", strategy, mDeviceForStrategy[strategy]);
        return mDeviceForStrategy[strategy];
    }

    switch (strategy) {
    case STRATEGY_DTMF:
        if (!isInCall()) {
            // when off call, DTMF strategy follows the same rules as MEDIA strategy
            device = getDeviceForStrategy(STRATEGY_MEDIA, false);
            break;
        }
        // when in call, DTMF and PHONE strategies follow the same rules
        // FALL THROUGH

    case STRATEGY_PHONE:
        // for phone strategy, we first consider the forced use and then the available devices by order
        // of priority
        switch (mForceUse[AudioSystem::FOR_COMMUNICATION]) {
        case AudioSystem::FORCE_BT_SCO:
            if (!isInCall() || strategy != STRATEGY_DTMF) {
                device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT;
                if (device) break;
            }
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
            if (device) break;
            // if SCO device is requested but no SCO device is available, fall back to default case
            // FALL THROUGH

        default:    // FORCE_NONE
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET;
            if (device) break;
#ifdef WITH_A2DP
            // when not in a phone call, phone strategy should route STREAM_VOICE_CALL to A2DP
            if (!isInCall() && !mA2dpSuspended) {
                device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP;
                if (device) break;
                device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES;
                if (device) break;
            }
#endif
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_DGTL_DOCK_HEADSET;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_AUX_DIGITAL;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_ANLG_DOCK_HEADSET;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_EARPIECE;
            if (device == 0) {
                LOGE("getDeviceForStrategy() earpiece device not found");
            }
            break;

        case AudioSystem::FORCE_SPEAKER:
#ifdef WITH_A2DP
            // when not in a phone call, phone strategy should route STREAM_VOICE_CALL to
            // A2DP speaker when forcing to speaker output
            if (!isInCall() && !mA2dpSuspended) {
                device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER;
                if (device) break;
            }
#endif
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_DGTL_DOCK_HEADSET;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_AUX_DIGITAL;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_ANLG_DOCK_HEADSET;
            if (device) break;
            device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_SPEAKER;
            if (device == 0) {
                LOGE("getDeviceForStrategy() speaker device not found");
            }
            break;
        }
    break;

#ifndef ANDROID_DEFAULT_CODE
    case STRATEGY_PROPRIETARY:{
       uint32_t device2 = 0;
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET;
        }
        if(device2 ==0){
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_AUX_DIGITAL;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_SPEAKER;
        }
        // device is DEVICE_OUT_SPEAKER if we come from case STRATEGY_SONIFICATION, 0 otherwise
        device |= device2;
        if (device == 0) {
            LOGE("getDeviceForStrategy() speaker device not found");
        }
        break;
    }
#endif
    case STRATEGY_SONIFICATION:

        // If incall, just select the STRATEGY_PHONE device: The rest of the behavior is handled by
        // handleIncallSonification().
        if (isInCall()) {
            device = getDeviceForStrategy(STRATEGY_PHONE, false);
            break;
        }
        // FALL THROUGH

    case STRATEGY_ENFORCED_AUDIBLE:
        // strategy STRATEGY_ENFORCED_AUDIBLE uses same routing policy as STRATEGY_SONIFICATION
        // except when in call where it doesn't default to STRATEGY_PHONE behavior

        device = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_SPEAKER;
        if (device == 0) {
            LOGE("getDeviceForStrategy() speaker device not found");
        }
        // The second device used for sonification is the same as the device used by media strategy
        // FALL THROUGH

    case STRATEGY_MEDIA: {
        uint32_t device2 = 0;
       #ifdef WITH_A2DP
        if ((mA2dpOutput != 0) && !mA2dpSuspended &&
                (strategy == STRATEGY_MEDIA || a2dpUsedForSonification())) {
                   if (device2 == 0) {
                       device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP;
                   }
                   if (device2 == 0) {
                       device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES;
                   }
                   if (device2 == 0) {
                       device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER;
                   }
               }
       #endif
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_DGTL_DOCK_HEADSET;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_AUX_DIGITAL;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_ANLG_DOCK_HEADSET;
        }
        if (device2 == 0) {
            device2 = mAvailableOutputDevices & AudioSystem::DEVICE_OUT_SPEAKER;
        }

        // device is DEVICE_OUT_SPEAKER if we come from case STRATEGY_SONIFICATION or
        // STRATEGY_ENFORCED_AUDIBLE, 0 otherwise
        device |= device2;
        if (device == 0) {
            LOGE("getDeviceForStrategy() speaker device not found");
        }
        } break;

    default:
        LOGW("getDeviceForStrategy() unknown strategy: %d", strategy);
        break;
    }

    LOGV("getDeviceForStrategy() strategy %d, device %x", strategy, device);
    return device;
}

void AudioYusuPolicyManager::updateDeviceForStrategy()
{
    for (int i = 0; i < NUM_STRATEGIES; i++) {
        mDeviceForStrategy[i] = getDeviceForStrategy((routing_strategy)i, false);
    }
}

void AudioYusuPolicyManager::setOutputDevice(audio_io_handle_t output, uint32_t device, bool force, int delayMs)
{
    LOGV("setOutputDevice() output %d device %x delayMs %d", output, device, delayMs);
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);


    if (outputDesc->isDuplicated()) {
        setOutputDevice(outputDesc->mOutput1->mId, device, force, delayMs);
        setOutputDevice(outputDesc->mOutput2->mId, device, force, delayMs);
        LOGD("setOutputDevice outputDesc->isDuplicated()");
        return;
    }
#ifdef WITH_A2DP
    // filter devices according to output selected
    if (output == mA2dpOutput) {
        device &= AudioSystem::DEVICE_OUT_ALL_A2DP;
    } else {
        device &= ~AudioSystem::DEVICE_OUT_ALL_A2DP;
    }
#endif

    uint32_t prevDevice = (uint32_t)outputDesc->device();
    LOGD("setOutputDevice() output %d prevDevice %x device %x force = %d", output, prevDevice, device,force);
    // Do not change the routing if:
    //  - the requestede device is 0
    //  - the requested device is the same as current device and force is not specified.
    // Doing this check here allows the caller to call setOutputDevice() without conditions
#ifndef ANDROID_DEFAULT_CODE
    // Scenario: mATV playing with headset, then start to record. The record sound play via speaker and headset.
    // (both speaker+headset are select)
    // Ex: For mATV(analog path) case, after record sound stop. the current device is 0.
    //     (For mATV analog path, no AudioTrack is created.)
    //     During the record sound playing period, the device is 2. It will do setStrategyMute function.
    //     But record sound stop, the device is 0. No one will do Un-mute function again.
    if (( (device == 0 && AudioSystem::popCount(prevDevice) != 2 ) || device == prevDevice) && !force) {
        LOGD("setOutputDevice() setting same device %x or null device for output %d", device, output);
        return;
    }
#else
    if ((device == 0 || device == prevDevice) && !force) {
        LOGV("setOutputDevice() setting same device %x or null device for output %d", device, output);
        return;
    }
#endif

    outputDesc->mDevice = device;
    // mute media streams if both speaker and headset are selected
    if (output == mHardwareOutput && AudioSystem::popCount(device) == 2) {
        setStrategyMute(STRATEGY_MEDIA, true, output);
#ifndef ANDROID_DEFAULT_CODE
        //<---weiguo li  mute STRATEGY_PROPRIETARY
        setStrategyMute(STRATEGY_PROPRIETARY, true, output);
        //---->
        // when type is boot, should not sleep. boot device whend headphone is pluged in for example,
        // sound output from speaker and headset,  and when headsebserver get up, it will come here,
        // if you sleep, bootsoud will soud off and on.
        // wait for the PCM output buffers to empty before proceeding with the rest of the command
        if(device != 0 && device != prevDevice)
        {
            usleep(outputDesc->mLatency*2*1000);
        }
        //<-- Weiguo li  force hardware to use speaker
        AudioParameter outputCmd = AudioParameter();
        uint32_t value = (uint32_t)AudioSystem::FOR_MEDIA<<16 |AudioSystem::FORCE_SPEAKER;
        mForceUse[AudioSystem::FOR_MEDIA] = AudioSystem::FORCE_SPEAKER;
        outputCmd.addInt(String8(keyAddtForceuseNormal),value);
        mpClientInterface->setParameters(0, outputCmd.toString());
        //-->
#else
        // wait for the PCM output buffers to empty before proceeding with the rest of the command
        // FIXME: increased delay due to larger buffers used for low power audio mode.
        // remove when low power audio is controlled by policy manager.
        usleep(outputDesc->mLatency*8*1000);
#endif
    }

    // do the routing
    AudioParameter param = AudioParameter();
    param.addInt(String8(AudioParameter::keyRouting), (int)device);
    mpClientInterface->setParameters(mHardwareOutput, param.toString(), delayMs);
    // update stream volumes according to new device
    applyStreamVolumes(output, device, delayMs);

    // if changing from a combined headset + speaker route, unmute media streams
    if (output == mHardwareOutput && AudioSystem::popCount(prevDevice) == 2) {
#ifndef ANDROID_DEFAULT_CODE
        //<---weiguo li force hardware to use None
        AudioParameter outputCmd = AudioParameter();
        uint32_t value = (uint32_t)AudioSystem::FOR_MEDIA<<16 |AudioSystem::FORCE_NONE;
        mForceUse[AudioSystem::FOR_MEDIA] = AudioSystem::FORCE_NONE;
        outputCmd.addInt(String8(keyAddtForceuseNormal),value);
        //delayms to let camera sound to play out completely  through speaker.
        mpClientInterface->setParameters(0, outputCmd.toString(),delayMs);
        //---->
#endif
        setStrategyMute(STRATEGY_MEDIA, false, output, delayMs);
#ifndef ANDROID_DEFAULT_CODE
        //<---weiguo li  unmute STRATEGY_PROPRIETARY
        setStrategyMute(STRATEGY_PROPRIETARY, false, output,delayMs);
        //---->
#endif
    }
}

uint32_t AudioYusuPolicyManager::getDeviceForInputSource(int inputSource)
{
    uint32_t device;

    switch(inputSource) {
    case AUDIO_SOURCE_DEFAULT:
    case AUDIO_SOURCE_MIC:
    case AUDIO_SOURCE_VOICE_RECOGNITION:
    case AUDIO_SOURCE_VOICE_COMMUNICATION:
        if (mForceUse[AudioSystem::FOR_RECORD] == AudioSystem::FORCE_BT_SCO &&
            mAvailableInputDevices & AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
            device = AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        } else if (mAvailableInputDevices & AudioSystem::DEVICE_IN_WIRED_HEADSET) {
            device = AudioSystem::DEVICE_IN_WIRED_HEADSET;
        } else {
            device = AudioSystem::DEVICE_IN_BUILTIN_MIC;
        }
        break;
    case AUDIO_SOURCE_CAMCORDER:
        if (hasBackMicrophone()) {
#ifndef ANDROID_DEFAULT_CODE
            device = AudioSystem::DEVICE_IN_BACK_MIC|AudioSystem::DEVICE_IN_AMBIENT;;
#else
            device = AudioSystem::DEVICE_IN_BACK_MIC;
#endif
        } else {
            device = AudioSystem::DEVICE_IN_BUILTIN_MIC;
        }
        break;
    case AUDIO_SOURCE_VOICE_UPLINK:
    case AUDIO_SOURCE_VOICE_DOWNLINK:
    case AUDIO_SOURCE_VOICE_CALL:
        device = AudioSystem::DEVICE_IN_VOICE_CALL;
        break;
#ifndef ANDROID_DEFAULT_CODE
    // Don't care the input source set by application. Always check the I2S/Line-in compile option for device.
    case AUDIO_SOURCE_MATV :
#ifdef MATV_AUDIO_LINEIN_PATH
        device = AudioSystem::DEVICE_IN_FM;
#else
        device = AudioSystem::DEVICE_IN_AUX_DIGITAL;
#endif
        break;
    case AUDIO_SOURCE_FM:
#ifdef FM_DIGITAL_INPUT
        device = AudioSystem::DEVICE_IN_AUX_DIGITAL;
#else
        device = AudioSystem::DEVICE_IN_FM;
#endif
        break;
#endif
    default:
        LOGW("getInput() invalid input source %d use MIC as defualt", inputSource);
#ifndef ANDROID_DEFAULT_CODE
        device = AudioSystem::DEVICE_IN_BUILTIN_MIC;
#else
        device = 0;
#endif
        break;
    }
    LOGV("getDeviceForInputSource()input source %d, device %08x", inputSource, device);
    return device;
}

audio_io_handle_t AudioYusuPolicyManager::getActiveInput()
{
    for (size_t i = 0; i < mInputs.size(); i++) {
        if (mInputs.valueAt(i)->mRefCount > 0) {
            return mInputs.keyAt(i);
        }
    }
    return 0;
}

#ifndef ANDROID_DEFAULT_CODE
uint32 AudioYusuPolicyManager::GetMultiplier(uint32 volume,uint32 dBAttenTotal)
{
   uint32 fGainMultiplier;
   if (dBAttenTotal==0)
   {
      fGainMultiplier = volume;
   }
   else if (dBAttenTotal>200)
   {
       fGainMultiplier = 0;
   }
   else
   {
      uint32 index = GetMapVolumeIndex(volume);
      LOGV("GetMultiplierindex index = %d dBAttenTotal = %d",index, dBAttenTotal);
      index += dBAttenTotal;
      if(index > 200){
          fGainMultiplier =0;
      }
      else{
          fGainMultiplier = (uint32)GainMap[index-1];
      }
   }
    return fGainMultiplier;
}

int AudioYusuPolicyManager::GetOutputDevice()
{
    // check if force use exist , get output device for certain mode
    int OutputDevice = 0;
    if(mPhoneState == AudioSystem::MODE_NORMAL ){
        if( mForceUse[AudioSystem::FOR_MEDIA] != 0){
            OutputDevice = Audio_Match_Force_device(mForceUse[AudioSystem::FOR_MEDIA]);
        }
        else {
            OutputDevice = Audio_Find_Normal_Output_Device(mAvailableOutputDevices);
        }
    }
    if(mPhoneState == AudioSystem::MODE_RINGTONE){
        if( mForceUse[AudioSystem::FOR_MEDIA] != 0){
            OutputDevice = Audio_Match_Force_device(mForceUse[AudioSystem::FOR_MEDIA]);
        }
        else {
            OutputDevice = Audio_Find_Ringtone_Output_Device(mAvailableOutputDevices);
        }
    }
    else if(mPhoneState == AudioSystem::MODE_IN_CALL){
        //LOGV("mForceUse[AudioSystem::FOR_COMMUNICATION] = %d",mForceUse[AudioSystem::FOR_COMMUNICATION]);
        if( mForceUse[AudioSystem::FOR_COMMUNICATION] != 0){
            OutputDevice = Audio_Match_Force_device(mForceUse[AudioSystem::FOR_COMMUNICATION]);
        }
        else {
            OutputDevice = Audio_Find_Incall_Output_Device(mAvailableOutputDevices);
        }
    }
    else if(mPhoneState == AudioSystem::MODE_IN_COMMUNICATION){
        if( mForceUse[AudioSystem::FOR_COMMUNICATION] != 0){
            OutputDevice = Audio_Match_Force_device(mForceUse[AudioSystem::FOR_COMMUNICATION]);
        }
        else {
            OutputDevice = Audio_Find_Communcation_Output_Device(mAvailableOutputDevices);
        }
    }
    return OutputDevice;
}

#ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
float AudioYusuPolicyManager::ComputegainTableVolume(int stream , float &volInt)
{
    // get output device
    int OutputDevice = GetOutputDevice();
    int fVolume = 0.0;
    int volmax=0 , volmin =0,volumeindex =0;

    switch(stream){
        case AudioSystem::VOICE_CALL :{
            STREAM_VOICE_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Voice_Gain_table;
            uint32 gainlevel = VOICE_GAIN_TABLE_LEVEL;
            if(mPhoneState == AudioSystem::MODE_IN_COMMUNICATION){
                if(OutputDevice == AudioSystem::DEVICE_OUT_EARPIECE){
                    volmax =(int)(pStream->Voice_Gain[SIP_VOICE_SPEAKER][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Voice_Gain[SIP_VOICE_SPEAKER][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Voice_Gain[SIP_VOICE_SPEAKER],
                        volmin,volmax,volInt,gainlevel);
                }
                if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                    volmax =(int)(pStream->Voice_Gain[SIP_VOICE_SPEAKER][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Voice_Gain[SIP_VOICE_SPEAKER][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Voice_Gain[SIP_VOICE_SPEAKER],
                        volmin,volmax,volInt,gainlevel);
                }
                else{
                    volmax =(int)(pStream->Voice_Gain[SIP_VOICE_HEADSET][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Voice_Gain[SIP_VOICE_HEADSET][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Voice_Gain[SIP_VOICE_HEADSET],
                        volmin,volmax,volInt,gainlevel);
                }
            }
            else{
                if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                    volmax =(int)(pStream->Voice_Gain[VOICE_SPEAKER][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Voice_Gain[VOICE_SPEAKER][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Voice_Gain[VOICE_SPEAKER],
                        volmin,volmax,volInt,gainlevel);
                }
                else{
                    volmax =(int)(pStream->Voice_Gain[VOICE_HEADSET][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Voice_Gain[VOICE_HEADSET][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Voice_Gain[VOICE_HEADSET],
                        volmin,volmax,volInt,gainlevel);
                }
            }
            break;
        }
        case AudioSystem::SYSTEM :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.System_Gain_table;
            uint32 gainlevel = SYSTEM_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::RING :{
            STREAM_RING_GAIN_CONTROL_STRUCT* pStream = &Audio_gaintable_Custom_Volume.Ring_Gain_table;
            uint32 gainlevel = RING_GAIN_TABLE_LEVEL;
            if(mPhoneState == AudioSystem::MODE_RINGTONE){
                if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER &&mPhoneState == AudioSystem::MODE_RINGTONE){
                    volmax =(int)(pStream->Ring_Stream_Gain[RING_MODE_SPEAKER][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Ring_Stream_Gain[RING_MODE_SPEAKER][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Ring_Stream_Gain[RING_MODE_SPEAKER],
                         volmin,volmax,volInt,gainlevel);
                }
                else if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER &&mPhoneState == AudioSystem::MODE_RINGTONE){
                    volmax =(int)(pStream->Ring_Stream_Gain[RING_MODE_HEADSET][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Ring_Stream_Gain[RING_MODE_HEADSET][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Ring_Stream_Gain[RING_MODE_HEADSET],
                         volmin,volmax,volInt,gainlevel);
                }
            }
            else{
                if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                    volmax =(int)(pStream->Ring_Stream_Gain[RING_HEADSET][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Ring_Stream_Gain[RING_HEADSET][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Ring_Stream_Gain[RING_HEADSET],
                        volmin,volmax,volInt,gainlevel);
                }
                else{
                    volmax =(int)(pStream->Ring_Stream_Gain[RING_SPEAKER][gainlevel].u8Gain_digital);
                    volmin = (int)(pStream->Ring_Stream_Gain[RING_SPEAKER][0].u8Gain_digital);
                    CheckMaxMinValue (volmin,volmax);
                    fVolume = MapGaintableVoltoCustomVol(pStream->Ring_Stream_Gain[RING_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
                }
            }
            break;
        }
        case AudioSystem::MUSIC :{
            STREAM_GAIN_CONTROL_STRUCT* pStream = &Audio_gaintable_Custom_Volume.Music_Gain_table;
            uint32 gainlevel = MUSIC_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                LOGD("ComputegainTableVolume STREAM_SPEAKER");
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                LOGD("ComputegainTableVolume STREAM_HEADSET");
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::ALARM :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Alarm_Gain_table;
            uint32 gainlevel = ALARM_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::NOTIFICATION :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Notification_Gain_table;
            uint32 gainlevel = NOTIFICATION_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::BLUETOOTH_SCO :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Bluetooth_sco_Gain_table;
            uint32 gainlevel = BLUETOOTHSCO_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::ENFORCED_AUDIBLE :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.EnforceAudible_table;
            uint32 gainlevel = ENFORCEAUDIBLE_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::DTMF :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Dtmf_Gain_table;
            uint32 gainlevel = DTMF_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::TTS :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Tts_Gain_table;
            uint32 gainlevel = TTS_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::FM :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Fm_Gain_table;
            uint32 gainlevel = FM_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        case AudioSystem::MATV :{
            STREAM_GAIN_CONTROL_STRUCT *pStream = &Audio_gaintable_Custom_Volume.Matv_Gain_table;
            uint32 gainlevel = MATV_GAIN_TABLE_LEVEL;
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =(int)(pStream->Stream_Gain[STREAM_SPEAKER][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_SPEAKER][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_SPEAKER],
                    volmin,volmax,volInt,gainlevel);
            }
            else{
                volmax =(int)(pStream->Stream_Gain[STREAM_HEADSET][gainlevel].u8Gain_digital);
                volmin = (int)(pStream->Stream_Gain[STREAM_HEADSET][0].u8Gain_digital);
                CheckMaxMinValue (volmin,volmax);
                fVolume = MapGaintableVoltoCustomVol(pStream->Stream_Gain[STREAM_HEADSET],
                    volmin,volmax,volInt,gainlevel);
            }
            break;
        }
        default:
            LOGE("NO stream can map gain");
            break;
    }
    return fVolume;
}
#endif
#endif //#ifndef ANDROID_DEFAULT_CODE

AudioYusuPolicyManager::device_category AudioYusuPolicyManager::getDeviceCategory(uint32_t device)
{
    if (device == 0) {
        // this happens when forcing a route update and no track is active on an output.
        // In this case the returned category is not important.
        return DEVICE_CATEGORY_SPEAKER;
    }

    if (AudioSystem::popCount(device) > 1) {
        // Multiple device selection is either:
        //  - speaker + one other device: give priority to speaker in this case.
        //  - one A2DP device + another device: happens with duplicated output. In this case
        // retain the device on the A2DP output as the other must not correspond to an active
        // selection if not the speaker.
        if (device & AUDIO_DEVICE_OUT_SPEAKER)
            return DEVICE_CATEGORY_SPEAKER;

        device &= AUDIO_DEVICE_OUT_ALL_A2DP;
    }

    LOGW_IF(AudioSystem::popCount(device) != 1,
            "getDeviceCategory() invalid device combination: %08x",
            device);

    switch(device) {
        case AUDIO_DEVICE_OUT_EARPIECE:
            return DEVICE_CATEGORY_EARPIECE;
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
            return DEVICE_CATEGORY_HEADSET;
        case AUDIO_DEVICE_OUT_SPEAKER:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
        default:
            return DEVICE_CATEGORY_SPEAKER;
    }
}

float AudioYusuPolicyManager::volIndexToAmpl(uint32_t device, const StreamDescriptor& streamDesc,
        int indexInUi)
{
    device_category deviceCategory = getDeviceCategory(device);
    const VolumeCurvePoint *curve = streamDesc.mVolumeCurve[deviceCategory];

    // the volume index in the UI is relative to the min and max volume indices for this stream type
    int nbSteps = 1 + curve[VOLMAX].mIndex -
            curve[VOLMIN].mIndex;
    int volIdx = (nbSteps * (indexInUi - streamDesc.mIndexMin)) /
            (streamDesc.mIndexMax - streamDesc.mIndexMin);

    // find what part of the curve this index volume belongs to, or if it's out of bounds
    int segment = 0;
    if (volIdx < curve[VOLMIN].mIndex) {         // out of bounds
        return 0.0f;
    } else if (volIdx < curve[VOLKNEE1].mIndex) {
        segment = 0;
    } else if (volIdx < curve[VOLKNEE2].mIndex) {
        segment = 1;
    } else if (volIdx <= curve[VOLMAX].mIndex) {
        segment = 2;
    } else {                                                               // out of bounds
        return 1.0f;
    }

    // linear interpolation in the attenuation table in dB
    float decibels = curve[segment].mDBAttenuation +
            ((float)(volIdx - curve[segment].mIndex)) *
                ( (curve[segment+1].mDBAttenuation -
                        curve[segment].mDBAttenuation) /
                    ((float)(curve[segment+1].mIndex -
                            curve[segment].mIndex)) );

    float amplification = exp( decibels * 0.115129f); // exp( dB * ln(10) / 20 )

    LOGV("VOLUME vol index=[%d %d %d], dB=[%.1f %.1f %.1f] ampl=%.5f",
            curve[segment].mIndex, volIdx,
            curve[segment+1].mIndex,
            curve[segment].mDBAttenuation,
            decibels,
            curve[segment+1].mDBAttenuation,
            amplification);

    return amplification;
}

const AudioYusuPolicyManager::VolumeCurvePoint
    AudioYusuPolicyManager::sDefaultVolumeCurve[AudioYusuPolicyManager::VOLCNT] = {
    {1, -63.0f}, {88, -42.0f}, {172, -21.0f}, {256, 0.0f}
};

const AudioYusuPolicyManager::VolumeCurvePoint
    AudioYusuPolicyManager::sDefaultMediaVolumeCurve[AudioYusuPolicyManager::VOLCNT] = {
    {1, -63.0f}, {88, -42.0f}, {172, -21.0f}, {256, 0.0f}
};

const AudioYusuPolicyManager::VolumeCurvePoint
    AudioYusuPolicyManager::sSpeakerMediaVolumeCurve[AudioYusuPolicyManager::VOLCNT] = {
    {1, -63.0f}, {88, -42.0f}, {172, -21.0f}, {256, 0.0f}
};

const AudioYusuPolicyManager::VolumeCurvePoint
    AudioYusuPolicyManager::sSpeakerSonificationVolumeCurve[AudioYusuPolicyManager::VOLCNT] = {
    {1, -63.0f}, {176, -20.0f}, {216, -10.0f}, {256, 0.0f}
};


const AudioYusuPolicyManager::VolumeCurvePoint
            *AudioYusuPolicyManager::sVolumeProfiles[AudioYusuPolicyManager::NUM_STRATEGIES]
                                                   [AudioYusuPolicyManager::DEVICE_CATEGORY_CNT] = {
    { // STRATEGY_MEDIA
        sDefaultMediaVolumeCurve, // DEVICE_CATEGORY_HEADSET
        sSpeakerMediaVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultMediaVolumeCurve  // DEVICE_CATEGORY_EARPIECE
    },
    { // STRATEGY_PHONE
        sDefaultVolumeCurve, // DEVICE_CATEGORY_HEADSET
        sDefaultVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve  // DEVICE_CATEGORY_EARPIECE
    },
    { // STRATEGY_SONIFICATION
        sDefaultVolumeCurve, // DEVICE_CATEGORY_HEADSET
        sSpeakerSonificationVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve  // DEVICE_CATEGORY_EARPIECE
    },
    {  // STRATEGY_DTMF
        sDefaultVolumeCurve, // DEVICE_CATEGORY_HEADSET
        sDefaultVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve  // DEVICE_CATEGORY_EARPIECE
    },
    { // STRATEGY_ENFORCED_AUDIBLE
        sDefaultVolumeCurve, // DEVICE_CATEGORY_HEADSET
        sSpeakerSonificationVolumeCurve, // DEVICE_CATEGORY_SPEAKER
        sDefaultVolumeCurve  // DEVICE_CATEGORY_EARPIECE
    },
};

void AudioYusuPolicyManager::initializeVolumeCurves()
{
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        for (int j = 0; j < DEVICE_CATEGORY_CNT; j++) {
            mStreams[i].mVolumeCurve[j] =
                    sVolumeProfiles[getStrategy((AudioSystem::stream_type)i)][j];
        }
    }
}

#ifndef ANDROID_DEFAULT_CODE
float AudioYusuPolicyManager::computeCustomVolume(int stream, float &volInt)
{
    // check if force use exist , get output device for certain mode
    int OutputDevice = GetOutputDevice();

    // compute custom volume
    float volume =0.0;
    int volmax=0 , volmin =0,volumeindex =0;

    switch(stream){
        case AudioSystem::RING:
	case AudioSystem::ALARM:
	case AudioSystem::NOTIFICATION:
            volmax =Audio_Custom_Volume.audiovolume_ring[VOL_HANDFREE][Custom_Voume_Step];
            volmin = Audio_Custom_Volume.audiovolume_ring[VOL_HANDFREE][0];
            volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_ring[VOL_HANDFREE],volmin,volmax,volInt,stream);
            break;
        case AudioSystem::MUSIC:
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE],volmin,volmax,volInt,stream);
            }
            else{
                volmax =Audio_Custom_Volume.audiovolume_media[VOL_HEADSET][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_media[VOL_HEADSET][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_media[VOL_HEADSET],volmin,volmax,volInt,stream);
            }
            break;
        case AudioSystem::FM:
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER || mFmForceSpeakerState ){
                volmax =Audio_Custom_Volume.audiovolume_fmr[VOL_HANDFREE][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_fmr[VOL_HANDFREE][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_fmr[VOL_HANDFREE],volmin,volmax,volInt,stream);
            }
            else{
                volmax =Audio_Custom_Volume.audiovolume_fmr[VOL_HEADSET][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_fmr[VOL_HEADSET][0];
                volume= MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_fmr[VOL_HEADSET],volmin,volmax,volInt,stream);
            }
            break;
        case AudioSystem::MATV:
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =Audio_Custom_Volume.audiovolume_matv[VOL_HANDFREE][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_matv[VOL_HANDFREE][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_matv[VOL_HANDFREE],volmin,volmax,volInt,stream);
            }
            else{
                volmax =Audio_Custom_Volume.audiovolume_matv[VOL_HEADSET][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_matv[VOL_HEADSET][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_matv[VOL_HEADSET],volmin,volmax,volInt,stream);
            }
            break;
        case AudioSystem::VOICE_CALL:
           if(mPhoneState == AudioSystem::MODE_IN_COMMUNICATION){
               if(OutputDevice == AudioSystem::DEVICE_OUT_EARPIECE){
                   LOGD("MODE_IN_COMMUNICATION AudioSystem::VOICE_CALL DEVICE_OUT_EARPIECE");
                   volmax =Audio_Custom_Volume.audiovolume_key[VOL_NORMAL][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_key[VOL_NORMAL][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_key[VOL_NORMAL],volmin,volmax,volInt);
               }
               else if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                   LOGD("MODE_IN_COMMUNICATION AudioSystem::VOICE_CALL DEVICE_OUT_SPEAKER");
                   volmax =Audio_Custom_Volume.audiovolume_key[VOL_HANDFREE][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_key[VOL_HANDFREE][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_key[VOL_HANDFREE],volmin,volmax,volInt);
               }
               else{
                   LOGD("MODE_IN_COMMUNICATION AudioSystem::VOICE_CALL Headset");
                   volmax =Audio_Custom_Volume.audiovolume_key[VOL_HEADSET][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_key[VOL_HEADSET][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_key[VOL_HEADSET],volmin,volmax,volInt);
               }
           }
           else{
               // this mode is actually in call mode
               if(OutputDevice == AudioSystem::DEVICE_OUT_EARPIECE){
                   LOGD("AudioSystem::VOICE_CALL DEVICE_OUT_EARPIECE");
                   volmax =Audio_Custom_Volume.audiovolume_sph[VOL_NORMAL][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_sph[VOL_NORMAL][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_sph[VOL_NORMAL],0,VOICE_VOLUME_MAX,volInt);
               }
               else if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                   LOGD("AudioSystem::VOICE_CALL DEVICE_OUT_SPEAKER");
                   volmax =Audio_Custom_Volume.audiovolume_sph[VOL_HANDFREE][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_sph[VOL_HANDFREE][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_sph[VOL_HANDFREE],0,VOICE_VOLUME_MAX,volInt);
               }
               else{
                   LOGD("AudioSystem::VOICE_CALL Headset");
                   volmax =Audio_Custom_Volume.audiovolume_sph[VOL_HEADSET][Custom_Voume_Step];
                   volmin = Audio_Custom_Volume.audiovolume_sph[VOL_HEADSET][0];
                   volume = MapVoiceVoltoCustomVol(Audio_Custom_Volume.audiovolume_sph[VOL_HEADSET],0,VOICE_VOLUME_MAX,volInt);
               }
           }
           break;
        default:
            if(OutputDevice == AudioSystem::DEVICE_OUT_SPEAKER){
                volmax =Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_media[VOL_HANDFREE],volmin,volmax,volInt,stream);
            }
            else{
                volmax =Audio_Custom_Volume.audiovolume_media[VOL_HEADSET][Custom_Voume_Step];
                volmin = Audio_Custom_Volume.audiovolume_media[VOL_HEADSET][0];
                volume = MapVoltoCustomVol(Audio_Custom_Volume.audiovolume_media[VOL_HEADSET],volmin,volmax,volInt,stream);
            }
            break;
    }
    //LOGV("stream = %d after computeCustomVolume , volInt = %d volume = %f",stream,volInt,volume);
    return volume;
}

float AudioYusuPolicyManager::computeCustomVoiceVolume(int stream, int index, audio_io_handle_t output, uint32_t device)
{
    float volume = 1.0;
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    StreamDescriptor &streamDesc = mStreams[stream];

    if (device == 0) {
        device = outputDesc->device();
    }

    float volInt = (volume_Mapping_Step * (index - streamDesc.mIndexMin)) / (streamDesc.mIndexMax - streamDesc.mIndexMin);
    //LOGD("computeCustomVoiceVolume stream = %d index = %d volInt = %f",stream,index,volInt);
    #ifndef MTK_AUDIO_GAIN_TABLE_SUPPORT
    volume = computeCustomVolume(stream,volInt);
    #else
    // do table gain volume mapping....
    volume  = ComputegainTableVolume(stream,volInt);
    #endif
    LOGD("computeCustomVoiceVolume volume = %f volInt = %f",volume,volInt);
    volInt =android::AudioSystem::linearToLog(volInt);
    volume = volInt;
    return volume;
}
#endif //#ifndef ANDROID_DEFAULT_CODE

float AudioYusuPolicyManager::computeVolume(int stream, int index, audio_io_handle_t output, uint32_t device)
{
    float volume = 1.0;
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    StreamDescriptor &streamDesc = mStreams[stream];

    if (device == 0) {
        device = outputDesc->device();
    }
    // if volume is not 0 (not muted), force media volume to max on digital output
    if (stream == AudioSystem::MUSIC &&
        index != mStreams[stream].mIndexMin &&
        (device == AudioSystem::DEVICE_OUT_AUX_DIGITAL ||
        device == AudioSystem::DEVICE_OUT_DGTL_DOCK_HEADSET)) {
        return 1.0;
    }
#ifndef ANDROID_DEFAULT_CODE
    float volInt = (volume_Mapping_Step * (index - streamDesc.mIndexMin)) / (streamDesc.mIndexMax - streamDesc.mIndexMin);
    // here volint is 0~100
    //LOGD("computeVolume stream = %d index = %d volInt = %f",stream,index,volInt);
    #ifndef MTK_AUDIO_GAIN_TABLE_SUPPORT
    if(output == mHardwareOutput){
        volume = computeCustomVolume(stream,volInt);
    }
    #else
    // do table gain volume mapping....
    if(output == mHardwareOutput){
        volume  = ComputegainTableVolume(stream,volInt);
    }
    #endif
    volume = android::AudioSystem::linearToLog(volInt);
    if(stream == AudioSystem::BOOT)
    {
        volume = BOOT_ANIMATION_VOLUME;
        LOGD("boot animation vol= %f",volume);
    }
#else
    volume = volIndexToAmpl(device, streamDesc, index);
#endif//#ifndef ANDROID_DEFAULT_CODE

    // if a headset is connected, apply the following rules to ring tones and notifications
    // to avoid sound level bursts in user's ears:
    // - always attenuate ring tones and notifications volume by 6dB
    // - if music is playing, always limit the volume to current music volume,
    // with a minimum threshold at -36dB so that notification is always perceived.
    if ((device &
        (AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP |
        AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
        AudioSystem::DEVICE_OUT_WIRED_HEADSET |
        AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)) &&
        ((getStrategy((AudioSystem::stream_type)stream) == STRATEGY_SONIFICATION) ||
#ifndef ANDROID_DEFAULT_CODE
         (stream == AudioSystem::BOOT) ||
#endif
         (stream == AudioSystem::SYSTEM)) &&
        streamDesc.mCanBeMuted) {
        volume *= SONIFICATION_HEADSET_VOLUME_FACTOR;
        // when the phone is ringing we must consider that music could have been paused just before
        // by the music application and behave as if music was active if the last music track was
        // just stopped
        if (outputDesc->mRefCount[AudioSystem::MUSIC] || mLimitRingtoneVolume) {
            float musicVol = computeVolume(AudioSystem::MUSIC, mStreams[AudioSystem::MUSIC].mIndexCur, output, device);
            float minVol = (musicVol > SONIFICATION_HEADSET_VOLUME_MIN) ? musicVol : SONIFICATION_HEADSET_VOLUME_MIN;
            if (volume > minVol) {
                volume = minVol;
                LOGV("computeVolume limiting volume to %f musicVol %f", minVol, musicVol);
            }
        }
    }

    return volume;
}

#ifndef ANDROID_DEFAULT_CODE
int AudioYusuPolicyManager::GetMasterVolumeStream(AudioOutputDescriptor *outputDesc)
{
    if(outputDesc->mRefCount[AudioSystem::VOICE_CALL]&&outputDesc->mMuteCount[AudioSystem::VOICE_CALL]==0)
        return AudioSystem::VOICE_CALL;
    else if(outputDesc->mRefCount[AudioSystem::BLUETOOTH_SCO]&&outputDesc->mMuteCount[AudioSystem::BLUETOOTH_SCO]==0)
         return AudioSystem::BLUETOOTH_SCO;
    else if(outputDesc->mRefCount[AudioSystem::RING]&&outputDesc->mMuteCount[AudioSystem::RING]==0)
         return AudioSystem::RING;
     else if(outputDesc->mRefCount[AudioSystem::NOTIFICATION]&&outputDesc->mMuteCount[AudioSystem::NOTIFICATION]==0)
         return AudioSystem::NOTIFICATION;
     else if(outputDesc->mRefCount[AudioSystem::ALARM]&&outputDesc->mMuteCount[AudioSystem::ALARM]==0)
         return AudioSystem::ALARM;
     else if(outputDesc->mRefCount[AudioSystem::MATV]&&outputDesc->mMuteCount[AudioSystem::MATV]==0)
         return AudioSystem::MATV;
     else if(outputDesc->mRefCount[AudioSystem::MUSIC]&&outputDesc->mMuteCount[AudioSystem::MUSIC]==0)
         return AudioSystem::MUSIC;
     else if(outputDesc->mRefCount[AudioSystem::FM]&&outputDesc->mMuteCount[AudioSystem::FM]== 0)
         return AudioSystem::FM;
     else
         return AudioSystem::SYSTEM;  // SYSTEM should be default analog value value
}

bool AudioYusuPolicyManager::AdjustMasterVolumeIncallState(AudioSystem::stream_type stream)
{
     if(stream == AudioSystem::VOICE_CALL || stream == AudioSystem::BLUETOOTH_SCO ){
           return true;
     }
     else{
         return false;
     }
}


bool AudioYusuPolicyManager::AdjustMasterVolumeNormalState(AudioSystem::stream_type stream)
{
     if(stream == AudioSystem::MUSIC || stream == AudioSystem::ALARM ||stream == AudioSystem::NOTIFICATION ||
       stream == AudioSystem::FM || stream == AudioSystem::MATV||stream == AudioSystem::RING||stream == AudioSystem::SYSTEM){
           return true;
     }
     else{
         return false;
     }
}

bool AudioYusuPolicyManager::AdjustMasterVolumeRingState(AudioSystem::stream_type stream)
{
     if(stream == AudioSystem::RING){
           return true;
     }
     else{
         return false;
     }
}


bool AudioYusuPolicyManager::StreamMatchPhoneState(AudioSystem::stream_type stream){
    if(mPhoneState == AudioSystem::MODE_RINGTONE && AdjustMasterVolumeRingState(stream) ){
        return true;
    }
    //  allow others to adjust volume
    else if(mPhoneState == AudioSystem::MODE_NORMAL &&AdjustMasterVolumeNormalState(stream)){
        return true;
    }
    else{
        return false;
    }
}

status_t AudioYusuPolicyManager::AdjustMasterVolume(int stream, int index,
    audio_io_handle_t output,AudioOutputDescriptor *outputDesc, unsigned int condition)
{
    // condition 0 :: output stop
    // condition 1 :: have output start
    LOGD("AdjustMasterVolume AdjustMasterVolume stream = %d index = %d output = %d condition = %d",stream,index,output,condition);

     if(output != mHardwareOutput)
     {
        LOGW("output = %d mHardwareOutput = %d",output,mHardwareOutput);
        return NO_ERROR;
     }
    // do not change actual stream volume if the stream is muted
    if (mOutputs.valueFor(output)->mMuteCount[stream] != 0) {
        LOGV(" stream %d muted count %d", stream, mOutputs.valueFor(output)->mMuteCount[stream]);
        return NO_ERROR;
    }

    // do not change in call volume if bluetooth is connected and vice versa
    if ((stream == AudioSystem::VOICE_CALL && mForceUse[AudioSystem::FOR_COMMUNICATION] == AudioSystem::FORCE_BT_SCO) ||
        (stream == AudioSystem::BLUETOOTH_SCO && mForceUse[AudioSystem::FOR_COMMUNICATION] != AudioSystem::FORCE_BT_SCO)) {
        LOGV("checkAndSetVolume() cannot set stream %d volume with force use = %d for comm",
             stream, mForceUse[AudioSystem::FOR_COMMUNICATION]);
        return INVALID_OPERATION;
    }

    ActiveStream  = GetMasterVolumeStream(outputDesc);
    // check if stream can adjust mastervolume in this phonemode
    if(!StreamMatchPhoneState((AudioSystem::stream_type)ActiveStream)){
        return NO_ERROR;
    }

    for(int i=0 ; i < AudioSystem::NUM_STREAM_TYPES; i++){
        LOGV("stream= %d outputDesc->mRefCount = %d  outputDesc->mMuteCount = %d",
             i,outputDesc->mRefCount[i],outputDesc->mMuteCount[i]);
    }

    //float volume = computeCustomVolume(ActiveStream,  mStreams[ActiveStream].mIndexCur, output, outputDesc->device());
    float volume  = 0.0;
    LOGV("AdjustMasterVolume ActiveStream = %d volume = %f outputDesc->mLatency = %d",ActiveStream,volume,outputDesc->mLatency);
    if(outputDesc->refCount () == condition){
        SetMasterVolume(volume,1);
    }
    else{
        SetMasterVolume(volume,outputDesc->mLatency*4);
    }

    if(volume > 0.0){
        mpClientInterface->setStreamVolume((AudioSystem::stream_type)ActiveStream, 1.0, output,outputDesc->mLatency);
    }
    else{
        mpClientInterface->setStreamVolume((AudioSystem::stream_type)ActiveStream, 0.0, output,outputDesc->mLatency);
    }
    return NO_ERROR;
}

void AudioYusuPolicyManager::SetStreamIndex_UCM(audio_io_handle_t output,AudioSystem::stream_type stream,int index)
{
    LOGD("SetStreamIndex_UCM output = %d stream = %d index = %d",output,stream,index);
    if(output!=mHardwareOutput )
        return;

    // tell audiohardware stream index
    AudioParameter outputCmd = AudioParameter();
    int Streamindex =0;
    Streamindex|= stream;
    Streamindex = Streamindex<<4;
    Streamindex|= index;
    outputCmd.addInt(String8(keySetVoumeIndex),Streamindex);
    mpClientInterface->setParameters(0, outputCmd.toString ());
}
#endif //#ifndef ANDROID_DEFAULT_CODE

status_t AudioYusuPolicyManager::checkAndSetVolume(int stream, int index, audio_io_handle_t output, uint32_t device, int delayMs, bool force)
{

    // do not change actual stream volume if the stream is muted
    if (mOutputs.valueFor(output)->mMuteCount[stream] != 0) {
        LOGV("checkAndSetVolume() stream %d muted count %d", stream, mOutputs.valueFor(output)->mMuteCount[stream]);
        return NO_ERROR;
    }

    // do not change in call volume if bluetooth is connected and vice versa
    if ((stream == AudioSystem::VOICE_CALL && mForceUse[AudioSystem::FOR_COMMUNICATION] == AudioSystem::FORCE_BT_SCO) ||
        (stream == AudioSystem::BLUETOOTH_SCO && mForceUse[AudioSystem::FOR_COMMUNICATION] != AudioSystem::FORCE_BT_SCO)) {
        LOGV("checkAndSetVolume() cannot set stream %d volume with force use = %d for comm",
             stream, mForceUse[AudioSystem::FOR_COMMUNICATION]);
        return INVALID_OPERATION;
    }

    float volume = computeVolume(stream, index, output, device);
#ifndef ANDROID_DEFAULT_CODE
     //for VT notify tone when incoming call. it's volume will be adusted in hardware.
     if((stream == AudioSystem::VOICE_CALL ||stream == AudioSystem::BLUETOOTH_SCO) && mOutputs.valueFor(output)->mRefCount[stream]!=0 && mPhoneState==AudioSystem::MODE_IN_CALL)
     {
        volume =1.0;
     }
#endif
    // We actually change the volume if:
    // - the float value returned by computeVolume() changed
    // - the force flag is set
    if (volume != mOutputs.valueFor(output)->mCurVolume[stream] ||
            force) {
        mOutputs.valueFor(output)->mCurVolume[stream] = volume;
        LOGV("setStreamVolume() for output %d stream %d, volume %f, delay %d", output, stream, volume, delayMs);
        if (stream == AudioSystem::VOICE_CALL ||
            stream == AudioSystem::DTMF ||
            stream == AudioSystem::BLUETOOTH_SCO) {
            // offset value to reflect actual hardware volume that never reaches 0
            // 1% corresponds roughly to first step in VOICE_CALL stream volume setting (see AudioService.java)
            volume = 0.01 + 0.99 * volume;
            // Force VOICE_CALL to track BLUETOOTH_SCO stream volume when bluetooth audio is
            // enabled
            if (stream == AudioSystem::BLUETOOTH_SCO) {
                mpClientInterface->setStreamVolume(AudioSystem::VOICE_CALL, volume, output, delayMs);
            }
        }

        mpClientInterface->setStreamVolume((AudioSystem::stream_type)stream, volume, output, delayMs);
#ifndef ANDROID_DEFAULT_CODE
        #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
        SetStreamIndex_UCM(output,(AudioSystem::stream_type)stream,index);
        #endif
#endif //#ifndef ANDROID_DEFAULT_CODE
    }
    if (stream == AudioSystem::VOICE_CALL ||
        stream == AudioSystem::BLUETOOTH_SCO) {
        float voiceVolume;
        // Force voice volume to max for bluetooth SCO as volume is managed by the headset
        if (stream == AudioSystem::VOICE_CALL) {
#ifndef ANDROID_DEFAULT_CODE
            #ifndef MTK_AUDIO_GAIN_TABLE_SUPPORT
            voiceVolume = computeCustomVoiceVolume(stream, index, output, device);
            #else
            #endif
#else
            voiceVolume = (float)index/(float)mStreams[stream].mIndexMax;
#endif

        } else {
            voiceVolume = 1.0;
        }
        if (voiceVolume != mLastVoiceVolume && output == mHardwareOutput) {
            mpClientInterface->setVoiceVolume(voiceVolume, delayMs);
            mLastVoiceVolume = voiceVolume;
        }
    }

    return NO_ERROR;
}

void AudioYusuPolicyManager::applyStreamVolumes(audio_io_handle_t output, uint32_t device, int delayMs,bool force)
{
    LOGV("applyStreamVolumes() for output %d and device %x", output, device);

    for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
        checkAndSetVolume(stream, mStreams[stream].mIndexCur, output, device, delayMs);
    }
}


void AudioYusuPolicyManager::setStrategyMute(routing_strategy strategy, bool on, audio_io_handle_t output, int delayMs)
{
    LOGD("setStrategyMute() strategy %d, mute %d, output %d", strategy, on, output);
    for (int stream = 0; stream < AudioSystem::NUM_STREAM_TYPES; stream++) {
        if (getStrategy((AudioSystem::stream_type)stream) == strategy) {
            setStreamMute(stream, on, output, delayMs);
        }
    }
}

void AudioYusuPolicyManager::setStreamMute(int stream, bool on, audio_io_handle_t output, int delayMs)
{
    StreamDescriptor &streamDesc = mStreams[stream];
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);

    LOGD("setStreamMute() stream %d, mute %d, output %d, mMuteCount %d", stream, on, output, outputDesc->mMuteCount[stream]);

    if (on) {
        if (outputDesc->mMuteCount[stream] == 0) {
            if (streamDesc.mCanBeMuted) {
                checkAndSetVolume(stream, 0, output, outputDesc->device(), delayMs);
            }
        }
        // increment mMuteCount after calling checkAndSetVolume() so that volume change is not ignored
        outputDesc->mMuteCount[stream]++;
    } else {
        if (outputDesc->mMuteCount[stream] == 0) {
            LOGD("setStreamMute() unmuting non muted stream!");
            return;
        }
        if (--outputDesc->mMuteCount[stream] == 0) {
            checkAndSetVolume(stream, streamDesc.mIndexCur, output, outputDesc->device(), delayMs);
        }
    }
}

void AudioYusuPolicyManager::handleIncallSonification(int stream, bool starting, bool stateChange)
{
    // if the stream pertains to sonification strategy and we are in call we must
    // mute the stream if it is low visibility. If it is high visibility, we must play a tone
    // in the device used for phone strategy and play the tone if the selected device does not
    // interfere with the device used for phone strategy
    // if stateChange is true, we are called from setPhoneState() and we must mute or unmute as
    // many times as there are active tracks on the output

    if (getStrategy((AudioSystem::stream_type)stream) == STRATEGY_SONIFICATION) {
        AudioOutputDescriptor *outputDesc = mOutputs.valueFor(mHardwareOutput);
        LOGV("handleIncallSonification() stream %d starting %d device %x stateChange %d",
                stream, starting, outputDesc->mDevice, stateChange);
        if (outputDesc->mRefCount[stream]) {
            int muteCount = 1;
            if (stateChange) {
                muteCount = outputDesc->mRefCount[stream];
            }
            if (AudioSystem::isLowVisibility((AudioSystem::stream_type)stream)) {
                LOGV("handleIncallSonification() low visibility, muteCount %d", muteCount);
                for (int i = 0; i < muteCount; i++) {
                    setStreamMute(stream, starting, mHardwareOutput);
                }
            } else {
                LOGV("handleIncallSonification() high visibility");
                if (outputDesc->device() & getDeviceForStrategy(STRATEGY_PHONE)) {
                    LOGV("handleIncallSonification() high visibility muted, muteCount %d", muteCount);
                    for (int i = 0; i < muteCount; i++) {
                        setStreamMute(stream, starting, mHardwareOutput);
                    }
                }
                if (starting) {
                    mpClientInterface->startTone(ToneGenerator::TONE_SUP_CALL_WAITING, AudioSystem::VOICE_CALL);
                } else {
                    mpClientInterface->stopTone();
                }
            }
        }
    }
}

bool AudioYusuPolicyManager::isInCall()
{
    return isStateInCall(mPhoneState);
}

bool AudioYusuPolicyManager::isStateInCall(int state) {
    return ((state == AudioSystem::MODE_IN_CALL) ||
            (state == AudioSystem::MODE_IN_COMMUNICATION));
}

#ifndef ANDROID_DEFAULT_CODE
bool AudioYusuPolicyManager::streamForcedToSpeaker(int streamType)
{
    return (streamType == AudioSystem::RING||streamType == AudioSystem::NOTIFICATION );
}

bool AudioYusuPolicyManager::streamForcedToSpeakerandMute(int streamType)
{
    return (streamType == AudioSystem::ALARM );
}

bool AudioYusuPolicyManager::streamForcedAudible(int streamType)
{
    return (streamType == AudioSystem::ENFORCED_AUDIBLE);
}

bool AudioYusuPolicyManager::streamMuteInRingTone(int streamType)
{
    return ( streamType == AudioSystem::MUSIC || streamType == AudioSystem::MATV||
             streamType == AudioSystem::TTS || streamType == AudioSystem::FM);
}

bool AudioYusuPolicyManager::streamMuteInForceSpeaker(int streamType)
{
    return ( streamType == AudioSystem::MUSIC || streamType == AudioSystem::MATV||
             streamType == AudioSystem::TTS || streamType == AudioSystem::FM);
}


bool AudioYusuPolicyManager::IsOutputDeviceWiredOn(void)
{
   if( mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)
      return true;
   else if( mAvailableOutputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET)
      return true;
   else
       return false;
}

void AudioYusuPolicyManager::SetMasterVolume(float volume,int delay){
    AudioParameter outputCmd = AudioParameter();
    outputCmd.addFloat(String8("SetMasterVolume"),volume);
    mpClientInterface->setParameters(0, outputCmd.toString(),delay);
}
#endif

bool AudioYusuPolicyManager::needsDirectOuput(AudioSystem::stream_type stream,
                                    uint32_t samplingRate,
                                    uint32_t format,
                                    uint32_t channels,
                                    AudioSystem::output_flags flags,
                                    uint32_t device)
{
   return ((flags & AudioSystem::OUTPUT_FLAG_DIRECT) ||
          (format !=0 && !AudioSystem::isLinearPCM(format)));
}

uint32_t AudioYusuPolicyManager::getMaxEffectsCpuLoad()
{
    return MAX_EFFECTS_CPU_LOAD;
}

uint32_t AudioYusuPolicyManager::getMaxEffectsMemory()
{
    return MAX_EFFECTS_MEMORY;
}

// --- AudioOutputDescriptor class implementation

AudioYusuPolicyManager::AudioOutputDescriptor::AudioOutputDescriptor()
    : mId(0), mSamplingRate(0), mFormat(0), mChannels(0), mLatency(0),
    mFlags((AudioSystem::output_flags)0), mDevice(0), mOutput1(0), mOutput2(0)
{
    // clear usage count for all stream types
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        mRefCount[i] = 0;
        mCurVolume[i] = -1.0;
        mMuteCount[i] = 0;
        mStopTime[i] = 0;
    }
}

uint32_t AudioYusuPolicyManager::AudioOutputDescriptor::device()
{
    uint32_t device = 0;
    if (isDuplicated()) {
        device = mOutput1->mDevice | mOutput2->mDevice;
    } else {
        device = mDevice;
    }
    return device;
}

void AudioYusuPolicyManager::AudioOutputDescriptor::changeRefCount(AudioSystem::stream_type stream, int delta)
{
    // forward usage count change to attached outputs
    if (isDuplicated()) {
        mOutput1->changeRefCount(stream, delta);
        mOutput2->changeRefCount(stream, delta);
    }
    if ((delta + (int)mRefCount[stream]) < 0) {
        LOGW("changeRefCount() invalid delta %d for stream %d, refCount %d", delta, stream, mRefCount[stream]);
        mRefCount[stream] = 0;
        return;
    }
    mRefCount[stream] += delta;
    LOGV("changeRefCount() stream %d, count %d", stream, mRefCount[stream]);
}

uint32_t AudioYusuPolicyManager::AudioOutputDescriptor::refCount()
{
    uint32_t refcount = 0;
    for (int i = 0; i < (int)AudioSystem::NUM_STREAM_TYPES; i++) {
        refcount += mRefCount[i];
    }
    return refcount;
}

uint32_t AudioYusuPolicyManager::AudioOutputDescriptor::strategyRefCount(routing_strategy strategy)
{
    uint32_t refCount = 0;
    for (int i = 0; i < (int)AudioSystem::NUM_STREAM_TYPES; i++) {
        if (getStrategy((AudioSystem::stream_type)i) == strategy) {
            refCount += mRefCount[i];
        }
    }
    return refCount;
}

status_t AudioYusuPolicyManager::AudioOutputDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " Sampling rate: %d\n", mSamplingRate);
    result.append(buffer);
    snprintf(buffer, SIZE, " Format: %d\n", mFormat);
    result.append(buffer);
    snprintf(buffer, SIZE, " Channels: %08x\n", mChannels);
    result.append(buffer);
    snprintf(buffer, SIZE, " Latency: %d\n", mLatency);
    result.append(buffer);
    snprintf(buffer, SIZE, " Flags %08x\n", mFlags);
    result.append(buffer);
    snprintf(buffer, SIZE, " Devices %08x\n", device());
    result.append(buffer);
    snprintf(buffer, SIZE, " Stream volume refCount muteCount\n");
    result.append(buffer);
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        snprintf(buffer, SIZE, " %02d     %.03f     %02d       %02d\n", i, mCurVolume[i], mRefCount[i], mMuteCount[i]);
        result.append(buffer);
    }
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

// --- AudioInputDescriptor class implementation

AudioYusuPolicyManager::AudioInputDescriptor::AudioInputDescriptor()
    : mSamplingRate(0), mFormat(0), mChannels(0),
      mAcoustics((AudioSystem::audio_in_acoustics)0), mDevice(0), mRefCount(0),
      mInputSource(0)
{
}

status_t AudioYusuPolicyManager::AudioInputDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " Sampling rate: %d\n", mSamplingRate);
    result.append(buffer);
    snprintf(buffer, SIZE, " Format: %d\n", mFormat);
    result.append(buffer);
    snprintf(buffer, SIZE, " Channels: %08x\n", mChannels);
    result.append(buffer);
    snprintf(buffer, SIZE, " Acoustics %08x\n", mAcoustics);
    result.append(buffer);
    snprintf(buffer, SIZE, " Devices %08x\n", mDevice);
    result.append(buffer);
    snprintf(buffer, SIZE, " Ref Count %d\n", mRefCount);
    result.append(buffer);
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

// --- StreamDescriptor class implementation

void AudioYusuPolicyManager::StreamDescriptor::dump(char* buffer, size_t size)
{
    snprintf(buffer, size, "      %02d         %02d         %02d         %d\n",
            mIndexMin,
            mIndexMax,
            mIndexCur,
            mCanBeMuted);
}

// --- EffectDescriptor class implementation

status_t AudioYusuPolicyManager::EffectDescriptor::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, " I/O: %d\n", mIo);
    result.append(buffer);
    snprintf(buffer, SIZE, " Strategy: %d\n", mStrategy);
    result.append(buffer);
    snprintf(buffer, SIZE, " Session: %d\n", mSession);
    result.append(buffer);
    snprintf(buffer, SIZE, " Name: %s\n",  mDesc.name);
    result.append(buffer);
    snprintf(buffer, SIZE, " %s\n",  mEnabled ? "Enabled" : "Disabled");
    result.append(buffer);
    write(fd, result.string(), result.size());

    return NO_ERROR;
}

#ifndef ANDROID_DEFAULT_CODE

int AudioYusuPolicyManager::Audio_Match_Force_device(AudioSystem::forced_config  Force_config)
{
    //LOGV("Audio_Match_Force_device Force_config=%x",Force_config);
    switch(Force_config)
    {
        case AudioSystem::FORCE_SPEAKER:
        {
            return AudioSystem::DEVICE_OUT_SPEAKER;
        }
        case AudioSystem::FORCE_HEADPHONES:
        {
            return AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
        }
        case AudioSystem::FORCE_BT_SCO:
        {
            return AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET;
        }
        case AudioSystem::FORCE_BT_A2DP:
        {
            return AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP;
        }
        case AudioSystem::FORCE_WIRED_ACCESSORY:
        {
            return AudioSystem::DEVICE_OUT_AUX_DIGITAL;
        }
        default:
        {
            LOGE("Audio_Match_Force_device with no config =%d",Force_config);
            return AudioSystem::FORCE_NONE;
        }
    }
    return AudioSystem::FORCE_NONE;
}

int AudioYusuPolicyManager::Audio_Find_Normal_Output_Device(uint32_t mRoutes)
{
    LOGV("Audio_Find_Normal_Output_Device mRoutes = %x",mRoutes);
    if(mRoutes &  (AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) )
        return AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    else if(mRoutes &  (AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
        return AudioSystem::DEVICE_OUT_WIRED_HEADSET;
    else if(mRoutes &  (AudioSystem::DEVICE_OUT_AUX_DIGITAL) )
        return AudioSystem::DEVICE_OUT_AUX_DIGITAL;
    else if(mRoutes &  (AudioSystem::DEVICE_OUT_SPEAKER) )
        return AudioSystem::DEVICE_OUT_SPEAKER;
    else{
      LOGE("Audio_Find_Normal_Output_Device with no devices");
        return AudioSystem::DEVICE_OUT_SPEAKER;
    }
}

int AudioYusuPolicyManager::Audio_Find_Ringtone_Output_Device(uint32_t mRoutes)
{
   LOGV("Audio_Find_Normal_Output_Device mRoutes=%x",mRoutes);
   return AudioSystem::DEVICE_OUT_SPEAKER;
}

int AudioYusuPolicyManager::Audio_Find_Incall_Output_Device(uint32_t mRoutes)
{
     LOGV("Audio_Find_Incall_Output_Device mRoutes = %x",mRoutes);

     //can be adjust to control output deivces
     if(mRoutes &(AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) ) // if headphone . still ouput from headsetphone
         return AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
     else if(mRoutes &	(AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
         return AudioSystem::DEVICE_OUT_WIRED_HEADSET;
     else if(mRoutes &	(AudioSystem::DEVICE_OUT_EARPIECE) )
         return AudioSystem::DEVICE_OUT_EARPIECE;
     else if(mRoutes &	(AudioSystem::DEVICE_OUT_SPEAKER) )
         return AudioSystem::DEVICE_OUT_SPEAKER;
     else{
      LOGE("Audio_Find_Incall_Output_Device with no devices");
    	 return AudioSystem::DEVICE_OUT_EARPIECE;
     }
}

int AudioYusuPolicyManager::Audio_Find_Communcation_Output_Device(uint32_t mRoutes)
{
     LOGV("Audio_Find_Communcation_Output_Device mRoutes = %x",mRoutes);
   //can be adjust to control output deivces
   if(mRoutes &(AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) ) // if headphone . still ouput from headsetphone
      return AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
   else if(mRoutes &	(AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
      return AudioSystem::DEVICE_OUT_WIRED_HEADSET;
   else if(mRoutes &	(AudioSystem::DEVICE_OUT_EARPIECE) )
      return AudioSystem::DEVICE_OUT_EARPIECE;
   else if(mRoutes &	(AudioSystem::DEVICE_OUT_SPEAKER) )
      return AudioSystem::DEVICE_OUT_SPEAKER;
   else{
      LOGE("Audio_Find_Incall_Output_Device with no devices");
      return AudioSystem::DEVICE_OUT_EARPIECE;
   }
}

void AudioYusuPolicyManager::LoadCustomVolume()
{
    LOGD("LoadCustomVolume Audio_Custom_Volume");
    android::GetAudioCustomParamFromNV (&Audio_Custom_Volume);
}

status_t AudioYusuPolicyManager::SetPolicyManagerParameters(int par1,int par2 ,int par3,int par4)
{
    LOGD("SetPolicyManagerParameters par1 = %d par2 = %d par3 = %d par4 = %d",par1,par2,par3,par4);
    switch(par1){
        case POLICY_LOAD_VOLUME:{
            LoadCustomVolume();
            break;
         }
        case POLICY_SET_FM_SPEAKER:
        {
            if(mFmForceSpeakerState== par2){
                break;
            }
            else{
                mFmForceSpeakerState = par2;
            }
            break;
        }
        // only apply in maudiohardware
        default:
            break;
    }
    status_t volStatus =checkAndSetVolume(AudioSystem::FM, mStreams[AudioSystem::FM].mIndexCur,
        mOutputs.keyAt(0),mOutputs.valueAt(0)->device(),50,true);
    return NO_ERROR;
}

#endif
}; // namespace android

