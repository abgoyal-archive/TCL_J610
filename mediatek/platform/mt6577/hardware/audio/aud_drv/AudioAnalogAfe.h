


#ifndef ANDROID_AUDIO_ANALOGAFE_H
#define ANDROID_AUDIO_ANALOGAFE_H


#include <stdint.h>
#include <sys/types.h>
#include "AudioYusuDef.h"


#ifndef ABB_MDSYS_BASE
#define ABB_MDSYS_BASE           (0xFD114000)
#endif
#define MIX_ABB_REGION           (0x9B4)
#define MIX_PMU_REGION           (0x724)

#define MASK_ALL                  (0xFFFFFFFF)


typedef enum
{
   AnalogAFE_MUTE_L    = 0,
   AnalogAFE_MUTE_R    = 1,
   AnalogAFE_MUTE_ALL  = 2,
   AnalogAFE_MUTE_NONE = 3
} AnalogAFE_MUTE;

typedef enum
{
    AUDIO_PATH,
    VOICE_PATH,
    FM_PATH_MONO,
    FM_PATH_STEREO,
    FM_MONO_AUDIO,
    FM_STEREO_AUDIO,
    NONE_PATH
}AnalogAFE_Mux;


// AP_MDSYS
#define WR_PATH0         (ABB_MDSYS_BASE+0x0100)
#define WR_PATH1         (ABB_MDSYS_BASE+0x0104)
#define WR_PATH2         (ABB_MDSYS_BASE+0x0108)
#define ABIST_MON_CON0   (ABB_MDSYS_BASE+0x0220)
#define ABIST_MON_CON1   (ABB_MDSYS_BASE+0x0224)
#define ABIST_MON_CON2   (ABB_MDSYS_BASE+0x0228)
#define ABIST_MON_CON3   (ABB_MDSYS_BASE+0x022C)
#define ABIST_MON_CON4   (ABB_MDSYS_BASE+0x0230)
#define ABIST_MON_CON5   (ABB_MDSYS_BASE+0x0234)
#define ABIST_MON_CON6   (ABB_MDSYS_BASE+0x0238)
#define ABIST_MON_CON7   (ABB_MDSYS_BASE+0x023C)
#define ABIST_MON_CON8   (ABB_MDSYS_BASE+0x0240)
#define AUDIO_CON0       (ABB_MDSYS_BASE+0x0300)
#define AUDIO_CON1       (ABB_MDSYS_BASE+0x0304)
#define AUDIO_CON2       (ABB_MDSYS_BASE+0x0308)
#define AUDIO_CON3       (ABB_MDSYS_BASE+0x030C)
#define AUDIO_CON4       (ABB_MDSYS_BASE+0x0310)
#define AUDIO_CON5       (ABB_MDSYS_BASE+0x0314)
#define AUDIO_CON6       (ABB_MDSYS_BASE+0x0318)
#define AUDIO_CON7       (ABB_MDSYS_BASE+0x031C)
#define AUDIO_CON8       (ABB_MDSYS_BASE+0x0320)
#define AUDIO_CON9       (ABB_MDSYS_BASE+0x0324)
#define AUDIO_CON10      (ABB_MDSYS_BASE+0x0328)
#define AUDIO_CON11      (ABB_MDSYS_BASE+0x032C)
#define AUDIO_CON12      (ABB_MDSYS_BASE+0x0330)
#define AUDIO_CON13      (ABB_MDSYS_BASE+0x0334)
#define AUDIO_CON14      (ABB_MDSYS_BASE+0x0338)
#define AUDIO_CON15      (ABB_MDSYS_BASE+0x033C)
#define AUDIO_CON16      (ABB_MDSYS_BASE+0x0340)
#define AUDIO_CON17      (ABB_MDSYS_BASE+0x0344)
#define AUDIO_CON20      (ABB_MDSYS_BASE+0x0380)
#define AUDIO_CON21      (ABB_MDSYS_BASE+0x0384)
#define AUDIO_CON22      (ABB_MDSYS_BASE+0x0388)
#define AUDIO_CON23      (ABB_MDSYS_BASE+0x038C)
#define AUDIO_CON24      (ABB_MDSYS_BASE+0x0390)
#define AUDIO_CON25      (ABB_MDSYS_BASE+0x0394)
#define AUDIO_CON26      (ABB_MDSYS_BASE+0x0398)
#define AUDIO_CON27      (ABB_MDSYS_BASE+0x039C)
#define AUDIO_CON28      (ABB_MDSYS_BASE+0x03A0)
#define AUDIO_CON29      (ABB_MDSYS_BASE+0x03A4)
#define AUDIO_CON30      (ABB_MDSYS_BASE+0x03A8)
#define AUDIO_CON31      (ABB_MDSYS_BASE+0x03AC)
#define AUDIO_CON32      (ABB_MDSYS_BASE+0x03B0)
#define AUDIO_CON33      (ABB_MDSYS_BASE+0x03B4)
#define AUDIO_CON34      (ABB_MDSYS_BASE+0x03B8)
#define AUDIO_CON35      (ABB_MDSYS_BASE+0x03BC)

