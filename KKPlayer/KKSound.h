#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include "IKKAudio.h"
#include "InterCriSec.h"

#ifndef KKSound_H_
#define KKSound_H_
typedef  void (*pfun)(void *UserData,char *buf,int buflen);

class CKKSound: public IKKAudio
{
   public:
	       CKKSound(void);
	       ~CKKSound(void);

		   void InitAudio();
		   void SetWindowHAND(int m_hwnd);
		   void SetUserData(void* UserData);
		   /********������Ƶ�ص�����*********/
		   void SetAudioCallBack(pfun fun);
		   //��Ҫ���߳��е�������
		   void PlayAudio();
		   void CloseAudio();
           void SetVolume(long value);
		   long GetVolume();
   private:
	       DWORD res;
		   LPVOID buf; 
		   DWORD  buf_len;  
		   DWORD offset;
	       LPDIRECTSOUND8 lpDirectSound;
		   IDirectSoundBuffer *ppDSBuffer;
		   IDirectSoundBuffer8 *m_pDSBuffer8; //used to manage sound buffers.  
		   IDirectSoundNotify8 *m_pDSNotify;
           /**********�ص�����***********/
		   pfun m_pFun;
		   /**********�û�����***********/
		   void* m_UserData;
		   bool IsClose;
		   InterCriSec m_Lock;
};
#endif
