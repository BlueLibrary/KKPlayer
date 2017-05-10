#ifndef WinDir_H_
#define WinDir_H_
#include "..\stdafx.h"
#include <string>
#ifdef  UNICODE  
     typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >  kkString;
#else
     typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> >           kkString;
#endif
class  CWinDir{
 public:
	    CWinDir();
		~CWinDir();
		//��ȡ�û�AppĿ¼
		std::basic_string<TCHAR> GetUserAppDir();
		//�ж��ļ��Ƿ����
		bool IsExist(LPCTSTR Path);
		//���Ŀ¼����������Զ�����
		bool CreateDir(LPCTSTR Path);
		//��ȡ��ǰģ��Ŀ¼
		std::basic_string<TCHAR> GetModulePath();
		/**
		*�����ļ�,ʹ��UIcode�Ÿ������ĳɹ�
		*/
		bool Copyfile(LPCTSTR pTo,LPCTSTR pFrom);
		bool Deletefile(LPCTSTR pPath);
		///�ж��Ƿ���Ŀ¼
		int  IsDirectory(LPCTSTR Path);

		void GetDiskFreeInfo(LPCTSTR pPath,double &TotalSize/***G***/,double &FreeSize);
		void GetBestDrive(LPTSTR OutStr,int Len);
};
#endif