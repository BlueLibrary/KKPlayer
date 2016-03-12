/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
#pragma once
#include <queue>
#include "stdafx.h"

#include "IKKAudio.h"
#include "render/render.h"
#include "IKKPlayUI.h"


#include "KKLock.h"
#include "KKVideoInfo.h"
#ifndef KKPlayer_H_
#define KKPlayer_H_
enum OpenMediaEnum
{
	No=0,
	Dump=1,//ת��
};
enum SeekEnum
{
	Up,
	Down,
	Left,
	Right
};
class KKPlayer
{
    public:
	        KKPlayer(IKKPlayUI* pPlayUI,IKKAudio* pSound);
	        ~KKPlayer(void);
			void SetWindowHwnd(HWND hwnd);
			/*********��ý��************/
			int OpenMedia(char* fileName,OpenMediaEnum en=No,char* FilePath="C:\\"); 
			void CloseMedia(); 
    		void ReadAV();
            int GetCurTime();
			
			void RenderImage(CRender *pRender);
			void AdjustDisplay(int w,int h);
			
#ifdef WIN32
			/*****Gdi*****/
			void OnDrawImageByDc(HDC memdc);
			void VideoDisplay(void *buf,int w,int h,void *usadata,double last_duration,double pts,double duration,long long pos,double diff);
#endif           
			//��ȡ��Ļ����
			//static unsigned WINAPI PicGdiGrab(LPVOID lpParameter);
			void SetVolume(long value);
			long GetVolume();
			//��ͣ
			void Pause();
			//�������
			void KKSeek( SeekEnum en,int value);
			void InitSound();
public:
			//��Ƶˢ�߳�
			void VideoRefresh();
			  //��Ƶ��Ϣ
			SKK_VideoState *pVideoInfo;
private:
	        //�ļ��򿪺���Ҫ��ʲô
	        OpenMediaEnum m_OpenMediaEnum;
	        CKKLock m_CloseLock;
	        bool m_bOpen;
	        IKKPlayUI* m_pPlayUI;
	        void VideoPushStream();
	        int WindowWidth;
			int WindowHeight;
	        SKK_Frame *m_DisplayVp;
	        IKKAudio* m_pSound;
			HWND m_hwnd;
	        void PacketQueuefree();
		    /*******��ʾ��Ƶ**********/
		    void video_image_refresh(SKK_VideoState *is);
			int64_t start_time;
			
			//��ǰʱ��
			int m_CurTime;
			SKK_ThreadInfo m_ReadThreadInfo;
			SKK_ThreadInfo m_VideoRefreshthreadInfo;
			//�ȴ��¼�
			HANDLE m_WaitEvent;

			//�������ݶ���
			std::queue<AVPacket *> m_PushPktQue;
			CKKLock m_PushStreamLock;
			char* m_pStrFilePath;
};
#endif
