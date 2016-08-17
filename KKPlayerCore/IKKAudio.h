/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
#ifndef IKKAudio_H_
#define IKKAudio_H_
typedef  void (*pfun)(void *UserData,char *buf,int buflen);
//typedef  void (*pfun)(void *UserData,char *buf,int buflen);
/*********/
class IKKAudio
{
  public:
	      virtual ~IKKAudio(){}
	      virtual void SetWindowHAND(int m_hwnd)=0;
	      virtual void SetUserData(void* UserData)=0;
	      /********������Ƶ�ص�����*********/
	      virtual void SetAudioCallBack(pfun fun)=0;
		  /***********��ʼ����Ƶ�豸*********/
	      virtual int OpenAudio( int &wanted_channel_layout, int &wanted_nb_channels,int &wanted_sample_rate)=0;
		  /*******��ȡ��Ƶ����********/
          virtual void ReadAudio()=0;	   
          virtual void Start()=0;	   
		  virtual void Stop()=0;	   
		  /*********�ر�**********/
		  virtual void CloseAudio()=0;	
		  /*********��������************/
		  /**********���ֵ1000����Сֵ0*******/
		  virtual void SetVolume(long value)=0;
		  virtual long GetVolume()=0;
};
#endif