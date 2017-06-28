#include "../stdafx.h"
#ifndef RENDER_H
#define RENDER_H
#ifndef  LOBYTE
#define LOBYTE(w)           ((unsigned char)(((unsigned long)(w)) & 0xff))
#endif
#ifndef GetRValue
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((unsigned short)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
#endif
#define kkRGB(r,g,b)              ((unsigned long )(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))
///��������
typedef struct kkRect
{
	int left;
	int top;
	int right;
	int bottom;
}kkRect;
//λͼ����
typedef struct kkBitmap
{
   void *pixels;
   int  width;
   int  height;
   unsigned char format;  /// 1 BGRA 32
}kkBitmap;
///��Ƶͼ����Ϣ
typedef struct kkAVPicInfo
{
    unsigned char *data[8];
    int      linesize[8];
	int      picformat;
	int      width;
	int      height;

}kkAVPicInfo;
typedef void (*fpRenderImgCall) (void* BufRgb,int width,int height,int BufLen,void* UserData);
/*******
State = 0  ��Ҫ�ͷ���Դ
State = 1  ����ʧ��
State = 2  ���óɹ�
*****/
typedef void (*fpResetDevCall) (void* UserData,int State);
class IkkRender
{
public:
    enum RenderType
    {
        GDI_RENDER,
        D3D_RENDER,
    };
    virtual bool init(HWND hView) = 0;
    virtual void destroy() = 0;
    virtual void resize(unsigned int w, unsigned int h) = 0;
    //virtual void WinSize(unsigned int w, unsigned int h) = 0;

	virtual void render(kkAVPicInfo *Picinfo,bool wait)=0;
	//���ֱ���ͼƬ
	virtual void renderBk(unsigned char* buf,int len)=0;
	virtual void SetWaitPic(unsigned char* buf,int len)=0;

	virtual void LoadCenterLogo(unsigned char* buf,int len)=0;

	virtual void SetErrPic(unsigned char* buf,int len)=0;
	virtual void ShowErrPic(bool show)=0;
	virtual void FillRect(kkBitmap img,kkRect rt,unsigned int color)=0;

	//���ô�����ʾ
	virtual void SetLeftPicStr(const char *str)=0;
	virtual void SetRenderImgCall(fpRenderImgCall fp,void* UserData)=0;

	///���ڻ�ȡd3d��Ϣ
	virtual bool GetHardInfo(void** pd3d,void** pd3ddev,int *ver)=0;
	virtual void SetResetHardInfoCall(fpResetDevCall call,void* UserData)=0;

	virtual void renderLock()=0;
	virtual void renderUnLock()=0;
};

#endif