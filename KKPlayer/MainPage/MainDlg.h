#ifndef CMainDlg_H_
#define CMainDlg_H_
#include "../stdafx.h"
namespace SOUI
{
class CMainDlg : public SHostWnd
{
	public:
		CMainDlg();
		~CMainDlg();

		void OnClose()
		{
			DestroyWindow();
		}
		void OnMaximize()
		{
			SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
		}
		void OnRestore()
		{
			SendMessage(WM_SYSCOMMAND,SC_RESTORE);
		}
		void OnMinimize()
		{
			SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
		}
 //virtual BOOL _HandleEvent(SOUI::EventArgs *pEvt){return FALSE;}
		void OnSize(UINT nType, CSize size)
		{
			SetMsgHandled(FALSE);
			if(!m_bLayoutInited)
				return;
			if(nType==SIZE_MAXIMIZED)
			{
				FindChildByName(L"btn_restore")->SetVisible(TRUE);
				FindChildByName(L"btn_max")->SetVisible(FALSE);
			}else if(nType==SIZE_RESTORED)
			{
				FindChildByName(L"btn_restore")->SetVisible(FALSE);
				FindChildByName(L"btn_max")->SetVisible(TRUE);
			}
		}
		void OnBtnMsgBox()
		{
			SMessageBox(NULL,_T("this is a message box"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
			SMessageBox(NULL,_T("this message box includes two buttons"),_T("haha"),MB_YESNO|MB_ICONQUESTION);
			SMessageBox(NULL,_T("this message box includes three buttons"),NULL,MB_ABORTRETRYIGNORE);
		}
	    
		int OnCreate(LPCREATESTRUCT lpCreateStruct);
		void OnShowWindow(BOOL bShow, UINT nStatus);
		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

        void OnFolder();
		void OnFileList();
		void OnAVPlay();
		
		void OnTimer(char cTimerID);
		bool OnSliderVideo(EventArgs *pEvtBase);
		bool OnSliderAudio(EventArgs *pEvtBase);

		void OnDecelerate();
		void OnAccelerate();
	protected:

		EVENT_MAP_BEGIN()
			 
			EVENT_NAME_COMMAND(L"btn_close",OnClose)
			EVENT_NAME_COMMAND(L"btn_min",OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max",OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore",OnRestore)
			EVENT_NAME_COMMAND(L"btn_msgbox",OnBtnMsgBox)
			EVENT_NAME_COMMAND(L"AVFolderBtn",OnFolder)
			EVENT_NAME_COMMAND(L"AVFileListBtn",OnFileList)
			EVENT_NAME_COMMAND(L"AVPlayBtn",OnAVPlay)
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
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
	private:
		BOOL			m_bLayoutInited;
		int             m_PlayerState;
		int             m_CumrTime;
		int             m_SeekTimer;
		//包序列号
		int             m_serial;
		
	};
}
#endif