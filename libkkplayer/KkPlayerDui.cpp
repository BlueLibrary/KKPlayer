#include "KkPlayerDui.h"
unsigned char* CKKPlayerDui::GetWaitImage(int &length,int curtime)
{
  return 0;
}
unsigned char* CKKPlayerDui::GetErrImage(int &length,int ErrType)
{
  return 0;
}
unsigned char* CKKPlayerDui::GetBkImage(int &length)
{
  return 0;
}
unsigned char* CKKPlayerDui::GetCenterLogoImage(int &length)
{
  return 0;
}
//��ʧ��
void CKKPlayerDui::OpenMediaStateNotify(char* strURL,EKKPlayerErr err)
{

}
 ///��ȡ�̴߳��ļ�ǰ�Ļص�����,���ڷֶΣ�����1�������зֶΣ���ʱInOutUrl�Ѿ����޸ġ�
 int CKKPlayerDui::PreOpenUrlCallForSeg(char *InOutUrl,int *AvIsSeg,int *Interrupt)
 {
    return 1;
 }
/*******��Ƶ����������.���ڼ�����һ����Ƭ*******/
void CKKPlayerDui::GetNextAVSeg(void *playerIns,int Err,int quesize,KKPlayerNextAVInfo &NextInfo)
{
    return;
}

/***��Ƶ��ȡ�߳̽������ã�ֻ��ʹ�����ֵַ****/
void  CKKPlayerDui::AVReadOverThNotify(void *playerIns)
{
    return;
}
/***��Ƶ��ȡ�߳̽�������***/
void  CKKPlayerDui::AVRender()
{
    return;
}