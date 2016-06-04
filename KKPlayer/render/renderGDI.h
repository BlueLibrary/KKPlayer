#ifndef RENDER_GDI_H
#define RENDER_GDI_H

#include "../../KKPlayerCore/render/render.h"

#include <core\SkCanvas.h>
#include <core\SkBitmap.h>
#include <core\SkTypeface.h>
#include <core\SkImageDecoder.h>
#include <core\SkStream.h>
class CRenderGDI : public CRender
{
public:
    CRenderGDI();
    ~CRenderGDI();
    virtual bool init(HWND hView);
    virtual void destroy();
    virtual void resize(unsigned int w, unsigned int h);
	virtual void LoadCenterLogo(unsigned char* buf,int len);
	void render(char* buf,int width,int height);
	virtual void renderBk(unsigned char* buf,int len);
	void SetWaitPic(unsigned char* buf,int len);
	void SetBkImagePic(unsigned char* buf,int len);
    void WinSize(unsigned int w, unsigned int h);
protected:
    void createBitmap(unsigned int w, unsigned int h);
    void DrawSkVideo(SkCanvas& canvas,char *buf,int w,int h);
    void skiaSal(char *buf,int w,int h);
private:
	SkPaint  m_Paint; 
	HWND m_hView;
	HBITMAP m_hBitmap;
	HDC m_hDC;

	//��ʾ������
	unsigned int m_width;
	//��ʾ����߶�
	unsigned int m_height;

	//ͼ��������
	unsigned int m_Picwidth;
	//ͼ������߶�
	unsigned int m_Picheight;

	char* m_BkBuffer;
	int m_BkLen;

	char *m_WaitBuffer;
	int m_WaitLen;

	void* m_pixels;
};

#endif