
#ifndef _AUDIO_CUST_PARAM_TYPE_H_
#define _AUDIO_CUST_PARAM_TYPE_H_

namespace android {

/* speech structure definition*/
typedef enum
{
    VOL_NORMAL   = 0 ,
    VOL_HEADSET      ,
    VOL_HANDFREE     ,
    MAX_VOL_CATE
} volume_category_enum;

typedef enum
{
    VOL_TYPE_TON  = 0 ,
    VOL_TYPE_KEY      ,
    VOL_TYPE_MIC      ,
    VOL_TYPE_FMR      ,
    VOL_TYPE_SPH      ,
    VOL_TYPE_SID	    ,
    VOL_TYPE_MEDIA    ,
    MAX_VOL_TYPE
} volume_type_enum;

typedef struct
{
    /* volume setting */
    char volume[MAX_VOL_CATE][MAX_VOL_TYPE];
    /* speech enhancement */
    unsigned short speech_common_para[12];
    unsigned short speech_mode_para[8][16];
    unsigned short speech_volume_para[4];//in the feature, should extend to [MAX_VOL_CATE][MAX_VOL_TYPE][4]
    /* debug info */
    unsigned short debug_info[16];
   /* speech input FIR */
   short          sph_in_fir[6][45];
   /* speech output FIR */
   short          sph_out_fir[6][45];
   /* digital gain of DL speech */
   unsigned short Digi_DL_Speech;
   /* digital gain of uplink speech */
   unsigned short Digi_Microphone;
   /* FM record volume*/
   unsigned short FM_Record_Volume;
   /* user mode : normal mode, earphone mode, loud speaker mode */
   unsigned short Bluetooth_Sync_Type;
   unsigned short Bluetooth_Sync_Length;
   unsigned short bt_pcm_in_vol;
   unsigned short bt_pcm_out_vol;
   unsigned short user_mode;
   /* auto VM record setting */
   unsigned short bSupportVM;
   unsigned short bAutoVM;
   // set to 1900, 2000, 2100, 2200¡Adefault 1900
   // control AFE_VAC_CON0  VMIC_VREF
   unsigned short uMicbiasVolt;
} SPH_CUSTOM_PARAM_STRUCT;

}; // namespace android

#endif //_AUDIO_CUST_PARAM_TYPE_H_
