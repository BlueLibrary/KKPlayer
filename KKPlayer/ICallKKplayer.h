#ifndef  ICallKKplayer_H_
#define  ICallKKplayer_H_
#include "../KKPlayerCore/KKPlayerInfo.h"
#include <string>
//errcode �ο�EKKPlayerErr
typedef void (*fpKKPlayerErrNotify)(void *UserData,int errcode);

class  ICallKKplayer
{
public:
	    virtual ~ICallKKplayer(){}
		///����URL�滻����
	     virtual void        SetKKPlayerGetUrl(fpKKPlayerGetUrl pKKPlayerGetUrl)=0;
	    virtual int          Pause()=0;
		virtual int          PktSerial()=0;
		virtual void         OnDecelerate()=0;
		virtual void         OnAccelerate()=0;
		/******����ʵʱ��ý����С�ӳ�,��Сֵ2����λ��**********/
		virtual int          SetMaxRealtimeDelay(int Delay)=0;
		virtual void         SetErrNotify(void *UserData,fpKKPlayerErrNotify m_ErrNotify)=0;
		///ץȡ��ƵͼƬ
		virtual bool         GrabAvPicBGRA(void* buf,int len,int &w,int &h,bool keepscale=true)=0;

		//��ȡ���ŵ�ʱ��
		virtual int          GetPlayTime()=0;
		virtual int          GetTotalTime()=0;
		virtual int          GetRealtime()=0;
		virtual bool         GetMediaInfo(MEDIA_INFO& info)=0;
		virtual void         SetVolume(long value,bool tip=true)=0;
		virtual long         GetVolume()=0;
		virtual void         AvSeek(int value)=0;
		virtual int          OpenMedia(std::string url,std::string cmd)=0;
		virtual int          DownMedia(char *KKVURL,bool Down=true)=0;
		virtual void         CloseMedia()=0;
		//ȫ��
		virtual void         FullScreen()=0;
		virtual HWND         GetPlayerWnd()=0;
};
#endif