//NoNeedWindows_H����Ҫwindowsͷ�ļ�
//#define WIN32_KK //ʹ��windows �߳�
#include <stdlib.h>
#ifdef _WINDOWS
     #define WIN32_KK  //����ʹ��pthred�߳�
#else
     #define Android_Plat 1
#endif
#ifndef stdafx_H_KK_
#define stdafx_H_KK_
   
	#ifdef WIN32_KK
			#define WIN32_LEAN_AND_MEAN
			#ifndef NoNeedWindows_H 
				#include <Windows.h>
				#include <WinUser.h>
			#endif
			#include <process.h>
			#include <assert.h>
            void WindowsLOGE(const char* format,...);
            typedef CRITICAL_SECTION KKCRITICAL_SECTION;
			//void __cdecl LOGE(const char* _Format,...);

#ifndef  LOGE_KK
            #define  LOGE_KK WindowsLOGE
#endif
			//__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
	#else
            #include "platforms.h"
	#endif

	#ifdef WIN32
	       #define snprintf _snprintf
	#endif 
    #include "KKError.h"
    //ͳһ�����ڴ�
    void *KK_Malloc_(size_t size);
	void  KK_Free_(void *ptr);
#endif