#define AUDIO_NCP0        (ABB_MDSYS_BASE+0x0400)
#define AUDIO_NCP1        (ABB_MDSYS_BASE+0x0404)
#define AUDIO_LDO0        (ABB_MDSYS_BASE+0x0440)
#define AUDIO_LDO1        (ABB_MDSYS_BASE+0x0444)
#define AUDIO_LDO2        (ABB_MDSYS_BASE+0x0448)
#define AUDIO_LDO3        (ABB_MDSYS_BASE+0x044C)
#define AUDIO_GLB0        (ABB_MDSYS_BASE+0x0480)
#define AUDIO_GLB1        (ABB_MDSYS_BASE+0x0484)
#define AUDIO_GLB2        (ABB_MDSYS_BASE+0x0488)
#define AUDIO_REG0        (ABB_MDSYS_BASE+0x04C0)
#define AUDIO_REG1        (ABB_MDSYS_BASE+0x04C4)

#define BBRX_CON0         (ABB_MDSYS_BASE+0x0A00)
#define BBRX_CON1         (ABB_MDSYS_BASE+0x0A04)
#define BBRX_CON2         (ABB_MDSYS_BASE+0x0A08)
#define BBRX_CON3         (ABB_MDSYS_BASE+0x0A0C)
#define BBRX_CON4         (ABB_MDSYS_BASE+0x0A10)
#define BBRX_CON5         (ABB_MDSYS_BASE+0x0A14)
#define BBRX_CON6         (ABB_MDSYS_BASE+0x0A18)
#define BBRX_CON7         (ABB_MDSYS_BASE+0x0A1C)
#define BBRX_CON8         (ABB_MDSYS_BASE+0x0A20)
#define BBRX_CON9         (ABB_MDSYS_BASE+0x0A24)
#define BBRX_CON10        (ABB_MDSYS_BASE+0x0A28)
#define BBRX_CON11        (ABB_MDSYS_BASE+0x0A2C)
#define BBRX_CON12        (ABB_MDSYS_BASE+0x0A30)
#define BBRX_CON13        (ABB_MDSYS_BASE+0x0A34)
#define BBRX_CON14        (ABB_MDSYS_BASE+0x0A38)
#define BBRX_CON15        (ABB_MDSYS_BASE+0x0A3C)
#define BBRX_CON16        (ABB_MDSYS_BASE+0x0A40)
#define BBRX_CON17        (ABB_MDSYS_BASE+0x0A44)

#define BBTX_CON0         (ABB_MDSYS_BASE+0x0A80)
#define BBTX_CON1         (ABB_MDSYS_BASE+0x0A84)
#define BBTX_CON2         (ABB_MDSYS_BASE+0x0A88)
#define BBTX_CON3         (ABB_MDSYS_BASE+0x0A8C)
#define BBTX_CON4         (ABB_MDSYS_BASE+0x0A90)
#define BBTX_CON5         (ABB_MDSYS_BASE+0x0A94)
#define BBTX_CON6         (ABB_MDSYS_BASE+0x0A98)
#define BBTX_CON7         (ABB_MDSYS_BASE+0x0A9C)
#define BBTX_CON9         (ABB_MDSYS_BASE+0x0AA4)
#define BBTX_CON10        (ABB_MDSYS_BASE+0x0AA8)
#define BBTX_CON11        (ABB_MDSYS_BASE+0x0AAC)
#define BBTX_CON12        (ABB_MDSYS_BASE+0x0AB0)
#define BBTX_CON13        (ABB_MDSYS_BASE+0x0AB4)
#define BBTX_CON14        (ABB_MDSYS_BASE+0x0AB8)
#define BBTX_CON15        (ABB_MDSYS_BASE+0x0ABC)
#define BBTX_CON16        (ABB_MDSYS_BASE+0x0AC0)

