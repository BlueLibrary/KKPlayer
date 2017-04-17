/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
#include <queue>
#include <list>
#include "stdafx.h"

#include "IKKAudio.h"
#include "render/render.h"
#include "IKKPlayUI.h"


#include "KKLock.h"
#include "KKVideoInfo.h"
#include "SqlOp/AVInfomanage.h"
#include "KKPlugin.h"

#ifndef KKPlayer_H_
#define KKPlayer_H_

enum SeekEnum
{
	Up,
	Down,
	Left,
	Right
};

typedef struct AVCACHE_INFO
{
	int VideoSize;
	int AudioSize;
	int MaxTime;
}AVCACHE_INFO;
//��ȡ��������Ϣ
typedef struct MEDIA_INFO 
{
	//�ֱ���
	char              AVRes[32];
	char              AVinfo[1024];
	const char*       AvFile;
	int               FileSize;
	int               CurTime;
	int               TotalTime;//��ʱ��
	int               serial;
	bool              Open;
	int               KKState;
    short             SegId;
	AVCACHE_INFO      CacheInfo;
}MEDIA_INFO ;

class KKPlayer
{
    public:
	        KKPlayer(IKKPlayUI* pPlayUI,IKKAudio* pSound);
	        ~KKPlayer(void);
			/******Windowsƽ̨����**********/
			void SetWindowHwnd(HWND hwnd);

			
			///ǿ���жϡ�
			void ForceAbort();

			///����������Ƶ
			void SetLastOpenAudio(bool bLastOpenAudio);
			///�����Ƿ����
			void SetRender(bool bRender);
			/*********��ý��.�ɹ�����0��ʧ�ܷ���-1.************/
			int OpenMedia(char* URL,char* Other=""); 
			
			/*********�رղ�����*********/
			void CloseMedia(); 
    		
			//��ȡ���ŵ�ʱ��
            int GetCurTime();
			
			void RenderImage(IkkRender *pRender,bool Force);
			
			void OnDecelerate();
			void OnAccelerate();
			int  GetAVRate();
#ifdef WIN32_KK
			/*****Gdi*****/
			void OnDrawImageByDc(HDC memdc);
			void VideoDisplay(void *buf,int w,int h,void *usadata,double last_duration,double pts,double duration,long long pos,double diff);
#endif           
			void SetVolume(long value);
			long GetVolume();
			//��ͣ
			void Pause();
			//������ˣ����
			void KKSeek( SeekEnum en,int value);
			//��λʱ����
			void AVSeek(int value,short segid=-1);
			
			
			//��ȡ������Ϣ
			bool GetMediaInfo(MEDIA_INFO &info);
			
			//�õ������к�
			int   GetPktSerial();	
			short GetSegId();
			short GetCurSegId();
		
			//��ȡ�Ų�����ʷ��Ϣ
			void GetAVHistoryInfo(std::vector<AV_Hos_Info *> &slQue);

			//1Ϊ��ý��,û��open����-1/******�Ƿ�����ý��,�÷������Ǻ�׼ȷ*****/
			int GetRealtime();

			//�����BGRA��ʽ
			void SetBGRA();
			
			/******�Ƿ�׼������,׼���÷���1�����򷵻�0��û��open����-1*******/
			int GetIsReady();
			
			//�õ��ӳ�
			int GetRealtimeDelay();
			//ǿ��ˢ�²�����Que
			void ForceFlushQue();
		
			/******����ʵʱ��ý����С�ӳ�,��Сֵ2����λ��**********/
			int SetMaxRealtimeDelay(int Delay);
            
			//��ʾ��Ƶ׷����Ϣ,����1�ɹ�
			int ShowTraceAV(bool Show);


			/*******************�������,����1�ж�Ӧ�Ĳ��*********************/
			int KKProtocolAnalyze(char *StrfileName,KKPluginInfo &KKPl);
			/**********��Ӳ��**********/
			static void AddKKPluginInfo(KKPluginInfo &info);

#ifdef Android_Plat
			void *  GetVideoRefreshJNIEnv()
			{
			    return m_pVideoRefreshJNIEnv;
			}
#endif
private:
	       
	        /*********��Ƶˢ���߳�********/
	        static unsigned __stdcall VideoRefreshthread(LPVOID lpParameter);
			//��Ƶ�ص��߳�
			static unsigned __stdcall Audio_Thread(LPVOID lpParameter);
			//�ļ���ȡ�߳�
	        static unsigned __stdcall ReadAV_thread(LPVOID lpParameter);
			
			void        OpenAudioDev();
			
			void        loadSeg(AVFormatContext**  pAVForCtx,int AVQueSize,short segid=-1);
			void        InterSeek(AVFormatContext*  pAVForCtx);
			//���ݶ�ȡ
	        void        ReadAV();
			void OpenInputAV(const char *url,short segid);
	        //��Ƶˢ�߳�
			void VideoRefresh();
	        
			/*******��ʾ��Ƶ**********/
		    void video_image_refresh(SKK_VideoState *is);
			/******������Ƶͼ��******/
            void video_audio_display(IkkRender *pRender,SKK_VideoState *s);

			/********��ý������ˢ�º���**********/
			void Avflush(int64_t seek_target); 
			void AvflushRealTime(int Avtype);
			//����Ƶ
			void ReadAudioCall();
			void PacketQueuefree();
			KKPlayer(const CKKLock& cs);
			KKPlayer operator = (const CKKLock& cs);
private:
	        //�����Ϣ
	        static std::list<KKPluginInfo>  KKPluginInfoList;
			//������Ϣ
			AVCACHE_INFO m_AVCacheInfo;

	        //��Ƶ��Ϣ
	        SKK_VideoState *pVideoInfo; 

			
			///AV��Ƶ��Ϣ
			KKPlayerNextAVInfo m_AVNextInfo;
			//��¼������Ϣ��
			CAVInfoManage*     m_pAVInfomanage;
            //������Ϣ
			MEDIA_INFO         m_AVPlayInfo;
	        CKKLock            m_PlayerLock;
			
			///�Ƿ���ʾ
			bool         m_bRender;
			bool         m_bLastOpenAudio;
            volatile int m_nPreFile;

	        volatile  bool m_bOpen;
	        IKKPlayUI* m_pPlayUI;
	       
			//��ǰ�����к�
			volatile int m_PktSerial;
	        IKKAudio*    m_pSound;
			HWND         m_hwnd;
			int64_t      start_time;
			
			///seektime
			int          m_nSeekTime;
			short        m_nSeekSegId;
			//��ǰʱ��
			int m_CurTime;
			//��Ƶ��ȡ�߳�
			SKK_ThreadInfo m_ReadThreadInfo;
			//��Ƶˢ���߳�
			SKK_ThreadInfo m_VideoRefreshthreadInfo;
            //��Ƶ���ݻص��߳�
			SKK_ThreadInfo m_AudioCallthreadInfo;
			int64_t m_lstPts;
			
            //������Ⱦ��Ƶ����
			void *m_pAudioPicBuf;
			int   m_AudioPicBufLen;
			
#ifdef Android_Plat
			void *  m_pVideoRefreshJNIEnv;
#endif
			
};
#endif
