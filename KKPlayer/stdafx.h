// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#ifndef stdafx_kk_H_
#define stdafx_kk_H_
#include "targetver.h"


#ifndef LIBKKPLAYER
#include "UI.h"
#else
	#include <string>
	#include <vector>
	#include <list>
	#include <stdlib.h>
	#include <stdio.h>
	#include <tchar.h>
	#include <process.h>
	#include <Windows.h>
#endif
#include <assert.h>
typedef void *(*CreateRender)(HWND h,char *Oput);
typedef char (*DelRender)(void *p,char RenderType);
#endif
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
