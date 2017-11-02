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

#include "KKPlayerInfo.h"
#include "IKKAudio.h"
#include "render/render.h"
#include "IKKPlayUI.h"


#include "KKLock.h"
#include "KKVideoInfo.h"
#include "KKPlugin.h"
#include <string>
#ifndef KKPlayer_H_
#define KKPlayer_H_


class KKPlayer
{
    public:
	        KKPlayer(IKKPlayUI* pPlayUI,IKKAudio* pSound);
	        ~KKPlayer(void);
			/******Windowsƽ̨����**********/
			void          SetWindowHwnd(HWND hwnd);

			///����URL�滻����
			void           SetKKPlayerGetUrl(fpKKPlayerGetUrl pKKPlayerGetUrl);
			///ǿ���жϡ�
			void          ForceAbort();

			///����������Ƶ
			void          SetLastOpenAudio(bool bLastOpenAudio);
			///�����Ƿ����
			void          SetRender(bool bRender);
			/*********��ý��.�ɹ�����0��ʧ�ܷ���-1.************/
			int           OpenMedia(const char* URL,const char* Other=""); 
			
			/*********�رղ�����*********/
			void          CloseMedia(); 
			void          RenderImage(IkkRender *pRender,bool Force);
			
			void          OnDecelerate();
			void          OnAccelerate();
			int           GetAVRate();
#ifdef WIN32_KK
			/*****Gdi*****/
			void          OnDrawImageByDc(HDC memdc);
			void          VideoDisplay(void *buf,int w,int h,void *usadata,double last_duration,double pts,double duration,long long pos,double diff);
#endif           
			void          SetVolume(long value);
			long          GetVolume();
			//��ͣ
			void          Pause();
			//������ˣ����
			int           KKSeek( SeekEnum en,int value);
			//��λʱ����
			int           AVSeek(int value,short segid=-1);
			
			
			//��ȡ������Ϣ
			bool          GetMediaInfo(MEDIA_INFO &info);
			
			//�õ������к�
			int           GetPktSerial();	
			short         GetSegId();
			short         GetCurSegId();
		
		

			//1Ϊ��ý��,û��open����-1/******�Ƿ�����ý��,�÷������Ǻ�׼ȷ*****/
			int GetRealtime();

			//�����BGRA��ʽ
			void SetBGRA();
			///ץȡ��ƵͼƬ
			bool GrabAvPicBGRA(void* buf,int len,int &w,int &h,bool keepscale=true);
			/******�Ƿ�׼������,׼���÷���1�����򷵻�0��û��open����-1*******/
			int GetIsReady();
			
			//�õ��ӳ�
			int GetRealtimeDelay();
			//ǿ��ˢ�²�����Que
			void ForceFlushQue();
		
			/******����ʵʱ��ý����С�ӳ�,��Сֵ2����λ��**********/
			int SetMaxRealtimeDelay(double Delay);
            
			//��ʾ��Ƶ׷����Ϣ,����1�ɹ�
			int ShowTraceAV(bool Show);

			//��ȡ���ŵ�ʱ��
			int GetPlayTime();
			int GetTotalTime();

			
			/*******************�������,����1�ж�Ӧ�Ĳ��*********************/
			int KKProtocolAnalyze(char *StrfileName,KKPluginInfo &KKPl);
			/**********��Ӳ��**********/
			static void AddKKPluginInfo(KKPluginInfo &info);
			static std::list<KKPluginInfo>&  GetKKPluginInfoList();
#ifdef Android_Plat
			void *  GetVideoRefreshJNIEnv()
			{
			    return m_pVideoRefreshJNIEnv;
			}
			///����Surface,����Ӳ����Ⱦ��
			void    SetViewSurface(void* surface);
			bool    IsMediacodecSurfaceDisplay();
#endif

private:
	       
