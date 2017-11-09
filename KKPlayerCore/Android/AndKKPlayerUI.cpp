#include "AndKKPlayerUI.h"
#include "libavutil/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <linux/videodev2.h>
#include "GlEs2Render.h"
#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include "AndPlayerStateNotifyMgr.h"

///状态通知管理器
extern CAndPlayerStateNotifyMgr  PlayerStateNotifyMgr;

CAndKKPlayerUI::CAndKKPlayerUI(int RenderView):m_player(this,&m_Audio)
,m_nRenderType(RenderView),m_playerState(-1)///播放器未启动
,m_pRender(0),m_nRefreshPic(0)
{
    
   
    m_player.SetWindowHwnd(0); 
	m_Audio.SetWindowHAND(0); 
    m_bNeedReconnect= false; 
    if(m_nRenderType==0)	
	    m_pRender = new GlEs2Render(&m_player);
}



CAndKKPlayerUI::~CAndKKPlayerUI()
{
    m_player.CloseMedia();
    m_Audio.CloseAudio();
	
	if(m_nRenderType==0){
	   LOGE("GlEs2Render Del \n");
	   m_RenderLock.Lock();
	   if(m_pRender!=NULL){
       delete m_pRender;
	   m_pRender=NULL;
	   }
	   m_RenderLock.Unlock();
	}
    LOGI("~CAndKKPlayerUI\n");
}

int CAndKKPlayerUI::Init()
{
	int ret=0;
	if(m_nRenderType==0){
     
		ret=m_pRender-> init(0);
	}
	return ret;
}
		
int CAndKKPlayerUI::OnSize(int w,int h)
{
	
	if(m_nRenderType==0){
        m_pRender->resize(w,h);
	}
	return 0;
}

void SetKKplayerH264HardCodec(int value);
void SetKKplayerH265HardCodec(int value);
void  CAndKKPlayerUI::SetDecoderMethod(int method)
{
	
	if(method==0){
		SetKKplayerH264HardCodec(0);
        SetKKplayerH265HardCodec(0);
	}else{
		SetKKplayerH264HardCodec(SKK_VideoState::HARD_CODE_MEDIACODEC);
        SetKKplayerH265HardCodec(SKK_VideoState::HARD_CODE_MEDIACODEC);
	}
}
void CAndKKPlayerUI::SetSurfaceTexture(JNIEnv *env)
{
	jobject obj=0;
	m_RenderLock.Lock();
	if(m_pRender!=NULL){
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   obj=pRender->SetSurfaceTexture(env);
	   LOGI(" SetSurfaceTexture obj= %d\n",obj);
	   obj=pRender->GetViewSurface();
	   m_player.SetViewSurface(obj);
	}
	m_RenderLock.Unlock();

	
}

jobject CAndKKPlayerUI::GetSurfaceTexture()
{
	jobject obj=0;
	m_RenderLock.Lock();
	if(m_pRender!=NULL){
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   obj=pRender->GetSurfaceTexture();
	}
	m_RenderLock.Unlock();
	return obj;
}
void CAndKKPlayerUI::OnSurfaceTextureFrameAailable()
{
	m_RenderLock.Lock();
	if(m_pRender!=NULL){
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   pRender-> setFrameAvailable(true);
	}
	m_RenderLock.Unlock();
}
void CAndKKPlayerUI::SetKeepRatio(int KeepRatio)
{
	m_RenderLock.Lock();
	if(m_pRender!=NULL){
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   pRender->SetKeepRatio(KeepRatio);
	}
	m_RenderLock.Unlock();
}
void  CAndKKPlayerUI::Pause()
{
    m_player.Pause();
}
void  CAndKKPlayerUI::Seek(int value)
{
    m_player.AVSeek(value);
}





