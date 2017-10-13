
#ifndef AVInfomanage_H_
#define AVInfomanage_H_
#include "../stdafx.h"
#include "SqliteOp.h"
#include <vector>
#include <set>
#include <helper/SCriticalSection.h>
#include "../KKPlayerCore/KKLock.h"
typedef struct _AV_Hos_Info{
     char *url;
	 unsigned char* pBuffer;
	 int bufLen;
	 int width;
	 int height;
	 int CurTime;
	 int TotalTime;
	 int id;
}AV_Hos_Info;

typedef struct AV_Transfer_Info{
	 char UrlInfo[2046];
	 char Alias[32];
	 char Category[32];
	 unsigned int FileSize;
	 unsigned int AcSize;
	 int          Speed;
}AV_Transfer_Info;

class CHistoryInfoMgr{
public:
	     CHistoryInfoMgr(); 
		 ~CHistoryInfoMgr();
         void SetPath(const char *Path) ;
		 void InitDb();
		 /*******���Ž��ȸ�����Ϣ***********/
		 void UpDataAVinfo(const char *strpath,int curtime,int totaltime,unsigned char* Imgbuf,int buflen,int width,int height);
		 ///
         void DelAVinfo(const char *strpath);
		 //���´�����Ϣ
		 void UpdateTransferInfo(char *urlInfo,char* alias,char *category,unsigned int FileSize,unsigned int AcSize,int Speed);

		 void GetAVTransferInfo(std::vector<AV_Transfer_Info *> &slQue,int Finish);
		 //��ȡ�Ų�����ʷ��Ϣ
		 void GetAVHistoryInfo(std::vector<AV_Hos_Info *> &slQue);

		 void UpdataConfig(const char* StrKey,const char* StrValue);

		 bool GetConfig(const char* StrKey,std::string &OutValue);

		 int  GetH264Codec();
		 void UpdataH264Codec(int value);

		 int  GetH265Codec();
		 void UpdataH265Codec(int value);

		 int  GetUseLibRtmp();
		 void UpdataUseLibRtmp(int value);

		 int  GetRtmpDelay();
		 void UpdataRtmpDelay(int value);

		 int  GetRendererMethod();
		 void UpdataRendererMethod(int value);

  public:
	     static CHistoryInfoMgr *GetInance();
  private:
	     static unsigned __stdcall             SqlOp_Thread(LPVOID lpParameter);
		 void                                  SqlOpFun();
		 std::vector<AV_Transfer_Info *>       m_TransferQue;
		 HANDLE           m_hThread;
         volatile	bool  m_bThOver;
		 unsigned  int   m_iThaddr;
	     static CHistoryInfoMgr       *m_pInance;
	     CSqliteOp                     SqliteOp;
		 char                          m_strDbPath[1024];
		 void*                         m_pDb;
		 CKKLock                       m_Lock;
		 int        m_nH264Codec;
		 int        m_nH265Codec;
		 int        m_nUselibRtmp;
		 int        m_nlibRtmpDelay;
		 int        m_nRenderer;
};
#endif