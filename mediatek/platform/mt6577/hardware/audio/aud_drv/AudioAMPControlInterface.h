
#ifndef __AUDIO_AMP_CONTROL_INTERFACE_H__
#define __AUDIO_AMP_CONTROL_INTERFACE_H__

namespace android {

	enum AUDIO_AMP_CONTROL_COMMAND{
	    AUD_AMP_GET_CTRP_NUM ,
	    AUD_AMP_GET_CTRP_BITS,
	    AUD_AMP_GET_CTRP_TABLE,
	    AUD_AMP_GET_REGISTER,
	    AUD_AMP_SET_REGISTER,
	    AUD_AMP_SET_AMPGAIN,  // gain is use for low 24bits as external amp , device should base on control point set to AMPLL_CON0_REG
	    AUD_AMP_GET_AMPGAIN,
	    AUD_AMP_SET_MODE,
	    NUM_AUD_AMP_CONTROL_COMMAND
	};

	typedef struct {
		unsigned long int	command;
		unsigned long int 	param1;
		unsigned long int 	param2;
	}AMP_Control;

	class AudioAMPControlInterface
	{
	public:
		AudioAMPControlInterface(){}
		virtual ~AudioAMPControlInterface(){}
		//open/close device
		virtual bool setSpeaker(bool on)=0;
		virtual bool setHeadphone(bool on)=0;
		virtual bool setReceiver(bool on)=0;
		//set mode to amp
		virtual void setMode(int mode)=0;
		// for set and get parameters , for command1 and command 2 , use to command
		// and data is used for comamnd need to carry a lots of command.
		virtual int setParameters(int command1 ,int command2 , unsigned int data)=0;
		virtual int getParameters(int command1 ,int command2 , void * data)=0;
	};

	class AudioDeviceManger
	{
	public:
		static AudioAMPControlInterface *createInstance(int fd);
		static void releaseInstance(AudioAMPControlInterface *AudioDevice);
	};

}

#endif

