#ifndef KkPlayerDui_H_
#define KkPlayerDui_H_
#include "../KKPlayerCore/IKKPlayUI.h"
#include "KKPlayer.h"
class CKKPlayerDui:public  IKKPlayUI
{
public:
        CKKPlayerDui();
		~CKKPlayerDui();
public:
         unsigned char* GetWaitImage(int &length,int curtime);
		 unsigned char* GetErrImage(int &length,int ErrType);
	     unsigned char* GetBkImage(int &length);
		 unsigned char* GetCenterLogoImage(int &length);
		 //��ʧ��
		 void OpenMediaStateNotify(char* strURL,EKKPlayerErr err);
		 ///��ȡ�̴߳��ļ�ǰ�Ļص�����,���ڷֶΣ�����1�������зֶΣ���ʱInOutUrl�Ѿ����޸ġ�
		 int PreOpenUrlCallForSeg(char *InOutUrl,int *AvIsSeg,int *Interrupt);
		 /*******��Ƶ����������.���ڼ�����һ����Ƭ*******/
		 void GetNextAVSeg(void *playerIns,int Err,int quesize,KKPlayerNextAVInfo &NextInfo);

		 /***��Ƶ��ȡ�߳̽������ã�ֻ��ʹ�����ֵַ****/
		 void  AVReadOverThNotify(void *playerIns);
		 /***��Ƶ��ȡ�߳̽�������***/
		 void  AVRender();
};
#endif