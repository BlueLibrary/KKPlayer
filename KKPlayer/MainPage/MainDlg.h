#ifndef CMainDlg_H_
#define CMainDlg_H_
#include "../stdafx.h"
#include "AVMiniTittle.h"
#include "AVMiniBottom.h"
#include "AVMaxBottom.h"
#include "AVTransferDlg.h"
#include "IMainUI.h"
namespace SOUI
{
class CMainDlg : public SHostWnd,public IMainUI
{
	public:
			CMainDlg();
			~CMainDlg();

			void ForceNoRound();
			int  OnCreate(LPCREATESTRUCT lpCreateStruct);
			BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
			void OnClose();
			void OnMaximize();
			void OnRestore();
			void OnMinimize();
			void OnRButtonUp(UINT nFlags, CPoint point);

			void OnSize(UINT nType, CSize size);
			void OnBtnMsgBox();
			void OnShowWindow(BOOL bShow, UINT nStatus);
			
			void OnFolder();
			void OnFileList();
			void OnAVPlay();
			void OnMuteAudio();
			void OnTimer(char cTimerID);
			bool OnSliderVideo(EventArgs *pEvtBase);
			bool OnSliderAudio(EventArgs *pEvtBase);
			//打开音视频文件
			int  OpenMedia(char *KKVURL,const char* avname=NULL,const char* cmd=NULL);
			int  CloseMedia();
			/*********下载文件***************/
			int  DownMedia(char *path);
			void OnDecelerate();
			void OnAccelerate();

			//DUI菜单响应函数
			void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);


			void ShowMiniUI(bool show);
			//全屏
			bool GetFullScreen();
			bool GetScreenModel();
			void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			void SetSeekControlValue(int Value,int CacheValue);
			void SetSeekCacheControlValue(int CacheValue);
			void SetSeekControlRange(int Value);
			void OnShowWinPlay();
			void OnShowWinRes();

			void OnWebResSearch();
			void WinTabShow(int Tab);
			void ShowAVDownManageDlg();
		
			void SetVolume(int vol,bool ctl=false);
			
			void OnAvSetting();
			///得到当前正在打开文件的Url
			std::string    GetOpeningUrl(){ return m_urlStr;}
			
	protected:
		    BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
		    LRESULT OnUIMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT OnUIRbuttonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT OnUILbuttonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT OnSetAvTilte(UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT OnAvlistOpen(UINT uMsg, WPARAM wParam, LPARAM lParam);
		EVENT_MAP_BEGIN()
				EVENT_NAME_COMMAND(L"btn_TopSearch",OnWebResSearch)
				EVENT_NAME_COMMAND(L"btn_TopPlay_Opt",OnShowWinPlay)
				EVENT_NAME_COMMAND(L"btn_TopRes_Opt",OnShowWinRes)
				EVENT_NAME_COMMAND(L"btn_close",OnClose)
				EVENT_NAME_COMMAND(L"btn_min",OnMinimize)
				EVENT_NAME_COMMAND(L"btn_max",OnMaximize)
				EVENT_NAME_COMMAND(L"btn_More",OnAvSetting)
				EVENT_NAME_COMMAND(L"btn_restore",OnRestore)
				EVENT_NAME_COMMAND(L"btn_msgbox",OnBtnMsgBox)
				EVENT_NAME_COMMAND(L"Btn_OpenLocal",OnFolder)
				EVENT_NAME_COMMAND(L"AVFolderBtn",OnFolder)
				EVENT_NAME_COMMAND(L"AVFileListBtn",OnFileList)
				EVENT_NAME_COMMAND(L"AVPlayBtn",OnAVPlay)
				EVENT_NAME_COMMAND(L"btn_audio",OnMuteAudio)
				EVENT_NAME_COMMAND(L"BtnDecelerate",OnDecelerate)
				EVENT_NAME_COMMAND(L"BtnAccelerate",OnAccelerate)
				EVENT_NAME_HANDLER(L"slider_video_Seek",EventSliderPos::EventID,OnSliderVideo)
				EVENT_NAME_HANDLER(L"AvAudio",EventSliderPos::EventID,OnSliderAudio)
				 
		EVENT_MAP_END()	

		BEGIN_MSG_MAP_EX(CMainDlg)
				MSG_WM_CREATE(OnCreate)
				MSG_WM_INITDIALOG(OnInitDialog)
				MSG_WM_TIMER_EX(OnTimer) 
				MSG_WM_CLOSE(OnClose)
				MSG_WM_SIZE(OnSize)
				MSG_WM_SHOWWINDOW(OnShowWindow)
				MSG_WM_COMMAND(OnCommand)
				MSG_WM_RBUTTONUP(OnRButtonUp)
				MSG_WM_KEYDOWN(OnKeyDown) 
				MSG_WM_SETCURSOR(OnSetCursor);
		        MESSAGE_HANDLER_EX(WM_UI_MOUSEMOVE,OnUIMouseMove);
				MESSAGE_HANDLER_EX(WM_UI_RBUTTONUP,OnUIRbuttonUp);
				MESSAGE_HANDLER_EX(WM_UI_LBUTTONDOWN,OnUILbuttonDown);
				MESSAGE_HANDLER_EX(WM_UI_SetAvTilte,OnSetAvTilte);
				MESSAGE_HANDLER_EX(WM_UI_AVLIST_OPEN,OnAvlistOpen);
				CHAIN_MSG_MAP(SHostWnd)
				REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
	private:
			void               UpdateDownSpeed(const char* jsonstr);
			void               UpdateAVSeek();
			void               FullScreen();
			void               ScreenNormal();
			void               ScreenMini();

			///播放历史数据
			void               showAV_List();
			void               AV_Title();
			BOOL			   m_bLayoutInited;
			int                m_PlayerState;
			int                m_CumrTime;
			int                m_lastSeekTime;
			int                m_SeekTimer;
			//包序列号
			int                m_serial;
			int                m_MenuId;
			//全屏
			bool               m_bFullScreen;
			bool               m_bScreenNormal;
			//得到
			int                m_nDownSpeed;
			CAVMiniTittle      m_AVMiniTittle;
			CAVMiniBottom      m_AVMiniBottom;
			
			int                m_showAV_List;
			std::string        m_urlStr;
			int                m_nMuteVol;////
			int                m_bMini;
	};
}
#endif