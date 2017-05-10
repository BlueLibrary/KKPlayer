#include "../stdafx.h"
#include <vector>
#ifndef AVInfomanage_H_
#define AVInfomanage_H_
#include "SqliteOp.h"
#include "../../KKPlayerCore/KKLock.h"
typedef struct _AV_Hos_Info{
     char *url;
	 unsigned char* pBuffer;
	 int bufLen;
	 int width;
	 int height;
	 int CurTime;
	 int TotalTime;
}AV_Hos_Info;

class CHistoryInfoMgr{
public:
	     CHistoryInfoMgr(); 
		 ~CHistoryInfoMgr();
         void SetPath(const char *Path) ;
		 void InitDb();
		 /*******���Ž��ȸ�����Ϣ***********/
		 void UpDataAVinfo(char *strpath,int curtime,int totaltime,unsigned char* Imgbuf,int buflen,int width,int height);
       
		 //��ȡ�Ų�����ʷ��Ϣ
		 void GetAVHistoryInfo(std::vector<AV_Hos_Info *> &slQue);
  public:
	     static CHistoryInfoMgr *GetInance();
  private:
	     static CHistoryInfoMgr *m_pInance;
	     CSqliteOp  SqliteOp;
		 char       m_strDbPath[1024];
		 void*      m_pDb;
		 CKKLock    m_Lock;
};
#endif