#define APC_CON0          (ABB_MDSYS_BASE+0x0C00)
#define APC_CON1          (ABB_MDSYS_BASE+0x0C04)
#define VBIAS_CON0        (ABB_MDSYS_BASE+0x0C40)
#define VBIAS_CON1        (ABB_MDSYS_BASE+0x0C44)
#define AFC_CON0          (ABB_MDSYS_BASE+0x0CC0)
#define AFC_CON1          (ABB_MDSYS_BASE+0x0CC4)
#define AFC_CON2          (ABB_MDSYS_BASE+0x0CC8)

#define BBTX_CON17        (ABB_MDSYS_BASE+0x0F00)
#define BBTX_CON18        (ABB_MDSYS_BASE+0x0F04)
namespace android {

class AudioYusuHardware;

class AudioAnalog
{
public:

   AudioAnalog(AudioYusuHardware *hw);
   ~AudioAnalog();

   void SetAnaReg(uint32 offset,uint32 value, uint32 mask);
   void GetAnaReg(uint32 offset,uint32 *value);
   void AnalogAFE_Open(AnalogAFE_Mux mux);
   void AnalogAFE_Close(AnalogAFE_Mux mux);
   void AnalogAFE_ChangeMux(AnalogAFE_Mux mux);
   void AnalogAFE_Depop(AnalogAFE_Mux mux, bool Enable);
   void AnalogAFE_Set_Mute(AnalogAFE_MUTE MuteType);
   void AnalogAFE_EnableHeadset(bool Enable);
   bool AnalogAFE_Init(uint32 Fd);
   bool AnalogAFE_Deinit(void);
   void EnableHeadset(bool Enable);
   void AnalogAFE_Set_DL_AUDHPL_PGA_Gain(int gain_value);
   void AnalogAFE_Set_DL_AUDHPR_PGA_Gain(int gain_value);
   void AnalogAFE_Recover(void);
   void AnalogAFE_Set_LineIn_Gain(int gain_value);
   void AnalogAFE_Request_ANA_CLK(void);
   void AnalogAFE_Release_ANA_CLK(void);


//#ifdef AUDIO_HQA_SUPPORT
   void HQA_AFE_Set_DL_AUDHS_PGA_Gain(int gain_value);
   void HQA_AFE_Set_DL_AUDHPL_PGA_Gain(int gain_value);
   void HQA_AFE_Set_DL_AUDHPR_PGA_Gain(int gain_value);
   void HQA_AFE_Set_AUD_LineIn_Gain(int gain_value);
   void Afe_Set_AUD_Level_Shift_Buf_L_Gain(int gain_value);
   void Afe_Set_AUD_Level_Shift_Buf_R_Gain(int gain_value);
   void HQA_AFE_Set_AUD_UL_ANA_PreAMP_L_Gain(int gain_value);
   void HQA_AFE_Set_AUD_UL_ANA_PreAMP_R_Gain(int gain_value);
   void HQA_Analog_AFE_Select_Audio_Voice_Buffer(int gain_value);    // 1:Audio Buffer,  2:Voice Buffer
   void HQA_Audio_LineIn_Record(int bEnable);
   void HQA_Audio_LineIn_Play(int bEnable);
//#endif

private:
   AudioYusuHardware *mAudioHardware;
   int    mFd;
   pthread_mutex_t depopMutex; //used for depop flow
//#ifdef AUDIO_HQA_SUPPORT
   int m_audio_voice_DAC_sel;
//#endif
};

}; // namespace android

#endif

