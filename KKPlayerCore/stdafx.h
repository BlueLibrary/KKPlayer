//NoNeedWindows_H����Ҫwindowsͷ�ļ�
//#define WIN32_KK //ʹ��windows �߳�
#ifdef WIN32
#define WIN32_KK
#else
#define Android_Plat
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
            typedef CRITICAL_SECTION KKCRITICAL_SECTION;
            #define  LOGE(...)  ;
//(char _Labcd__[256]="%s";sprintf(_Labcd__,__VA_ARGS__);::OutputDebugStringA(_Labcd__))
	#else
            #include "platforms.h"
	#endif
#endif