MEDIA_INFO CAndKKPlayerUI::GetMediaInfo()
{
   MEDIA_INFO  info;
   memset(&info,0,sizeof(MEDIA_INFO));
   if(m_player.GetMediaInfo(info))
   {
   }else{
	   info.serial1=-1;
   }
   return info;
}
int  CAndKKPlayerUI::OpenMedia(char *str)
{
    CloseMedia();
    m_Audio.CloseAudio();
    LOGI(" CAndKKPlayerUI %s,%d\n",str, m_playerState);
    if( m_playerState<=-1)
    {
        m_bNeedReconnect=false;
        m_player.CloseMedia();
        m_playerState=m_player.OpenMedia(str);
    }
    return m_playerState;
}


void CAndKKPlayerUI::renderFrame(ANativeWindow* surface)
{
   if(m_nRenderType==0){
	   GlViewRender();
   }else if(m_nRenderType==1){
	      SurfaceViewRender(surface);
   }
}

void CAndKKPlayerUI::SurfaceViewRender(ANativeWindow* surface)
{
	
}

void CAndKKPlayerUI::AVRender()
{
	m_RenderLock.Lock();
	m_nRefreshPic=1;
	/*if(m_pRender!=NULL){
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   pRender->GlViewRender(m_nRefreshPic);
	}
	 m_nRefreshPic=0;*/
	m_RenderLock.Unlock();
}
IkkRender* CAndKKPlayerUI::GetRender()
{
	if(m_pRender!=NULL){
	  return m_pRender;
	}
	//m_RenderLock.Unlock();
}
void CAndKKPlayerUI::GlViewRender()
{
	
	m_RenderLock.Lock();
	if(m_pRender!=NULL){
	   bool bSurfaceDisplay=m_player.IsMediacodecSurfaceDisplay();
	   GlEs2Render*  pRender =(GlEs2Render* )m_pRender;
	   pRender->GlViewRender(m_nRefreshPic,bSurfaceDisplay);
	   m_nRefreshPic=0;
	}
	m_RenderLock.Unlock();
}
bool CAndKKPlayerUI::GetNeedReconnect()
{
    return  m_bNeedReconnect;
}
int CAndKKPlayerUI::GetPlayerState()
{
    return m_playerState;
}


unsigned char* CAndKKPlayerUI::GetWaitImage(int &length,int curtime)
{
    return NULL;
}
unsigned char*  CAndKKPlayerUI::GetCenterLogoImage(int &length)
{
    return NULL;
}
unsigned char* CAndKKPlayerUI::GetErrImage(int &length,int ErrType)
{
	length=0;
	return NULL;
}


unsigned char* CAndKKPlayerUI::GetBkImage(int &length)
{
    return NULL;
}
void CAndKKPlayerUI::OpenMediaStateNotify(char *strURL,EKKPlayerErr err)
{
    LOGE("Open Err %d \n",m_playerState);
	m_playerState=(int)err;
	PlayerStateNotifyMgr.PutAState((int)this,(int)err);
    //
    return;
}
int CAndKKPlayerUI::PreOpenUrlCallForSeg(char *InOutUrl,int *AvIsSeg,int *Interrupt)
{
	
	return 0;
}
//得到延迟
int  CAndKKPlayerUI::GetRealtimeDelay()
{
    return m_player.GetRealtimeDelay();
}
int  CAndKKPlayerUI::SetMinRealtimeDelay(int value)
{
    return  m_player.SetMaxRealtimeDelay(value);
}
//强制刷新Que
void  CAndKKPlayerUI::ForceFlushQue()
{
    m_player.ForceFlushQue();
}
void  CAndKKPlayerUI::GetNextAVSeg(void *playerIns,int Stata,int quesize,KKPlayerNextAVInfo &NextInfo)
{
     memset(&NextInfo,0,sizeof(KKPlayerNextAVInfo));	 
	 ///文件已经读取完毕
	if(Stata==AVERROR_EOF&&quesize==-1 ){
	

	}
}
void  CAndKKPlayerUI::AVReadOverThNotify(void *playerIns)
{
	
}

int  CAndKKPlayerUI::CloseMedia()
{
    m_player.CloseMedia();
    m_playerState=-1;
    return 0;
}





int CAndKKPlayerUI::GetIsReady()
{
    return  m_player.GetIsReady();
}
int CAndKKPlayerUI::GetRealtime()
{
    return  m_player.GetRealtime();
}