	        /*********��Ƶˢ���߳�********/
	        static unsigned __stdcall VideoRefreshthread(LPVOID lpParameter);
			//��Ƶ�ص��߳�
			static unsigned __stdcall Audio_Thread(LPVOID lpParameter);
			//�ļ���ȡ�߳�
	        static unsigned __stdcall ReadAV_thread(LPVOID lpParameter);
			
			void        OpenAudioDev();
			bool        OpenInputSegAV(const char *url,short segid,bool flush);
			bool        loadSeg(AVFormatContext**  pAVForCtx,int AVQueSize,short segid=-1,bool flush=false);
			void        InterSeek(AVFormatContext*  pAVForCtx);
			//���ݶ�ȡ
	        void        ReadAV();
			
			///�ӳٷ���
			void        AvDelayParser();
	        //��Ƶˢ�߳�
			void        VideoRefresh();
	        
			/*******��ʾ��Ƶ**********/
		    void        video_image_refresh(SKK_VideoState *is);
			/******������Ƶͼ��******/
            void        video_audio_display(IkkRender *pRender,SKK_VideoState *s);

			/********��ý������ˢ�º���**********/
			void        Avflush(int64_t seek_target); 
			void        AvflushRealTime(int Avtype);
			//����Ƶ�ص�
			void        ReadAudioCall();
			void        PacketQueuefree();

			
			KKPlayer(const KKPlayer& cs);
			KKPlayer operator = (const KKPlayer& cs);
private:
	        //�����Ϣ
	        static std::list<KKPluginInfo>  KKPluginInfoList;

			///������ѡ��
			std::string                     m_strcmd;
			
			//������Ϣ
			AVCACHE_INFO                    m_AVCacheInfo;

	        //��Ƶ��Ϣ
	        SKK_VideoState*                 pVideoInfo; 

			
			///AV��Ƶ��Ϣ
			KKPlayerNextAVInfo              m_AVNextInfo;
			
            //������Ϣ
			MEDIA_INFO                      m_AVPlayInfo;
			//��������
	        CKKLock                         m_PlayerLock;
            
			///������Ϣָʾ
			CKKLock                         m_AVInfoLock;
			///ָʾ�Ƿ��з�Ƭ
			int                             m_AvIsSeg;
			///�������
			int                             m_CacheAvCounter;
			///�Ƿ���ʾ
			bool                            m_bRender;
			bool                            m_bLastOpenAudio;
            volatile int                    m_nPreFile;

	        volatile  bool                  m_bOpen;
	        IKKPlayUI*                      m_pPlayUI;
	       
			//��ǰ�����к�
			volatile int                    m_PktSerial;
	        IKKAudio*                       m_pSound;
			///���ھ��
			HWND                            m_hwnd;
			int64_t                         start_time;
			
			///seektime
			int                             m_nSeekTime;
			short                           m_nSeekSegId;
			//��ǰʱ��
			int                             m_CurTime;
			int                             m_TotalTime;
			/// 
			//0x0001   ��Ƶ;
			//0x0010    ��Ƶ�� 
			//0x0100    ��Ļ�� 
			int                             m_nhasVideoAudio;
			//��Ƶ��ȡ�߳�
			SKK_ThreadInfo                  m_ReadThreadInfo;
			//��Ƶˢ���߳�
			SKK_ThreadInfo                  m_VideoRefreshthreadInfo;
            //��Ƶ���ݻص��߳�
			SKK_ThreadInfo                  m_AudioCallthreadInfo;
			int64_t                         m_lstPts;
			
            //������Ⱦ��Ƶ���εĻ���
			void*                           m_pAudioPicBuf;
			int                             m_AudioPicBufLen;
			//�����ͼ�εĸ�ʽ
			AVPixelFormat                   m_DstAVff;
			double                          m_nOpenTime;
#ifdef Android_Plat
			void*                           m_pVideoRefreshJNIEnv;
			void*                           m_pViewSurface;
			bool                            m_bSurfaceDisplay;
#endif
			///�����滻��ԭ����url
		    fpKKPlayerGetUrl                m_pKKPlayerGetUrl;           	
};

#endif
