//NoNeedWindows_H����Ҫwindowsͷ�ļ�
#ifndef stdafx_H_
#define stdafx_H_
	#ifdef WIN32
			#define WIN32_LEAN_AND_MEAN
			#ifndef NoNeedWindows_H 
			#include <Windows.h>
			#include <WinUser.h>
			#endif
			#include <process.h>
			#include <assert.h>
	#else
			#include <pthread.h>
            #include <unistd.h>
            #include "platforms.h"
	#endif
#endif