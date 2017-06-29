#ifndef CMainFrame_H_
#define CMainFrame_H_

#include <atlbase.h>
#include <atlapp.h>

#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>
#include <vector>
#include "KKPlayer.h"
#include "render/render.h"
#include "../KKPlayerCore/IKKPlayUI.h"
#include "ICallKKplayer.h"
#include "SDLSound.h"
#include "Resource.h"


#define  WM_MediaClose  WM_USER+100
#define  WM_OpenErr     WM_USER+101
typedef struct SWaitPicInfo
{
    unsigned char* Buf;
	int Len;
	short Time;
	int StartTime;
	int Index;
}SWaitPicInfo;

/*************���������ļ��ķ�Ƭ����***************/
typedef struct AVFILE_SEG_ITEM
{
   short                 SegId;
   int                   segsize;           ///�ļ���С��
   unsigned  int         milliseconds;      ///�ļ�ʱ�䡣 
   unsigned  int         seektime;
   char url[1024];        ///�ļ�·��.
   bool                  bIsAd;
   AVFILE_SEG_ITEM*      pre;
   AVFILE_SEG_ITEM*      next;
   AVFILE_SEG_ITEM()
   {
       memset(this,0,sizeof(AVFILE_SEG_ITEM));
   }
}AVFILE_SEG_ITEM;
typedef struct AVFILE_SEGS_INFO
{
   unsigned int            milliseconds;
   unsigned long long      FileSize;
   AVFILE_SEG_ITEM*        pItemHead;
   AVFILE_SEG_ITEM*        pItemTail;
   AVFILE_SEG_ITEM*        pCurItem;
   int                     ItemCount;
   AVFILE_SEGS_INFO()
   {
       memset(this,0,sizeof(AVFILE_SEGS_INFO));
   }
}AVFILE_SEGS_INFO;
/*************��Ƭ����****************/



class CMainFrame:public CFrameWindowImpl<CMainFrame>,public ICallKKplayer, public IKKPlayUI
{
public:
	CMainFrame(bool yuv420p=true,bool NeedDel=false);
	~CMainFrame();
	///���ó��޴�����Ⱦ
    void        SetDuiDraw(HWND hAudio,fpRenderImgCall  DuiDrawCall,void* UserData);
	int         GetDuiDraw();
/********��������ز���*********/
public:
	int          Pause();
	int          PktSerial();
	void         OnDecelerate();
	void         OnAccelerate();
	/******����ʵʱ��ý����С�ӳ�,��Сֵ2����λ��**********/
	int          SetMaxRealtimeDelay(int Delay);
	void         SetErrNotify(void *UserData,fpKKPlayerErrNotify m_ErrNotify);
    ///ץȡ��ƵͼƬ
    bool         GrabAvPicBGRA(void* buf,int len,int &w,int &h,bool keepscale=true);

	//��ȡ���ŵ�ʱ��
	int          GetPlayTime();
	int          GetTotalTime();
	int          GetRealtime();
    bool         GetMediaInfo(MEDIA_INFO& info);
	void         SetVolume(long value,bool tip=true);
	long         GetVolume();
	void         AvSeek(int value);
    int          OpenMedia(std::string url);
	int          DownMedia(char *KKVURL,bool Down=true);
    void         CloseMedia();
	//ȫ��
	void         FullScreen();

	HWND         GetPlayerWnd(){return m_hWnd;}
public:
	LRESULT  OnSize(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	BEGIN_MSG_MAP(CMainFrame)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_PAINT,OnPaint);
			MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd);
			MESSAGE_HANDLER(WM_SIZE,OnSize);
			MESSAGE_HANDLER(WM_TIMER,OnTimer);
			MESSAGE_HANDLER(WM_KEYDOWN,OnKeyDown);
			MESSAGE_HANDLER(WM_CLOSE,OnClose);
			MESSAGE_HANDLER(WM_MediaClose,OnMediaClose);
			MESSAGE_HANDLER(WM_OpenErr,OnOpenMediaErr);
			MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove);
			MESSAGE_HANDLER(WM_RBUTTONUP,OnRbuttonUp);
			MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLbuttonDown);
			MESSAGE_HANDLER(WM_SETCURSOR,OnSetCursor);
	END_MSG_MAP()
protected:
	    void     OnFinalMessage(HWND /*hWnd*/);
	    LRESULT  OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	    LRESULT  OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT  OnMediaClose(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnOpenMediaErr(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
	    LRESULT  OnPaint(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnEraseBkgnd(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		
		LRESULT  OnTimer(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		
		LRESULT  OnKeyDown(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnClose(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnRbuttonUp(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnMouseMove(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnLbuttonDown(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
		LRESULT  OnSetCursor(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled/**/);
protected:
		/********** IKKPlayUIʵ��*************/
		virtual unsigned char* GetErrImage(int &length,int ErrType);
		virtual unsigned char* GetWaitImage(int &len,int curtime);
		virtual unsigned char* GetBkImage(int &len);
		virtual unsigned char* GetCenterLogoImage(int &length);
		virtual void           OpenMediaStateNotify(char* strURL,EKKPlayerErr err);
		virtual int            PreOpenUrlCallForSeg(char *InOutUrl,int *AvIsSeg,int *Interrupt);
		/*******��Ƶ����������*******/
		virtual void           GetNextAVSeg(void *playerIns,int Stata,int quesize,KKPlayerNextAVInfo &NextInfo);
		virtual void           AVReadOverThNotify(void *playerIns);
		
		virtual void RenderLock();
	    virtual void RenderUnLock();
		virtual IkkRender* GetRender();
		virtual void         AVRender(); 	
private:
	    void                   OnDraw(HDC& memdc,RECT& rt);
private:
	   bool                  m_bNeedDel;
	   fpKKPlayerErrNotify   m_ErrNotify;
	   void*                 m_pErrNotifyUserData;
	   int                   m_nTipTick;
       int                   m_nSeekTip;
       IKKAudio*             m_pSound;

	   IkkRender*            m_pRender;
	   
	   int                   LeftWidth;
	   int                   LeftNavigationBarWidth;
    

	   ///�޴�����Ⱦ
	   int                                m_nDuiDraw;
	   fpRenderImgCall                    m_pDuiDrawCall;
	   void*                              m_pRenderUserData;
	   bool                               m_bYuv420p;
	   CKKLock                            m_FileSegLock;
	   AVFILE_SEGS_INFO                   m_FileInfos;///�ļ���Ƭ��Ϣ
	   short                              m_nCurSegId;
	   unsigned int                       m_nMilTimePos;
	   KKPlayer*                          m_pPlayerInstance;
	   unsigned int                       m_nPlayerInsCount;///������ʵ������
	   /*********Ĭ�ϱ���ͼƬ**********/
	   unsigned char*                     m_pBkImage;

	   unsigned char*                     m_pCenterLogoImage;
	   int                                m_CenterLogoLen;

	   unsigned char*                     m_pErrOpenImage;
	   int                                m_ErrOpenImgLen;
	   std::vector<SWaitPicInfo*>         m_WaitPicList;
	   SWaitPicInfo*                      m_CurWaitPic;
	   bool                               m_bOpen;
	   CPoint                             m_lastPoint;
	   UINT                               m_AVwTimerRes;
	   UINT                               m_AVtimerID;
	   bool                               m_bFullScreen;
	   int                                m_nFullLastTick;
	   int                                m_nCursorCount;
};
#endif