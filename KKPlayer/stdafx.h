// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifndef stdafx_H_
#define stdafx_H_
#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <string>
#include <vector>
#include <list>

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include <Windows.h>
#include <assert.h>
#ifndef LIBKKPLAYER
#include "UI.h"
#endif
typedef void *(*CreateRender)(HWND h,char *Oput);
typedef char (*DelRender)(void *p,char RenderType);
#endif
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
