/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
#ifndef KKPlayUI_H_
#define KKPlayUI_H_
#include "render/render.h"
enum EKKPlayerErr
{
     KKOpenUrlOk=0,          /***�������򿪳ɹ�**/
	 KKOpenUrlOkFailure=1,   /**��������ʧ��***/
	 KKAVNotStream=2,
	 KKAVReady=3,            ///�����Ѿ�׼������
	 KKAVWait=4,             ///��Ҫ����
	 KKRealTimeOver=5,
	 KKEOF=6,                 ///�ļ������ˡ�
     KKAVOver=7,              ///��Ƶ���Ž�����
	 KKSeekOk=8,              ///Seek�ɹ�
	 KKSeekErr=9,             ///seekʧ��
};
typedef struct KKPlayerNextAVInfo
{
     char url[1024];///����Ҫ�򿪵�url��ַ��
	 bool NeedRead; ///�Ƿ���Ҫ�򿪡�
	 bool lstOpSt;  ///�ϴδ��Ƿ�ɹ�
	 short SegId;   ///��Ҫ�򿪵ķֶ�Id
}KKPlayerNextAVInfo;
/*******UI�ӿ�********/
class IKKPlayUI
{
   public:
	         virtual unsigned char* GetWaitImage(int &length,int curtime)=0;
			 virtual unsigned char* GetErrImage(int &length,int ErrType)=0;
		     virtual unsigned char* GetBkImage(int &length)=0;
			 virtual unsigned char* GetCenterLogoImage(int &length)=0;
			 //��ʧ��
			 virtual void OpenMediaStateNotify(char* strURL,EKKPlayerErr err)=0;
			 ///��ȡ�̴߳��ļ�ǰ�Ļص�����,���ڷֶΣ�����1�������зֶΣ���ʱInOutUrl�Ѿ����޸ġ�
			 virtual int PreOpenUrlCallForSeg(char *InOutUrl,int *AvIsSeg,int *Interrupt)=0;
			 /*******��Ƶ����������.���ڼ�����һ����Ƭ*******/
			 virtual void GetNextAVSeg(void *playerIns,int Err,int quesize,KKPlayerNextAVInfo &NextInfo)=0;

			 /***��Ƶ��ȡ�߳̽������ã�ֻ��ʹ�����ֵַ****/
			 virtual void  AVReadOverThNotify(void *playerIns)=0;
			 /***��Ƶ��ȡ�߳̽�������***/
			 virtual void  AVRender()=0;

			/* virtual void RenderLock()=0;
			 virtual void RenderUnLock()=0;*/
			 virtual IkkRender* GetRender()=0;
   protected:
	         unsigned char* m_pBkImage;
			 int m_pBkImageLen;
			 int m_pCenterLogoImageLen;

};
#endif
