/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
//http://www.2cto.com/kf/201504/390386.html mediacodec aac ���롣

//#include "kk2dop.h"
#include "KKPlayer.h"
#include "KKInternal.h"
#include "rtmp/libRtmpPlugin.h"
#include "MD5/md5.h"
#include <string>
#define MaxTimeOutStr "50000000"
#define MaxTimeOut    45
static AVPacket flush_pkt;
static int decoder_reorder_pts = -1;
static int framedrop = -1;
static int fast = 0;
static int lowres = 0;
static int64_t sws_flags = SWS_BICUBIC;
static int av_sync_type =AV_SYNC_AUDIO_MASTER;//AV_SYNC_EXTERNAL_CLOCK;//AV_SYNC_AUDIO_MASTER;//AV_SYNC_VIDEO_MASTER;// AV_SYNC_AUDIO_MASTER;
double rdftspeed = 0.02;
char**  KKCommandLineToArgv(const char* CmdLine,int* _argc);
//extern AVPixelFormat DstAVff;//=AV_PIX_FMT_YUV420P;//AV_PIX_FMT_BGRA;
//�����BGRA��ʽ
void KKPlayer::SetBGRA()
{
	m_DstAVff=AV_PIX_FMT_BGRA;
}
bool KKPlayer::GrabAvPicBGRA(void* buf,int len,int &w,int &h,bool keepscale)
{
	bool Ok=false;

	if(len<w*h*4)
		return false;

	
	if(pVideoInfo!=NULL&&pVideoInfo->pFormatCtx!=NULL&&pVideoInfo->video_st!=NULL&&buf!=NULL){
		   pVideoInfo->pictq.mutex->Lock();
		   SKK_Frame *vp =frame_queue_peek_last(&pVideoInfo->pictq);
		  
		   if(vp!=NULL&&vp->Bmp.data[0]!=NULL&&vp->width>0)
			{
				if(keepscale)
				{
				   h= vp->height*w/vp->width;
				}
				AVPixelFormat srcFF=pVideoInfo->DstAVff;
				SwsContext * imgctx = NULL;
				imgctx = sws_getCachedContext( imgctx ,
				   vp->width,  vp->height ,srcFF ,
				   w,       h,               AV_PIX_FMT_BGRA,                
				 SWS_FAST_BILINEAR,
				 NULL, NULL, NULL);

				if( imgctx  !=NULL){
					 AVPicture               Bmp;
					 avpicture_fill((AVPicture *)&Bmp, ( uint8_t *)buf,AV_PIX_FMT_BGRA, w,h);
					 sws_scale( imgctx ,vp->Bmp.data, vp->Bmp.linesize,0,vp->height,Bmp.data, Bmp.linesize);
					 Ok=true;
					 sws_freeContext( imgctx );
					 imgctx=NULL;
				}
				
			}
			pVideoInfo->pictq.mutex->Unlock();
	}
	return Ok;
///DstAVff
}
//������尲׿ƽ̨��
#ifdef Android_Plat
void* kk_jni_attach_env();
int kk_jni_detach_env();
void  KKPlayer::SetViewSurface(void* surface)
{
   m_pViewSurface=surface;
   LOGE_KK("ViewSurface= %d \n",(int)surface);  
}
bool KKPlayer::IsMediacodecSurfaceDisplay()
{
	return m_bSurfaceDisplay;
    
}
#endif
std::list<KKPluginInfo>  KKPlayer::KKPluginInfoList;
void KKPlayer::AddKKPluginInfo(KKPluginInfo& info)
{
     KKPluginInfoList.push_back(info);
}
std::list<KKPluginInfo>&   KKPlayer::GetKKPluginInfoList()
{
	return KKPluginInfoList;
}
void register_Kkv();
KKPlayer::KKPlayer(IKKPlayUI* pPlayUI,IKKAudio* pSound):m_pSound(pSound),m_pPlayUI(pPlayUI),m_nPreFile(false)
,m_pAudioPicBuf(NULL)
,m_AudioPicBufLen(0)
,m_bRender(1)
,m_bLastOpenAudio(0)
,m_AvIsSeg(0)
,m_CacheAvCounter(0)
,m_TotalTime(0)
,start_time(AV_NOPTS_VALUE)
,m_CurTime(0)
,pVideoInfo(0)
,m_bOpen(false)
,m_nhasVideoAudio(0)
,m_DstAVff(AV_PIX_FMT_YUV420P)
,m_pKKPlayerGetUrl(NULL)
{
	
	static bool registerFF=true;
	if(registerFF)
	{
		 
		LOGE_KK("len:%d \n",strlen("����"));
		/*std::string strPort="";
		const char *pp=strPort.c_str();
		char* const aaa=(char* const)pp;*/
		av_register_all();
		avfilter_register_all();
		avformat_network_init();
       // register_Kkv();
		
		registerFF=false;
		AddlibRtmpPluginInfo();
	}
	
#ifdef Android_Plat
	m_pVideoRefreshJNIEnv=NULL;
	m_pViewSurface=0;
	m_bSurfaceDisplay=0;
#endif
#ifdef PRINT_CODEC_INFO
	AVInputFormat *ff=av_iformat_next(NULL);
	AVCodec *codec=av_codec_next(NULL);
    while(codec!=NULL)
	{
		
		LOGE_KK("%s \n",codec->name);
		codec=av_codec_next(codec);
    }
	AVHWAccel *hwaccel=av_hwaccel_next(NULL);
	while(hwaccel!=NULL)
	{
		const char *aa=hwaccel->name;
		LOGE_KK("%d,%s \n",i++,aa);
		hwaccel=av_hwaccel_next(hwaccel);
	}
#endif
	

	  
	 /*const  AVPixFmtDescriptor *frist=av_pix_fmt_desc_next(NULL);
     while(frist!=NULL)
	{
		
		LOGE_KK("pfm  %s \n",frist->name);
		frist=av_pix_fmt_desc_next(frist);
    }*/
	
     
}
KKPlayer::~KKPlayer(void)
{

}
void FreeKKIo(SKK_VideoState *kkAV);
void WillCloseKKIo(AVIOContext *io);
void KKPlayer::CloseMedia()
{
	m_AVInfoLock.Lock();
    if(pVideoInfo!=NULL)
	{
	    pVideoInfo->abort_request=1;
	}
	
	
	while(m_nPreFile==1)
	{
		m_AVInfoLock.Unlock();
		Sleep(10);
		m_AVInfoLock.Lock();
		LOGE_KK(" xx\n");
	}
	
	if(pVideoInfo!=NULL)
	{
		pVideoInfo->abort_request=1;
	}

	int pllx=1;
	while(m_nPreFile==2)
	{
		m_AVInfoLock.Unlock();
		if(pllx&&pVideoInfo->pFormatCtx!=NULL&&pVideoInfo->ioflags==AVFMT_FLAG_CUSTOM_IO)
		{
			WillCloseKKIo(pVideoInfo->pFormatCtx->pb);
				pllx=0;
		}
		Sleep(20);
		m_AVInfoLock.Lock();
		LOGE_KK("m_nPreFile=%d xx2\n",m_nPreFile);
	}
	m_nPreFile=0;
    m_AVInfoLock.Unlock();


	m_PlayerLock.Lock();
	if(!m_bOpen)
	{
		m_PlayerLock.Unlock();
		return;
	}
	if(m_pSound!=NULL)
       m_pSound->Stop();
	
	if(pVideoInfo==NULL) 
	{
		m_bOpen=false;
		m_PlayerLock.Unlock();
		return;
	}
   
	if(pllx&&pVideoInfo->pFormatCtx!=NULL&&pVideoInfo->ioflags==AVFMT_FLAG_CUSTOM_IO)
	{
		WillCloseKKIo(pVideoInfo->pFormatCtx->pb);
			pllx=0;
	}

	m_PlayerLock.Unlock();
	while(1)
	{
		if(m_ReadThreadInfo.ThOver==true&&m_VideoRefreshthreadInfo.ThOver==true&&m_AudioCallthreadInfo.ThOver==true)
		{
			break;
		}
		LOGE_KK("thread Over1 m_ReadThreadInfo%d,m_VideoRefreshthreadInfo%d \n",m_ReadThreadInfo.ThOver
			,m_VideoRefreshthreadInfo.ThOver
			);
		 av_usleep(5000);
	}


	LOGE_KK("thread Over 1");
	
	m_PlayerLock.Lock();
	#ifndef WIN32_KK
			pthread_join(m_ReadThreadInfo.Tid_task,0);
			pthread_join(m_VideoRefreshthreadInfo.Tid_task,0);
	#endif	

	pVideoInfo->videoq.m_pWaitCond->SetCond();
	pVideoInfo->audioq.m_pWaitCond->SetCond();
	pVideoInfo->subtitleq.m_pWaitCond->SetCond();

	pVideoInfo->pictq.m_pWaitCond->SetCond();
	pVideoInfo->sampq.m_pWaitCond->SetCond();
	pVideoInfo->subpq.m_pWaitCond->SetCond();/**/

	
	while(1)
	{
		if(pVideoInfo->viddec.decoder_tid.ThOver==true&&pVideoInfo->auddec.decoder_tid.ThOver==true&&pVideoInfo->subdec.decoder_tid.ThOver==true)
		{
			break;
		}
		LOGE_KK("thread Over2 viddec%d,auddec%d,subdec%d \n",pVideoInfo->viddec.decoder_tid.ThOver
			,pVideoInfo->auddec.decoder_tid.ThOver
			,pVideoInfo->subdec.decoder_tid.ThOver
			);
		if(!pVideoInfo->viddec.decoder_tid.ThOver){
			pVideoInfo->videoq.m_pWaitCond->SetCond();
			pVideoInfo->pictq.m_pWaitCond->SetCond();
		}
		 av_usleep(5000);
	}

	LOGE_KK("thread Over 2 \n");
	
#ifdef WIN32_KK
	//SDL_CloseAudio();
	//�رն�ȡ�߳�
	if(m_ReadThreadInfo.Addr!=0)
	   ::TerminateThread(m_ReadThreadInfo.ThreadHandel,0);
	::CloseHandle(m_ReadThreadInfo.ThreadHandel);
	m_ReadThreadInfo.ThreadHandel=0;
    
	//�ر�ˢ���߳�
	if(m_VideoRefreshthreadInfo.Addr!=0)
	   ::TerminateThread(m_VideoRefreshthreadInfo.ThreadHandel,0);
	::CloseHandle(m_VideoRefreshthreadInfo.ThreadHandel);
	m_VideoRefreshthreadInfo.ThreadHandel=0;
	
	//�ر���ؽ����߳�

	if(pVideoInfo->viddec.decoder_tid.Addr!=0)
	    ::TerminateThread(pVideoInfo->viddec.decoder_tid.ThreadHandel,0);
	::CloseHandle(pVideoInfo->viddec.decoder_tid.ThreadHandel);
	

	if(pVideoInfo->auddec.decoder_tid.Addr!=0)
	    ::TerminateThread(pVideoInfo->auddec.decoder_tid.ThreadHandel,0);
	::CloseHandle(pVideoInfo->auddec.decoder_tid.ThreadHandel);
	
	if(pVideoInfo->subdec.decoder_tid.Addr!=0)
	    ::TerminateThread(pVideoInfo->subdec.decoder_tid.ThreadHandel,0);
	::CloseHandle(pVideoInfo->subdec.decoder_tid.ThreadHandel);
#else
        
	if(pVideoInfo->viddec.decoder_tid.Addr!=0)
	    pthread_kill(pVideoInfo->viddec.decoder_tid.Tid_task,0);
	
	if(pVideoInfo->auddec.decoder_tid.Addr!=0)
	    pthread_kill(pVideoInfo->auddec.decoder_tid.Tid_task,0);
	
	if(pVideoInfo->subdec.decoder_tid.Addr!=0)
	     pthread_kill(pVideoInfo->subdec.decoder_tid.Tid_task,0);
#endif	
	
    /*******�¼�*********/
	//��Ƶ��
	delete pVideoInfo->videoq.m_pWaitCond;
	//��Ƶ��
	delete pVideoInfo->audioq.m_pWaitCond;
	//��Ļ��
	delete pVideoInfo->subtitleq.m_pWaitCond;

	delete pVideoInfo->pictq.m_pWaitCond;
	delete pVideoInfo->subpq.m_pWaitCond;
	delete pVideoInfo->sampq.m_pWaitCond;

	LOGE_KK("PacketQueuefree1 \n");
	PacketQueuefree();
	LOGE_KK("PacketQueuefree OK \n");

	delete pVideoInfo->videoq.pLock;
	pVideoInfo->videoq.pLock=NULL;
	delete pVideoInfo->subtitleq.pLock;
	pVideoInfo->subtitleq.pLock=NULL;
	delete pVideoInfo->audioq.pLock;
	pVideoInfo->audioq.pLock=NULL;
	
	
	LOGE_KK("pVideoInfo->pictq.mutex OK \n");
	delete pVideoInfo->pictq.mutex;
	pVideoInfo->pictq.mutex=NULL;
	delete pVideoInfo->subpq.mutex;
	pVideoInfo->subpq.mutex=NULL;
	delete pVideoInfo->sampq.mutex;
	pVideoInfo->sampq.mutex=NULL;

	
	if(pVideoInfo->swr_ctx!=NULL)
	{
	   swr_free(&pVideoInfo->swr_ctx);
	   pVideoInfo->swr_ctx=NULL;
	   LOGE_KK("swr_free OK \n");
	}

	if(pVideoInfo->img_convert_ctx!=NULL)
	{
		sws_freeContext(pVideoInfo->img_convert_ctx);
		pVideoInfo->img_convert_ctx=NULL;
		LOGE_KK("pVideoInfo->img_convert_ctx \n");
	}
	


	if(pVideoInfo->sub_convert_ctx!=NULL)
	{
		sws_freeContext(pVideoInfo->sub_convert_ctx);
		pVideoInfo->sub_convert_ctx=NULL;
		LOGE_KK("sub_convert_ctx \n");
	}
	


	if(pVideoInfo->InAudioSrc!=NULL)
	{
	    avfilter_free(pVideoInfo->InAudioSrc);
        LOGE_KK("InAudioSrc \n");
	}
	if(pVideoInfo->OutAudioSink!=NULL)
	{
	    avfilter_free(pVideoInfo->OutAudioSink);
	    LOGE_KK("OutAudioSink \n");
	}
	if(pVideoInfo->AudioGraph!=NULL)
	{
		avfilter_graph_free(&pVideoInfo->AudioGraph);
		LOGE_KK("pVideoInfo->AudioGraph \n");
	}
	
	if(pVideoInfo->auddec.avctx!=NULL){
		 LOGE_KK("avcodec_close(pVideoInfo->auddec.avctx); OK \n");
		 avcodec_close(pVideoInfo->auddec.avctx);
		 avcodec_free_context(&pVideoInfo->auddec.avctx);
		 av_freep(&pVideoInfo->audio_buf1);
	}
	
	if(pVideoInfo->subdec.avctx!=NULL)
	{
		 LOGE_KK("avcodec_close(pVideoInfo->subdec.avctx); OK \n");
	   avcodec_close(pVideoInfo->subdec.avctx);
	   avcodec_free_context(&pVideoInfo->subdec.avctx);
	}

	if(pVideoInfo->viddec.avctx!=NULL)
	{
	   LOGE_KK("avcodec_close(pVideoInfo->viddec.avctx); \n");
	   avcodec_close(pVideoInfo->viddec.avctx);
	   avcodec_free_context(&pVideoInfo->viddec.avctx);
	   LOGE_KK("avcodec_close(avcodec_free_context(&pVideoInfo->viddec.avctx) OK \n");
	}

	if(pVideoInfo->pFormatCtx!=NULL&&pVideoInfo->ioflags==AVFMT_FLAG_CUSTOM_IO)
	{
		FreeKKIo(pVideoInfo);
		pVideoInfo->pFormatCtx->pb=NULL;
	}

	LOGE_KK("pVideoInfo->pFormatCtx %d,%d \n",pVideoInfo->pFormatCtx,pVideoInfo->pSegFormatCtx);
	if(pVideoInfo->pFormatCtx!=NULL){
	    avformat_close_input(&pVideoInfo->pFormatCtx);
	    pVideoInfo->pFormatCtx=NULL;
	}
	if(pVideoInfo->pSegFormatCtx!=NULL){
		avformat_close_input(&pVideoInfo->pSegFormatCtx);
		pVideoInfo->pSegFormatCtx=NULL;
	}
   
	if(pVideoInfo->pKKPluginInfo!=NULL){
	   LOGE_KK("pVideoInfo->pKKPluginInfo\n");
	   KK_Free_(pVideoInfo->pKKPluginInfo);
	}

    av_packet_unref(pVideoInfo->pflush_pkt);
	KK_Free_(pVideoInfo->pflush_pkt);
	KK_Free_(pVideoInfo);
	pVideoInfo=NULL;

	start_time=AV_NOPTS_VALUE;
	m_CurTime=0;
    m_TotalTime=0;

	if(m_pAudioPicBuf!=NULL){
		m_AudioPicBufLen=0;
		KK_Free_(m_pAudioPicBuf);
		m_pAudioPicBuf=NULL;
	}


	m_nhasVideoAudio=0;
	m_bOpen=false;
	m_PlayerLock.Unlock();

	#ifdef Android_Plat
	    m_pVideoRefreshJNIEnv=NULL;
		 m_bSurfaceDisplay=false;
		 m_pViewSurface=0;
    #endif
	LOGE_KK("KKplay Over\n");
}

//��ȡý�岥�ŵ���Ϣ
bool KKPlayer::GetMediaInfo(MEDIA_INFO &info)
{
	bool ok=false;
	if(m_PlayerLock.TryLock()){
		if(m_bOpen&&pVideoInfo!=NULL&&m_nPreFile==3&&!pVideoInfo->abort_request){	
						
						info.Open=m_bOpen;
						
							info.CurTime=m_CurTime;
							if(pVideoInfo->pFormatCtx!=NULL)
							{
								            info.SegId=pVideoInfo->cursegid;
											info.AvFile=(const char*)pVideoInfo->filename;
											info.TotalTime=(pVideoInfo->pFormatCtx->duration/1000/1000);
											
											
											info.CacheInfo.AudioSize=m_AVCacheInfo.AudioSize;
											info.CacheInfo.VideoSize=m_AVCacheInfo.VideoSize;
											info.CacheInfo.MaxTime=m_AVCacheInfo.MaxTime;
										
											info.FileSize=pVideoInfo->fileSize;
										
											snprintf(info.AVRes,1024,"%dx%d",pVideoInfo->viddec_width,pVideoInfo->viddec_height);
											char infostr[1024]="";
											if(pVideoInfo->viddec.avctx!=NULL){

												memset(&info.videoinfo.codecname,0,32);
												strcpy(info.videoinfo.codecname, pVideoInfo->viddec.avctx->codec->name);
												info.videoinfo.bitrate= pVideoInfo->video_st->codecpar->bit_rate;
												//info.videoinfo.bitrate= pVideoInfo->viddec.avctx->bit_rate;
												info.videoinfo.framerate= pVideoInfo->viddec.avctx->framerate.num;
												
												if(pVideoInfo->auddec.avctx==NULL)
												{
													info.serial=pVideoInfo->viddec.pkt_serial;
													info.serial1=pVideoInfo->video_clock_serial;
												}
											}
											if(pVideoInfo->auddec.avctx!=NULL)
											{
                                                info.serial1=pVideoInfo->audio_clock_serial;
												info.serial=pVideoInfo->auddec.pkt_serial;
												strcpy(info.audioinfo.codecname,pVideoInfo->auddec.avctx->codec->name);
												
												info.audioinfo.bitrate= pVideoInfo->auddec.avctx->bit_rate;
											

												info.audioinfo.sample_rate=pVideoInfo->auddec.avctx->sample_rate;
												info.audioinfo.channels=pVideoInfo->auddec.avctx->channels;
											}
											if(pVideoInfo->seek_req!=0||m_nSeekTime!=0)
											{
											    info.serial=-1;
											}

											
							}
							m_AVPlayInfo=info;
							ok=true;
				
		      }
		m_PlayerLock.Unlock();/**/
	}else{
	    info=m_AVPlayInfo;
	}
	
	///�Զ���Io
	if(pVideoInfo!=NULL&&pVideoInfo->pFormatCtx!=NULL&&pVideoInfo->ioflags == AVFMT_FLAG_CUSTOM_IO){
	     if(pVideoInfo->pKKPluginInfo!=NULL&&pVideoInfo->pKKPluginInfo->KKDownAVFileSpeedInfo!=NULL){
			 char infostr[1024]="";
			 pVideoInfo->pKKPluginInfo->KKDownAVFileSpeedInfo(pVideoInfo->filename,infostr,1024);
			 strcpy(info.SpeedInfo,infostr);
		 }
	}/**/
	return ok;
}


void KKPlayer::SetWindowHwnd(HWND hwnd)
{
	m_hwnd=hwnd;
	//m_pSound->SetWindowHAND((int)m_hwnd);
	
}
///����URL�滻����
void           KKPlayer::SetKKPlayerGetUrl(fpKKPlayerGetUrl pKKPlayerGetUrl)
{
    m_pKKPlayerGetUrl=pKKPlayerGetUrl;
}
unsigned __stdcall  KKPlayer::ReadAV_thread(LPVOID lpParameter)
{
	KKPlayer *pPlayer=(KKPlayer *  )lpParameter;
	pPlayer->m_ReadThreadInfo.ThOver=false;
	pPlayer->ReadAV();
	
	pPlayer->m_ReadThreadInfo.ThOver=true;
    pPlayer->m_ReadThreadInfo.Addr=0;
	return 1;
}


#ifdef WIN32_KK
int index=0;
SYSTEMTIME Time_tToSystemTime(time_t t)
{
	tm temptm = *localtime(&t);
	SYSTEMTIME st = {1900 + temptm.tm_year, 
		1 + temptm.tm_mon, 
		temptm.tm_wday, 
		temptm.tm_mday, 
		temptm.tm_hour, 
		temptm.tm_min, 
		temptm.tm_sec, 
		0};
	return st;
}
#endif
//http://120.25.236.44:9999/1/test.m3u8
//rtmp://117.135.131.98/771/003 live=1
void KKPlayer::video_image_refresh(SKK_VideoState *is)
{
	
	
    double time=0,duration=0;
	if(is->audio_st)
	    m_CurTime=is->audio_clock;
	else if(is->video_st)
		m_CurTime=is->vidclk.pts;
	else
		m_CurTime=get_master_clock(is);
	if (is->video_st==NULL&&is->audio_st) {
		time = av_gettime_relative() / 1000000.0;
		if (is->force_refresh || is->last_vis_time + rdftspeed < time)
		{
			
			is->last_vis_time = time;
		}
		is->remaining_time = FFMIN(is->remaining_time, is->last_vis_time + rdftspeed - time);
		is->force_refresh=1;
	}

	
	if(is->video_st)
    {
retry:
			//û������
			if (frame_queue_nb_remaining(&is->pictq) <= 0)
			{
			   goto display;
			}
			

			SKK_Frame *vp;
			
		
			//��ȡ��һ�εĶ�ȡλ��
			SKK_Frame *lastvp = frame_queue_peek_last(&is->pictq);

			
			/**********��ȡ��λ��**********/
			vp = frame_queue_peek(&is->pictq);
			is->video_clock=vp->pts;
			if(is->realtime&&is->audio_st==NULL&&!is->abort_request){
				if(pVideoInfo->nRealtimeDelay>0)
				{
				
					pVideoInfo->nRealtimeDelay-=(vp->pts-lastvp->pts);
					//is->nMinRealtimeDelay=3;
					if(pVideoInfo->nRealtimeDelay>is->nMaxRealtimeDelay)
					{
						frame_queue_next(&is->pictq,true);
						is->nRealtimeDelayCount++;
						goto retry;
						//goto retry:
					}else
						is->nRealtimeDelayCount=0;
				}
			}
			

			/*if (vp->serial != is->videoq.serial) {
				frame_queue_next(&is->pictq,true);
				update_video_pts(is, vp->pts, vp->pos, vp->serial);
				is->redisplay=0;
				goto retry;
			}
			*/
			
				
				/*******ʱ��**********/
				if (lastvp->serial != vp->serial && !is->redisplay)
				{
					is->frame_timer = av_gettime_relative() / 1000000.0;
				}

				if (is->paused)
					goto display;
				//is->frame_timer += delay;
				/******��һ�θ��º���һ��ʱ��Ĳ�ֵ��ͼƬ֮���ֵ******/
				is->last_duration = vp_duration(is, lastvp, vp);/******pts-pts********/

				if (is->redisplay)
					is->delay= 0.0;
				else/**/
				is->delay = compute_target_delay(is->last_duration, is);

#ifdef WIN32
				char abcdx[102]="";
				sprintf_s(abcdx,102,"is->delay:%f \n",is->delay);
				OutputDebugStringA(abcdx);

#endif
				time= av_gettime_relative()/1000000.0;
				if (time < is->frame_timer + is->delay&&!is->redisplay) {
					double llxxxx=is->frame_timer + is->delay - time;
					is->remaining_time = FFMIN(llxxxx, is->remaining_time);
					goto display;
				}
	
				is->frame_timer += is->delay;
				if (is->delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
					is->frame_timer = time;


				if(!isNAN(vp->pts)&&!is->redisplay)
				{
					update_video_pts(is, vp->pts, vp->pos, vp->serial);
				}
				if (frame_queue_nb_remaining(&is->pictq) > 1)
				{
					SKK_Frame *nextvp = frame_queue_peek_next(&is->pictq);
					duration = vp_duration(is, vp, nextvp);
					if((is->redisplay ||(get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration)
					{
						is->frame_drops_late++;
						frame_queue_next(&is->pictq,true);
						is->redisplay=0;
						goto retry;
					}
				}
				
				if (vp->buffer)
				{
					    int total=(pVideoInfo->pFormatCtx->duration/1000/1000);
						if(start_time==AV_NOPTS_VALUE)
						{
						  start_time=vp->pts;
						}
				}

				if (is->subtitle_st) {
					SKK_Frame *sp=0, *sp2=0;
                    while (frame_queue_nb_remaining(&is->subpq) > 0) {
                        sp = frame_queue_peek(&is->subpq);

                        if (frame_queue_nb_remaining(&is->subpq) > 1)
                            sp2 = frame_queue_peek_next(&is->subpq);
                        else
                            sp2 = NULL;

                        if (sp->serial != is->subtitleq.serial
                                || (is->vidclk.pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
                                || (sp2 && is->vidclk.pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                        {
                            if (sp->uploaded)
							{
                               /* int i;
                                for (i = 0; i < sp->sub.num_rects; i++)
								{
                                    AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                    uint8_t *pixels;
                                    int pitch, j;
                                    }
                                }*/
                            }
                            frame_queue_next(&is->subpq,true);
                        } else {
                            break;
                        }
                    }
                }

				frame_queue_next(&is->pictq,true);
				is->force_refresh=1;
	}
display:
	if(is->force_refresh&&m_pPlayUI!=NULL)
	{		
		m_pPlayUI->AVRender();
	}
	if(is->subtitle_st!=NULL)
	{

	}
	is->force_refresh=0;
}

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a%b + b : a%b;
}
///��Ⱦ��Ƶ����
void KKPlayer::video_audio_display(IkkRender *pRender,SKK_VideoState *s)
{
//	return ;
	int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
    int ch, channels, h, h2;
	unsigned int bgcolor, fgcolor;
    int64_t time_diff;
    int rdft_bits, nb_freq;
	int height=300;
	int width=700;
	if(m_pAudioPicBuf==NULL){
	    m_AudioPicBufLen=avpicture_get_size(AV_PIX_FMT_BGRA,width, height);
	    m_pAudioPicBuf=(uint8_t *)KK_Malloc_(m_AudioPicBufLen);
	}
    for (rdft_bits = 1; (1 << rdft_bits) < 2 * height; rdft_bits++)
        ;
    nb_freq = 1 << (rdft_bits - 1);

    channels = s->audio_tgt.channels;
    nb_display_channels = channels;
    if (!s->paused) {
		int data_used= s->show_mode ==  SKK_VideoState::SHOW_MODE_WAVES ? width : (2*nb_freq);
        n = 2 * channels;
        delay = s->audio_write_buf_size;
        delay /= n;

        /* to be more precise, we take into account the time spent since
           the last buffer computation */
        if (s->audio_callback_time) {
            time_diff = av_gettime_relative() - s->audio_callback_time;
            delay -= (time_diff * s->audio_tgt.freq) / 1000000;
        }

        delay += 2 * data_used;
        if (delay < data_used)
            delay = data_used;

        i_start= x = compute_mod(s->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
        if (1||s->show_mode == SKK_VideoState::SHOW_MODE_WAVES) {
            h = INT_MIN;
            for (i = 0; i < 1000; i += channels) {
                int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
                int a = s->sample_array[idx];
                int b = s->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
                int c = s->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
                int d = s->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
                int score = a - d;
                if (h < score && (b ^ c) < 0) {
                    h = score;
                    i_start = idx;
                }
            }
        }

        s->last_i_start = i_start;
    } else {
        i_start = s->last_i_start;
    }

	if(pRender!=NULL)
	{
				kkBitmap img;
				img.pixels=m_pAudioPicBuf;
				img.width=width;
				img.height=height;
				kkRect rt={0,0,img.width,img.height};
				bgcolor = kkRGB(0,0,0);
				//pRender->FillRect(img,rt,bgcolor);
		        fgcolor =kkRGB(0xff, 0xff, 0xff);
				
               //if(0)
				{
				   nb_display_channels= FFMIN(nb_display_channels, 2);
					if (rdft_bits != s->rdft_bits) {
						av_rdft_end(s->rdft);
						av_free(s->rdft_data);
						s->rdft = av_rdft_init(rdft_bits, DFT_R2C);
						s->rdft_bits = rdft_bits;
						s->rdft_data =(FFTSample *) av_malloc_array(nb_freq, 4 *sizeof(*s->rdft_data));
					}
					if (!s->rdft || !s->rdft_data){
						av_log(NULL, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
						//s->show_mode = SHOW_MODE_WAVES;
					} else {
						FFTSample *data[2];
						for (ch = 0; ch < nb_display_channels; ch++) {
							data[ch] = s->rdft_data + 2 * nb_freq * ch;
							i = i_start + ch;
							for (x = 0; x < 2 * nb_freq; x++) {
								double w = (x-nb_freq) * (1.0 / nb_freq);
								data[ch][x] = s->sample_array[i] * (1.0 - w * w);
								i += channels;
								if (i >= SAMPLE_ARRAY_SIZE)
									i -= SAMPLE_ARRAY_SIZE;
							}
							av_rdft_calc(s->rdft, data[ch]);
						}
						/* Least efficient way to do this, we should of course
						 * directly access it but it is more than fast enough. */
						for (y = 0; y < height; y++) {
							double w = 1 / sqrt((double)nb_freq);
							int a = sqrt(w * hypot(data[0][2 * y + 0], data[0][2 * y + 1]));
							int b = (nb_display_channels == 2 ) ? sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))
																: a;
							a = FFMIN(a, 255);
							b = FFMIN(b, 255);
							fgcolor =kkRGB( a, b, (a + b) / 2);
							kkRect rt2={s->xpos,height-y, s->xpos+1,height-y+ 1};
						    pRender->FillRect(img,rt2, fgcolor);
						}
					}
				}


                if(s->img_convert_ctx ==NULL)
				s->img_convert_ctx = sws_getCachedContext(s->img_convert_ctx,width, height ,AV_PIX_FMT_BGRA,
					            width, height,               s->DstAVff,                
			                    SWS_FAST_BILINEAR,
			                    NULL, NULL, NULL);
				 if (s->img_convert_ctx == NULL) 
				 {
					 if(s->bTraceAV)
					 fprintf(stderr, "Cannot initialize the conversion context\n");
					 assert(0);
					 
				 }
			     AVPicture  InBmp; 
				 avpicture_fill((AVPicture *)&InBmp,(uint8_t *) m_pAudioPicBuf,AV_PIX_FMT_BGRA, width,height);

				 AVPicture  OutBmp; 
				 int numBytes=avpicture_get_size(s->DstAVff,width,height); //pFrame->width,pFrame->height
		         static   uint8_t * OutBmpbuffer=(uint8_t *)KK_Malloc_(numBytes)+100;
                 avpicture_fill((AVPicture *)&OutBmp, OutBmpbuffer,s->DstAVff, width,height);

			      sws_scale(s->img_convert_ctx, InBmp.data, InBmp.linesize,0,height,
					 OutBmp.data,OutBmp.linesize);
				  ///pRender->render((char*) OutBmpbuffer,width,height,width,false);

                  KK_Free_(OutBmpbuffer);

				  if (!s->paused)
							s->xpos++;
				  if (s->xpos >= width)
							s->xpos= 0;
		           s->remaining_time=0.1;				
	}
	
}
int KKPlayer::GetPktSerial()
{
	return m_PktSerial;
}
short KKPlayer::GetSegId()
{
	m_PlayerLock.Lock();
	if(pVideoInfo!=NULL)
	{
		m_PlayerLock.Unlock();
		return pVideoInfo->segid;
	}
	m_PlayerLock.Unlock();
	return -1;

}
short KKPlayer::GetCurSegId()
{
	m_PlayerLock.Lock();
	if(pVideoInfo!=NULL)
	{
		m_PlayerLock.Unlock();
	    return pVideoInfo->cursegid;
	}
	m_PlayerLock.Unlock();
	return -1;
}

void KKPlayer::VideoRefresh()
{
	
	if(pVideoInfo->IsReady==0||pVideoInfo->paused)
	{
		return;
	}
	
	video_image_refresh(pVideoInfo);
}
int KKPlayer::GetIsReady()
{
	if(pVideoInfo!=NULL)
	    return pVideoInfo->IsReady;
	return -1;
}

void KKPlayer::RenderImage(IkkRender *pRender,bool Force)
{
	 SKK_Frame *vp;
	 if(!m_bRender)
		 return;
	 if(m_PlayerLock.TryLock()){
		 if(m_pPlayUI!=NULL){
						if(pVideoInfo==NULL){			
							int len=0;
							unsigned char* pBkImage=m_pPlayUI->GetCenterLogoImage(len);
							if(pBkImage!=NULL&&len>0){
								pRender->LoadCenterLogo(pBkImage,len);
								if(pBkImage!=NULL&&len>0){
								   pRender->renderBk(pBkImage,len);
								}
							}
						}else {
							            bool bNeedforce=false;///ǿ��ˢ��
										if(m_bOpen&&m_nPreFile!=0){ 
												if(pVideoInfo->IsReady==0){
															if(pVideoInfo->abort_request==0){
																  int len=0;
																  unsigned char* pWaitImage=m_pPlayUI->GetWaitImage(len,0);
																  if(pWaitImage!=NULL){
																	 pRender->SetWaitPic(pWaitImage,len);
																	bNeedforce=true;
																  }
															}else{
															     pRender->SetWaitPic(0,0);
															}
												  }
											
													if(pVideoInfo->video_st!=NULL){
													   pVideoInfo->pictq.mutex->Lock();
													   vp =frame_queue_peek_last(&pVideoInfo->pictq);  
													   m_lstPts=vp->pts;
													   static int iddd=0;
													    LOGE_KK("RenderImage \n");
													   iddd=vp->serial;
													   if(vp&&vp->Bmp.data[0]!=NULL&&(m_lstPts!=vp->pts||Force)||vp->picformat==AV_PIX_FMT_MEDIACODEC)
													   {
														   if(vp->picformat==(int)AV_PIX_FMT_MEDIACODEC){
														   
															   kkAVPicInfo picinfo;
															   picinfo.width=vp->width;
															   picinfo.height=vp->height;
															   picinfo.picformat=vp->picformat;
															   pRender->render(&picinfo,bNeedforce);
														   }else if(!vp->uploaded&&vp->picformat!=(int)AV_PIX_FMT_DXVA2_VLD)
														   {
															  
															   kkAVPicInfo picinfo;
															   memcpy(picinfo.data,vp->Bmp.data,32);
															   memcpy(picinfo.linesize,vp->Bmp.linesize,32);
															   picinfo.width=vp->width;
															   picinfo.height=vp->height;
															   picinfo.picformat=vp->picformat;
															  
															   pRender->render(&picinfo,bNeedforce);
															   
														   }else if(bNeedforce){
														       pRender->render(NULL,bNeedforce);
														   }
													   }else if(pVideoInfo->IsReady==0){
														   pRender->render(NULL,true);
													   }
													   pVideoInfo->pictq.mutex->Unlock();
													}else if(pVideoInfo->audio_st!=NULL&&pVideoInfo->iformat!=NULL){
				                                         #ifdef  _WINDOWS
													         video_audio_display(pRender,pVideoInfo);
				                                         #endif
													}else if(pVideoInfo->IsReady==0){
														   pRender->render(NULL,true);
													}
										}
									  
						}
		 }
	    m_PlayerLock.Unlock();	
	 }
#ifdef _WINDOWS
	 else{
			if(m_pPlayUI!=NULL&&m_bOpen&&m_nPreFile<=2){ 
						int len=0;
						unsigned char* pWaitImage=m_pPlayUI->GetWaitImage(len,0);
						if(pWaitImage!=NULL){
							 pRender->SetWaitPic(pWaitImage,len);
							 pRender->render(NULL,true);
						}
			}
	 }
#endif
}

#ifdef WIN32_KK
void KKPlayer::OnDrawImageByDc(HDC memdc)
{
	//return;
	SKK_Frame *vp;
	if(pVideoInfo->IsReady==0)
		return;
	pVideoInfo->pictq.mutex->Lock();
	/**********��ȡ��λ��**********/
	vp = frame_queue_peek(&pVideoInfo->pictq);
	//VideoDisplay(vp->buffer,pVideoInfo->width,vp->height,&memdc,pVideoInfo->last_duration,vp->pts,vp->duration,vp->pos,pVideoInfo->delay);
	pVideoInfo->pictq.mutex->Unlock();
}
void KKPlayer::VideoDisplay(void *buf,int w,int h,void *usadata,double last_duration,double pts,double duration,int64_t pos,double diff)
{
	HDC dc=*(HDC*)usadata;
	BITMAPINFOHEADER header;
	header.biSize = sizeof(BITMAPINFOHEADER);

	header.biWidth = w;
	header.biHeight = h*(-1);
	header.biBitCount = 32;
	header.biCompression = 0;
	header.biSizeImage = 0;
	header.biClrImportant = 0;
	header.biClrUsed = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biPlanes = 1;

	RECT rt;
	::GetClientRect(m_hwnd,&rt);
	int www=rt.right-rt.left;
	int hhh=rt.bottom-rt.top;

	//����ͼ��
	StretchDIBits(dc, 0,   0, 
		www,   hhh, 
		0,   0, 
		w,   h, 
		buf, (BITMAPINFO*)&header,
		DIB_RGB_COLORS, SRCCOPY);/**/

	{			
		//����
		char t[256]="";
		sprintf(t, "VPdur:%f",duration);
		int w2=0;
		RECT rt2={w2,10,w2+150,30};
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		//PTS
		memset(t,0,256);
		sprintf(t, "VPpts:%f",pts);
		rt2.top=30;
		rt2.bottom=60;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		//pos
		memset(t,0,256);
		sprintf(t, "pos:%ld",pos);
		rt2.top=60;
		rt2.bottom=90;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		memset(t,0,256);
		int64_t timer=pVideoInfo->pFormatCtx->duration/1000;
		int h=(timer/(1000*60*60));
		int m=(timer%(1000*60*60))/(1000*60);
		int s=((timer%(1000*60*60))%(1000*60))/1000;
		sprintf(t, "timer:%d:%d:%d",h,m,s);
		rt2.top=90;
		rt2.bottom=120;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		memset(t,0,256);
		sprintf(t, "Vp��ֵ:%f",last_duration);
		rt2.top=120;
		rt2.bottom=150;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		//PTS
		memset(t,0,256);
		int ll2=pts;
		h=(ll2/(60*60));
		m=(ll2%(60*60))/(60);
		s=((ll2%(60*60))%(60));
		sprintf(t, "timer:%d:%d:%d",h,m,s);
		rt2.top=150;
		rt2.bottom=180;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);
		
		//��Ƶ
		memset(t,0,256);
		sprintf(t, "audio:%.3f",pVideoInfo->audio_clock);
		rt2.top=180;
		rt2.bottom=210;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		//���
		memset(t,0,256);
		sprintf(t, "V_Adif:%.3f",abs(pts-pVideoInfo->audio_clock));
		rt2.top=210;
		rt2.bottom=240;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		double V_ADiff=pts-pVideoInfo->audio_clock+diff;
		memset(t,0,256);
		sprintf(t, "��Ƶ��:%.3f",V_ADiff);
		
		rt2.top=240;
		rt2.bottom=270;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);

		memset(t,0,256);
		sprintf(t, "delay:%.3f",diff);
		rt2.top=270;
		rt2.bottom=300;
		::SetTextColor(dc,RGB(255,255,255));
		::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);
		if(buf==NULL)
		{
		   memset(t,0,256);
		   sprintf(t, "��:%f",pts);
		   rt2.top=300;
		   rt2.bottom=330;
		   ::SetTextColor(dc,RGB(255,255,255));
		  ::DrawTextA(dc,t,strlen(t),&rt2,DT_CENTER);
		}
	}
}

#endif
//unsigned __stdcall VideoRefreshthread(LPVOID lpParameter);  

char * c_left(char *dst,char *src, int n)
{
	char *p = src;
	char *q = dst;
	int len = strlen(src);
	if(n>len) n = len;
	/*p += (len-n);*/   /*���ұߵ�n���ַ���ʼ*/
	while(n--) *(q++) = *(p++);
	*(q++)='\0'; /*�б�Ҫ�𣿺��б�Ҫ*/
	return dst;
}
int KKPlayer::KKProtocolAnalyze(char* StrfileName,KKPluginInfo &KKPl)
{
	char* pos=strstr(StrfileName,":") ;
	int ret=0;
	if(pos!=NULL)
	{ 
		ret =-1;
		int Len=strlen(StrfileName)+1024;
		char *ProName=(char*)::malloc(Len);
		memset(ProName,0,Len);
		int lll=pos-StrfileName;
		c_left(ProName,StrfileName,lll);

		std::list<KKPluginInfo>::iterator It=KKPlayer::KKPluginInfoList.begin();
		for(;It!=KKPluginInfoList.end();++It)
		{
			if(strcmp(ProName,It->ptl)==0)
			{
				int len=strlen(StrfileName)-lll;
				if(len>0)
				{
					strcpy(ProName,pos+1);
					//strcpy(ProName,"");
					strcpy(StrfileName,"");
					strcpy(StrfileName,ProName);
					KKPl=*It;/**/
					ret= 1;
				}
			}
		}
		::free(ProName);
      
		
	}
	return ret;
}
int KKPlayer::GetRealtime()
{

	if(pVideoInfo!=NULL)
		return pVideoInfo->realtime;
	return -1;

}


//���� 1 ��ý��
int is_realtime2(const char *name)
{
	if(   !strcmp(name, "rtp")    || 
		!strcmp(name, "rtsp")   || 
		!strcmp(name, "sdp")
		)
		return 1;


	if(strncmp(name, "rtmp:",5)==0){
		return 1;
	}else if(!strncmp(name, "rtp:", 4)|| !strncmp(name, "udp:", 4)){
		  return 1;
	}else if(strncmp(name, "rtsp:",5)==0){
		return 1;
	}
	return 0;
}

void KKPlayer::ForceAbort()
{
    if(pVideoInfo!=NULL)
	{
	    pVideoInfo->abort_request=1;
	}
}
void KKPlayer::SetLastOpenAudio(bool bLastOpenAudio)
{
  m_bLastOpenAudio=bLastOpenAudio;
}
///�����Ƿ����
void KKPlayer::SetRender(bool bRender)
{
    m_bRender=bRender;
}
int KKPlayer::OpenMedia(const char* URL,const char* Other)
{
	if(strlen(URL)>2047)
	{
	    return -2;
	}
	m_AVInfoLock.Lock();
	if(pVideoInfo!=NULL){
	    pVideoInfo->abort_request=1;
	}
	m_AVInfoLock.Unlock();
	m_PlayerLock.Lock();
	if(m_nPreFile!=0|| m_bOpen)
	{
		m_PlayerLock.Unlock();
        return -1;
	}

	m_bOpen=true;	
	m_nPreFile=1;
	m_AvIsSeg=0;
	m_CacheAvCounter=0;
	
	pVideoInfo = (SKK_VideoState*)KK_Malloc_(sizeof(SKK_VideoState));
	m_strcmd=Other;

	pVideoInfo->nMaxRealtimeDelay=32000;//��λs
	pVideoInfo->pKKPluginInfo=(KKPluginInfo *)KK_Malloc_(sizeof(KKPluginInfo));
	pVideoInfo->pflush_pkt =(AVPacket*)KK_Malloc_(sizeof(AVPacket));
    pVideoInfo->DstAVff=m_DstAVff;
	pVideoInfo->IRender=m_pPlayUI->GetRender();

	#ifndef _WINDOWS
			pVideoInfo->ViewSurface=m_pViewSurface;
			LOGE_KK("pVideoInfo->ViewSurface= %d \n",(int)m_pViewSurface); 
    #endif
	
    m_PktSerial=0;
	m_nSeekTime=0;
	m_nSeekSegId=-1;

	LOGE_KK("Movie Path��\n");
	LOGE_KK("%s",URL);
	LOGE_KK("\n");
	pVideoInfo->viddec.decoder_tid.ThOver=true;
	pVideoInfo->auddec.decoder_tid.ThOver=true;
	pVideoInfo->subdec.decoder_tid.ThOver=true;
	
	m_ReadThreadInfo.ThOver=true;
	m_ReadThreadInfo.Addr=0;
	

	m_VideoRefreshthreadInfo.ThOver=true;
	m_VideoRefreshthreadInfo.Addr=0;

#ifdef WIN32_KK
    m_ReadThreadInfo.ThreadHandel=0;
    m_VideoRefreshthreadInfo.ThreadHandel=0;
#endif

	RECT rt;
	::GetClientRect(m_hwnd,&rt);
	
	
	av_init_packet(pVideoInfo->pflush_pkt);
	flush_pkt.data = (uint8_t *)pVideoInfo->pflush_pkt;

	memcpy(pVideoInfo->filename, URL, strlen(URL));

	pVideoInfo->realtime = is_realtime2(URL);
    if(pVideoInfo->realtime)
	{
	   pVideoInfo->NeedWait=true;
	}
	memset(&m_AVPlayInfo,0,sizeof(MEDIA_INFO));
	//��ʼ������
	packet_queue_init(&pVideoInfo->videoq);
	pVideoInfo->videoq.m_pWaitCond=new CKKCond_t();
	//pVideoInfo->videoq.m_pWaitCond->SetCond();
    //��Ƶ��
	packet_queue_init(&pVideoInfo->audioq);
	pVideoInfo->audioq.m_pWaitCond=new CKKCond_t();
	//pVideoInfo->audioq.m_pWaitCond->SetCond();
	//��Ļ��
	packet_queue_init(&pVideoInfo->subtitleq);
	pVideoInfo->subtitleq.m_pWaitCond=new CKKCond_t();
	//pVideoInfo->subtitleq.m_pWaitCond->SetCond();


	init_clock(&pVideoInfo->vidclk, &pVideoInfo->videoq.serial);
	init_clock(&pVideoInfo->audclk, &pVideoInfo->audioq.serial);
	init_clock(&pVideoInfo->extclk, &pVideoInfo->extclk.serial);

	pVideoInfo->video_stream=-1;
	pVideoInfo->audio_stream=-1;
	pVideoInfo->subtitle_stream=-1;

	pVideoInfo->audio_clock_serial = -1;
	pVideoInfo->av_sync_type = av_sync_type;

	/* start video display */
	if (frame_queue_init(&pVideoInfo->pictq, &pVideoInfo->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
	{

	}
	pVideoInfo->pictq.m_pWaitCond=new CKKCond_t();
    pVideoInfo->pictq.m_pWaitCond->SetCond();


	if (frame_queue_init(&pVideoInfo->subpq, &pVideoInfo->subtitleq, SUBPICTURE_QUEUE_SIZE, 1) < 0)
	{

	}
	pVideoInfo->subpq.m_pWaitCond=new CKKCond_t();
	pVideoInfo->subpq.m_pWaitCond->SetCond();


	if (frame_queue_init(&pVideoInfo->sampq, &pVideoInfo->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
	{

	}
	pVideoInfo->sampq.m_pWaitCond=new CKKCond_t();
    pVideoInfo->sampq.m_pWaitCond->SetCond();

	
	
    pVideoInfo->AVRate=100;
	m_ReadThreadInfo.ThOver=true;
	m_VideoRefreshthreadInfo.ThOver=true;
	m_AudioCallthreadInfo.ThOver=true;

	float aa=(float)pVideoInfo->AVRate/100;
	snprintf(pVideoInfo->Atempo,sizeof(pVideoInfo->Atempo),"atempo=%f",aa);

	pVideoInfo->InAudioSrc=NULL;
	pVideoInfo->OutAudioSink=NULL;
	pVideoInfo->AudioGraph=NULL;
	m_nhasVideoAudio=0;
	LOGE_KK("�����߳�\n");
#ifdef WIN32_KK
	m_ReadThreadInfo.ThreadHandel=(HANDLE)_beginthreadex(NULL, NULL, ReadAV_thread, (LPVOID)this, 0,&m_ReadThreadInfo.Addr);

	if(m_ReadThreadInfo.ThreadHandel==0)
		assert(0);
	m_VideoRefreshthreadInfo.ThreadHandel=(HANDLE)_beginthreadex(NULL, NULL, VideoRefreshthread, (LPVOID)this, 0,&m_VideoRefreshthreadInfo.Addr);
    m_AudioCallthreadInfo.ThreadHandel=(HANDLE)_beginthreadex(NULL, NULL, Audio_Thread, (LPVOID)this, 0,&m_AudioCallthreadInfo.Addr);
#else
	m_ReadThreadInfo.Addr = pthread_create(&m_ReadThreadInfo.Tid_task, NULL, (void* (*)(void*))ReadAV_thread, (LPVOID)this);
	LOGE_KK("m_ReadThreadInfo.Addr =%d\n",m_ReadThreadInfo.Addr);
	LOGE_KK("VideoRefreshthread XX");
	m_VideoRefreshthreadInfo.Addr = pthread_create(&m_VideoRefreshthreadInfo.Tid_task, NULL, (void* (*)(void*))VideoRefreshthread, (LPVOID)this);
	m_AudioCallthreadInfo.Addr =pthread_create(&m_AudioCallthreadInfo.Tid_task, NULL, (void* (*)(void*))Audio_Thread, (LPVOID)this);
#endif
	m_lstPts=-1;

	m_AVCacheInfo.AudioSize=0;
	m_AVCacheInfo.MaxTime=0;
	m_AVCacheInfo.VideoSize=0;


	/*if(strcmp(Other,"-pause")==0)
		pVideoInfo->paused=1;*/

	m_PlayerLock.Unlock();
	LOGE_KK("�����߳̽���\n");
	return 0;
}


void KKPlayer::OnDecelerate()
{
	int64_t seek_target =0;
	bool okk=false;
   // m_PlayerLock.Lock();
	if(pVideoInfo!=NULL&&pVideoInfo->AVRate>50)
	{
		pVideoInfo->AVRate-=10;
		float aa=(float)pVideoInfo->AVRate/100;
		snprintf(pVideoInfo->Atempo,sizeof(pVideoInfo->Atempo),"atempo=%f",aa); 
		seek_target= m_CurTime;/**/
		//packet_queue_put(&pVideoInfo->audioq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt);
		//packet_queue_put(&pVideoInfo->videoq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt);
        okk=true;
	}
	//m_PlayerLock.Unlock();
	if(okk)
	{
		KKSeek(Right,1);
		pVideoInfo->seek_req=2;
	}
}
void KKPlayer::OnAccelerate()
{
	int64_t seek_target=0;
	bool okk=false;
	//m_PlayerLock.Lock();
	if(pVideoInfo!=NULL&&pVideoInfo->AVRate<200)
	{
		pVideoInfo->AVRate+=10;
		float aa=(float)pVideoInfo->AVRate/100;
		  snprintf(pVideoInfo->Atempo,sizeof(pVideoInfo->Atempo),"atempo=%.2f",aa);
        //packet_queue_put(&pVideoInfo->audioq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt);
		//packet_queue_put(&pVideoInfo->videoq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt);
		seek_target = m_CurTime;
		okk=true;
	}
	//m_PlayerLock.Unlock();

	if(okk)
	{
		KKSeek(Right,1);
		pVideoInfo->seek_req=2;
	}
}
int KKPlayer::GetAVRate()
{

	int Rate=100;
	//m_PlayerLock.Lock();
	if(pVideoInfo!=NULL)
	{
		Rate=pVideoInfo->AVRate;
	}
	//m_PlayerLock.Unlock();
	return Rate;
}
/*********��Ƶˢ���߳�********/
 unsigned __stdcall KKPlayer::VideoRefreshthread(LPVOID lpParameter)
 {
	 LOGE_KK("VideoRefreshthread strat \n");
     KKPlayer* pPlayer=(KKPlayer*)lpParameter;
	 pPlayer->m_VideoRefreshthreadInfo.ThOver=false;
	 int llxx=0;
#ifdef WIN32
	 ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif 
#ifdef Android_Plat
	//pPlayer->m_pVideoRefreshJNIEnv=kk_jni_attach_env();
#endif
	 pPlayer->pVideoInfo->remaining_time = 0.01;
	
	 while(pPlayer->m_bOpen)
	 {
		
		if(pPlayer->pVideoInfo!=NULL)
		{
			if(pPlayer->pVideoInfo->abort_request==1)
				break;

			if (pPlayer->pVideoInfo->remaining_time > 0.0)
			{
				int64_t ll=(int64_t)(pPlayer->pVideoInfo->remaining_time* 1000000.0);
				av_usleep(ll);
			}
			 pPlayer->pVideoInfo->remaining_time = 0.01;
			
             pPlayer->VideoRefresh();
			 pPlayer->AvDelayParser();
				
		}else{
			av_usleep(5000);
		}
		
	 }
#ifdef Android_Plat
	//kk_jni_detach_env();
	pPlayer->m_pVideoRefreshJNIEnv=NULL;
#endif

	 pPlayer->m_VideoRefreshthreadInfo.Addr=0;
	 pPlayer->m_VideoRefreshthreadInfo.ThOver=true;
	 LOGE_KK("VideoRefreshthread over \n");
	 return 1;
 }

 //��Ƶ���ݻص��߳�
 unsigned __stdcall  KKPlayer::Audio_Thread(LPVOID lpParameter)
 {
	#ifdef WIN32
		 ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	#endif
	 KKPlayer* pPlayer=(KKPlayer*)lpParameter;
	 pPlayer->ReadAudioCall();
	 return 1;
 }
 void KKPlayer::ReadAudioCall()
 {
	 m_AudioCallthreadInfo.ThOver=false;
	 while(pVideoInfo!=NULL&&!pVideoInfo->abort_request)
	 {
		 if(pVideoInfo->pKKAudio!=NULL&&pVideoInfo->IsReady&&pVideoInfo->audio_st!=NULL)
		    pVideoInfo->pKKAudio->ReadAudio();
		 else
			 av_usleep(20000);
	 }
	 LOGE_KK("KKPlayer Audio_Thread over \n");
	 m_AudioCallthreadInfo.ThOver=true;
 }


 static int decode_interrupt_cb(void *ctx)
{
	 SKK_VideoState *is =(SKK_VideoState *) ctx;
	 return is->abort_request;
}
int  KKPlayer::GetRealtimeDelay()
{
	if(pVideoInfo!=NULL)
          return pVideoInfo->nRealtimeDelay;
	return 0;
}
//ǿ��ˢ��
void KKPlayer::ForceFlushQue()
{
	AvflushRealTime(1);  
	AvflushRealTime(2);  
	AvflushRealTime(3);  
}
 AVIOContext * CreateKKIo(SKK_VideoState *);
 int KKPlayer::ShowTraceAV(bool Show)
 {
		if(pVideoInfo!=NULL){
			pVideoInfo->bTraceAV=Show;
			return 1;
		}
		return 0;
 }

int  KKPlayer::GetPlayTime()
{
    return m_CurTime;
}
int  KKPlayer::GetTotalTime()
{
    return m_TotalTime;
}

 void  KKPlayer::OpenAudioDev()
 {
    ///������������Ƶ�ص�
	m_pSound->SetAudioCallBack(audio_callback);
	m_pSound->SetUserData(pVideoInfo);
	pVideoInfo->pKKAudio=m_pSound;


	if(m_bLastOpenAudio){
	int sample_rate    = pVideoInfo->auddec.avctx->sample_rate;
	int						nb_channels    = pVideoInfo->auddec.avctx->channels;
	int 						channel_layout = pVideoInfo->auddec.avctx->channel_layout;

	m_pSound->OpenAudio(channel_layout,nb_channels,sample_rate);
	}

 }


//����Ƶ�ļ���Ƭ
bool  KKPlayer::OpenInputSegAV(const char *url,short segid,bool flush)
{

	char filename[2048]="";
	strcpy(filename,url);
	if(!strncmp(filename, "rtmp:",5)){
       
	}else if(!strncmp(filename, "rtsp:",5)){
      
	}

	AVFormatContext *pFormatCtx= avformat_alloc_context();
	AVDictionary *format_opts=NULL;
	int err=-1;
	int scan_all_pmts_set = 0;

	pFormatCtx->interrupt_callback.callback = decode_interrupt_cb;
	pFormatCtx->interrupt_callback.opaque = pVideoInfo;
	if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
		av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
		scan_all_pmts_set = 1;
	}
	
	int Segioflags=0;
	if(KKProtocolAnalyze(filename,*pVideoInfo->pKKPluginInfo)==1)
	{	
	    pFormatCtx->pb=CreateKKIo(pVideoInfo);
        pFormatCtx->flags = AVFMT_FLAG_CUSTOM_IO;
		Segioflags= AVFMT_FLAG_CUSTOM_IO;
    }


	if(!strncmp(filename, "rtmp:",5)){
		av_dict_set(&format_opts, "rw_timeout", MaxTimeOutStr, AV_DICT_MATCH_CASE);
	}else if(!strncmp(filename, "rtsp:",5)){
		av_dict_set(&format_opts, "rtsp_transport", "tcp", AV_DICT_MATCH_CASE);
        av_dict_set(&format_opts, "stimeout", MaxTimeOutStr, AV_DICT_MATCH_CASE);
	}

	//�˺�����������
	err =avformat_open_input(
		&pFormatCtx,                   filename,
		pVideoInfo->iformat,    &format_opts);
 

	if(pFormatCtx!=0&&(strncmp(filename, "rtmp:",5)==0||strncmp(filename, "rtsp:",5)==0)){
		pFormatCtx->probesize = 100 *1024;
		pFormatCtx->max_analyze_duration=5 * AV_TIME_BASE;
		double  dx2=av_gettime ()/1000/1000-pVideoInfo->OpenTime;
        if(dx2>MaxTimeOut)
		{
		    err=-1;
		}
	}

    //�ļ���ʧ��
	if(err<0){
		avformat_close_input(&pFormatCtx);
		return false;
	}
    
	if (scan_all_pmts_set)
		av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	av_format_inject_global_side_data(pFormatCtx);
	AVDictionary **opts;
	AVDictionary *codec_opts=NULL;
	opts=setup_find_stream_info_opts(pFormatCtx, codec_opts);


	err=avformat_find_stream_info(pFormatCtx, opts);
	if(opts!=NULL){
        for (int i = 0; i < pFormatCtx->nb_streams; i++)
        av_dict_free(&opts[i]);
        av_freep(&opts);
	}
	if(err<0){
		if(pFormatCtx!=NULL)
		avformat_close_input(&pFormatCtx);
		return false;
    }

	int  i, ret=-1;
	int st_index[AVMEDIA_TYPE_NB]={-1,-1,-1,-1,-1};
	AVPacket pkt1, *pkt = &pkt1;
	int64_t stream_start_time;
	int pkt_in_play_range = 0;
	AVDictionaryEntry *t;

	int orig_nb_streams;
	int64_t pkt_ts;
    int64_t duration= AV_NOPTS_VALUE;
	
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		AVStream *st = pFormatCtx->streams[i];
		enum AVMediaType type = st->codec->codec_type;
		st_index[type] = i;
	}
	


	if (pVideoInfo->video_stream >= 0) {
		packet_queue_put(&pVideoInfo->videoq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,segid,flush);
	}

	if (pVideoInfo->audio_stream >= 0) {
		packet_queue_put(&pVideoInfo->audioq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt,segid,flush);
	}

	if (pVideoInfo->subtitle_stream >= 0) {
		packet_queue_put(&pVideoInfo->subtitleq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,segid,flush);
	}
    pVideoInfo->eof=0;
	pVideoInfo->pSegFormatCtx=pFormatCtx;
	pVideoInfo->Segioflags=Segioflags;
	return true;
}

bool  KKPlayer::loadSeg(AVFormatContext**  pAVForCtx,int AVQueSize,short segid,bool flush)
{
	AVFormatContext*  pFormatCtx=*pAVForCtx;
  
	m_AVNextInfo.SegId=segid;
	m_AVNextInfo.lstOpSt=true;
	m_AVNextInfo.NeedRead=0;
	strcpy(m_AVNextInfo.url,"");
ReOpenAV:
if(m_pPlayUI!=NULL)
{
	m_pPlayUI->GetNextAVSeg(this,AVERROR_EOF,AVQueSize,m_AVNextInfo);
	if(m_AVNextInfo.NeedRead&&strlen(m_AVNextInfo.url))
	{
		 if(OpenInputSegAV(m_AVNextInfo.url,m_AVNextInfo.SegId,flush))
		 {
			  if(pVideoInfo->pSegFormatCtx!=NULL)
			  {
				   pVideoInfo->segid=m_AVNextInfo.SegId;
				   *pAVForCtx=pVideoInfo->pSegFormatCtx;
			  }
		 }else{
			  m_AVNextInfo.lstOpSt=false;
		      goto ReOpenAV;
		 }
	}else{
           if(AVQueSize==-1)
		    m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,KKAVOver);
    }
}
	return m_AVNextInfo.NeedRead;
}
void  KKPlayer::InterSeek(AVFormatContext*  pFormatCtx)
{
	if(m_nSeekTime>0&&pVideoInfo!=NULL&&pVideoInfo->IsReady){
		double incr, pos, frac;
		incr=m_nSeekTime;
		pos = get_master_clock(pVideoInfo);
		if (isNAN(pos))
			pos = (double)pVideoInfo->seek_pos / AV_TIME_BASE;
		incr=m_nSeekTime-pos;
		pos += incr;
		incr=60;
		if (pFormatCtx->start_time != AV_NOPTS_VALUE && pos < pFormatCtx->start_time / (double)AV_TIME_BASE)
			pos = pFormatCtx->start_time / (double)AV_TIME_BASE;
		stream_seek(pVideoInfo, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
		m_nSeekTime=0;
	}
}

void KKPlayer::AvDelayParser()
{

	int pkgsize=pVideoInfo->audioq.size+pVideoInfo->videoq.size;
	///����Ƶ����Ƶ
	if(pVideoInfo->NeedWait&&pVideoInfo->audio_st!=NULL){
			if(pVideoInfo->audioq.size==0||pVideoInfo->videoq.size==0){
				  m_CacheAvCounter++; 
			}
			
			if(pVideoInfo->bTraceAV)
			   LOGE_KK("de %.3fs,que audioq:%d,videoq:%d,subtitleq:%d \n",pVideoInfo->nRealtimeDelay,pVideoInfo->audioq.size,pVideoInfo->videoq.size,pVideoInfo->subtitleq.size);
			
			bool Ok=false;
			//��ʼ����&&avsize>100
			if(pVideoInfo->nRealtimeDelay<=2){
				
				
				///����Ƶ������Ƶ
				if((m_nhasVideoAudio^0x0001)==0x0001&&(m_nhasVideoAudio^0x0010)==0x0010){
				    if(pVideoInfo->audioq.size<=1000&&pVideoInfo->videoq.size<=1000)
						 Ok= true;
				}else {
					   if(m_nhasVideoAudio^0x0010==0x0010)///��Ƶ
                            Ok=  pkgsize<=500 ? true:false;
					   else
					        Ok=  pkgsize<=1000 ? true:false;
				}
				if(Ok&&pVideoInfo->IsReady&&!pVideoInfo->eof){
					pVideoInfo->IsReady=0;
					pVideoInfo->paused|=0x010;
					m_CacheAvCounter=0;
					if(m_pPlayUI!=NULL){
						m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,KKAVWait);
					}
				}
				
			}

            if((m_nhasVideoAudio^0x0010)==0x0010&&(m_nhasVideoAudio^0x0001)==0x0001){
                    Ok=pVideoInfo->audioq.size>2000&&pVideoInfo->videoq.size>2000;
			}else{
				    Ok=pkgsize>4000? true:false;
			}
			/********ˢ�º󻺴�һ���,����������********/
			if((Ok|| pVideoInfo->eof)&&!pVideoInfo->IsReady){
				pVideoInfo->IsReady=1;
				pVideoInfo->paused^=0x010;

				if(m_pPlayUI!=NULL){
					if(pVideoInfo->eof==0)
					   m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,KKAVReady);
				}
			}
		}else if(pVideoInfo->NeedWait&&pVideoInfo->audio_st==NULL&&pVideoInfo->video_st!=NULL){
				if(pVideoInfo->videoq.size==0){
					m_CacheAvCounter++; 
				}   

				if(pVideoInfo->videoq.size<=1000&&pVideoInfo->IsReady&&m_CacheAvCounter>100&&!pVideoInfo->eof){
					pVideoInfo->IsReady=0;
					pVideoInfo->paused|=0x010;
					m_CacheAvCounter=0;
				}
				
				if(pVideoInfo->bTraceAV)
					LOGE_KK("de %.3fs,que audioq:%d,videoq:%d,subtitleq:%d \n",pVideoInfo->nRealtimeDelay,pVideoInfo->audioq.size,pVideoInfo->videoq.size,pVideoInfo->subtitleq.size);
				if((pVideoInfo->videoq.size>2000|| pVideoInfo->eof)&& pVideoInfo->IsReady==0){
					   pVideoInfo->IsReady=1;
					   pVideoInfo->paused^=0x010;
					   if(m_pPlayUI!=NULL){
						   if(pVideoInfo->eof==0)
						    m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename, KKAVReady);
					  }
				}
		}
}
/*****��ȡ��Ƶ��Ϣ******/
void KKPlayer::ReadAV()
{
	
	//rtmp://117.135.131.98/771/003 live=1
	m_PlayerLock.Lock();
	m_ReadThreadInfo.ThOver=false;
	LOGE_KK("ReadAV thread start \n");
	
	AVFormatContext* pFormatCtx= avformat_alloc_context();
	AVDictionary*    format_opts=NULL;
	int err=-1;
	int scan_all_pmts_set = 0;
	pVideoInfo->iformat=NULL;
	
	if(m_pKKPlayerGetUrl){
		char* outstr=0;
	    err=m_pKKPlayerGetUrl(pVideoInfo->filename,&outstr);
		if(err==0&&outstr!=0){
		     strcpy(pVideoInfo->filename,outstr);
		}
	}

	pFormatCtx->interrupt_callback.callback =   decode_interrupt_cb;
	pFormatCtx->interrupt_callback.opaque   =   pVideoInfo;

	if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
		av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
		
		scan_all_pmts_set = 1;
	}
	if(!strncmp(pVideoInfo->filename, "http:",4)){
		av_dict_set(&format_opts,"user-agent", "Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-CN; rv:1.9.2.15) Gecko/20110303 Firefox/3.6.15", AV_DICT_DONT_OVERWRITE);
	}
	pVideoInfo->pFormatCtx = pFormatCtx;
	
	if(m_pPlayUI->PreOpenUrlCallForSeg(pVideoInfo->filename,&m_AvIsSeg,(int*)&pVideoInfo->abort_request)){
	  
	   pVideoInfo->NeedWait=true;
	}

	AVIOContext *customio=0;
	if(KKProtocolAnalyze(pVideoInfo->filename,*pVideoInfo->pKKPluginInfo)==1){	
	    customio=pFormatCtx->pb=CreateKKIo(pVideoInfo);
        pFormatCtx->flags = AVFMT_FLAG_CUSTOM_IO;
		pVideoInfo->ioflags= AVFMT_FLAG_CUSTOM_IO;
    }

	int lxx=128&~128;
	m_AVInfoLock.Lock();
	m_nPreFile=2;
	m_AVInfoLock.Unlock();
	
	if(!strncmp(pVideoInfo->filename, "rtmp:",5)){
        //rtmp ��֧�� timeout
		av_dict_set(&format_opts, "rw_timeout", MaxTimeOutStr, AV_DICT_MATCH_CASE);
		av_dict_set(&format_opts, "fflags", "nobuffer ", 0);
		av_dict_set(&format_opts, "analyzeduration","1000000",0);
	}else if(!strncmp(pVideoInfo->filename, "rtsp:",5)){
		 av_dict_set(&format_opts, "rtsp_transport", "tcp", AV_DICT_MATCH_CASE);
		 av_dict_set(&format_opts, "fflags","nobuffer", 0);
		 av_dict_set(&format_opts, "max_delay","50",0);
		 av_dict_set(&format_opts, "analyzeduration","1000000",0);
	}else if(!strncmp(pVideoInfo->filename, "live:",5)){
		 av_dict_set(&format_opts, "fflags","nobuffer", 0);
		 av_dict_set(&format_opts, "max_delay","50",0);
		 av_dict_set(&format_opts, "analyzeduration","1000000",0);
	}
	///������ѡ��
	if(m_strcmd.length()>1)
	{
	     char   **argv = NULL;
	     int argc=0;
		 argv =KKCommandLineToArgv(m_strcmd.c_str(), &argc);
		 for (int i = 0; i < argc; i++)
		 {
			    char* cmd=argv[i];
				if(0==strcmp(cmd,"-fflags"))
				{
					if (++i < argc){
					   cmd=argv[i];
					   av_dict_set(&format_opts, "fflags", cmd, 0);
					}
				}
		 }
		 free(argv);
	}

	//av_dict_set(&format_opts, "fflags", "-nobuffer", 0);
	pVideoInfo->OpenTime= av_gettime ()/1000/1000;
	double Opex=0;

	LOGE_KK("call avformat_open_input \n");
	//�˺�����������
	err =avformat_open_input(&pFormatCtx,pVideoInfo->filename,pVideoInfo->iformat,   &format_opts);
    
	if(pFormatCtx!=0&&(strncmp(pVideoInfo->filename, "rtmp:",5)==0||strncmp(pVideoInfo->filename, "rtsp:",5)==0)){
		pFormatCtx->probesize = 100 *1024;
		pFormatCtx->max_analyze_duration=5 * AV_TIME_BASE;
		double  dx2=av_gettime ()/1000/1000-pVideoInfo->OpenTime;
        if(dx2>MaxTimeOut){
		    err=-1;
		}
	}
	
	LOGE_KK("avformat_open_input=%d,%s \n",err,pVideoInfo->filename);
	m_AVInfoLock.Lock();
	m_nPreFile=3;
	m_AVInfoLock.Unlock();

	if(pVideoInfo->abort_request==1){

		av_dict_free(&format_opts);
		if(pFormatCtx!=NULL)
		   avformat_free_context(pFormatCtx);
		pVideoInfo->pFormatCtx = NULL;
		 m_PlayerLock.Unlock();
		 return;
	 }

	 
    //�ļ���ʧ��
	if(err<0){
		
		
		if(customio!=0)
		{
				WillCloseKKIo(customio);
		}
	
		av_dict_free(&format_opts);
		if(pFormatCtx!=NULL)
		   avformat_free_context(pFormatCtx);
		pVideoInfo->pFormatCtx = NULL;
		char urlx[2048]="";
		strcpy(urlx,pVideoInfo->filename);
		pVideoInfo->abort_request=1;
		m_PlayerLock.Unlock();


		
        if(m_pPlayUI!=NULL){
			m_pPlayUI->OpenMediaStateNotify(urlx,KKOpenUrlOkFailure);
		}

		LOGE_KK("avformat_open_input <0 \n");
		return;
	}

    m_PlayerLock.Unlock();

	if (scan_all_pmts_set)
		av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	av_format_inject_global_side_data(pFormatCtx);
	
	AVDictionary **opts;
	AVDictionary *codec_opts=NULL;
	opts=setup_find_stream_info_opts(pFormatCtx, codec_opts);
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, opts)<0){
		av_dict_free(&format_opts);
		char urlx[256]="";
		strcpy(urlx,pVideoInfo->filename);
		pVideoInfo->abort_request=1;
		//m_PlayerLock.Unlock();
		if(m_pPlayUI!=NULL){
			m_pPlayUI->OpenMediaStateNotify(urlx,KKAVNotStream);
		}
		LOGE_KK("avformat_find_stream_info<0 \n");
		return;
    }
	
	if(opts!=NULL){
        for (int i = 0; i < pFormatCtx->nb_streams; i++)
            av_dict_free(&opts[i]);
        av_freep(&opts);
	}

	//if(m_bTrace)
	LOGE_KK("avformat_find_stream_info Ok \n");
	
	int  i, ret=-1;
	int st_index[AVMEDIA_TYPE_NB]={-1,-1,-1,-1,-1};
	AVPacket pkt1, *pkt = &pkt1;
	int64_t stream_start_time;
	int pkt_in_play_range = 0;

	int64_t pkt_ts;
    int64_t duration= AV_NOPTS_VALUE;
	if (start_time != AV_NOPTS_VALUE) {
		int64_t timestamp;
		timestamp = start_time;
		
		/* add the stream start time */
		if (pFormatCtx->start_time != AV_NOPTS_VALUE)
			timestamp += pFormatCtx->start_time;
		ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN, timestamp, INT64_MAX, 0);
		if (ret < 0) {
			
		}
	}


    char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
	wanted_stream_spec[AVMEDIA_TYPE_VIDEO]="vst";
	wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE]="sst";
	wanted_stream_spec[AVMEDIA_TYPE_AUDIO]="ast";
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		AVStream *st = pFormatCtx->streams[i];
		enum AVMediaType type = st->codec->codec_type;

        st->discard = AVDISCARD_ALL;
        if (type >= 0 &&  st_index[type] == -1)
            if (avformat_match_stream_specifier(pFormatCtx, st,wanted_stream_spec[type] ) > 0)
                st_index[type] = i;
	}
	/* open the streams */
	if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) 
	{
		if(!m_bLastOpenAudio){
		   OpenAudioDev();
	    }
	   
		LOGE_KK("AVMEDIA_TYPE_AUDIO \n");
		stream_component_open(pVideoInfo, st_index[AVMEDIA_TYPE_AUDIO]);
	}

	if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
		LOGE_KK("AVMEDIA_TYPE_VIDEO \n");
		ret = stream_component_open(pVideoInfo, st_index[AVMEDIA_TYPE_VIDEO]);
	}

	if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
		LOGE_KK("AVMEDIA_TYPE_SUBTITLE \n");
		stream_component_open(pVideoInfo, st_index[AVMEDIA_TYPE_SUBTITLE]);
	}
#ifdef Android_Plat
	if(pVideoInfo&&pVideoInfo->Hard_Code==SKK_VideoState::HARD_CODE_MEDIACODEC&&m_pViewSurface)
	{
	    m_bSurfaceDisplay=true;
	}
#endif

	if(pVideoInfo->iformat==NULL)
	{
		pVideoInfo->iformat=pVideoInfo->pFormatCtx->iformat;
	}
	pVideoInfo->max_frame_duration = (pVideoInfo->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	pVideoInfo->IsReady=1;	

	m_TotalTime=pVideoInfo->pFormatCtx->duration/1000/1000;
	int AVReadyOk=1;

	if(pVideoInfo->realtime&&pVideoInfo->audio_st==NULL)
	{
         pVideoInfo->max_frame_duration = 2.0;
	}
	
	
	int avsize=0;
	

	if(pVideoInfo->video_st==NULL&&pVideoInfo->audio_st!=NULL){
		pVideoInfo->show_mode=  SKK_VideoState::SHOW_MODE_WAVES;
	}
	

	while(m_bOpen) 
	{
		if(pVideoInfo->abort_request)
		{
			break;
		}
		

		int AVQueSize=0;

		/********************ʵʱ��ý�岻֧����ͣ******************************/
		if (pVideoInfo->paused != pVideoInfo->last_paused&&!pVideoInfo->realtime) 
		{
			pVideoInfo->last_paused = pVideoInfo->paused;//
			if (pVideoInfo->paused && AVQueSize>10000)
			{
				pVideoInfo->read_pause_return=1;
				av_read_pause(pFormatCtx);
				av_usleep(1000);
			}
			else{
				if(pVideoInfo->read_pause_return){
				   av_read_play(pFormatCtx);
				   pVideoInfo->read_pause_return=0;
				}
			}
		}

	
        /******���*******/
		if (pVideoInfo->seek_req&&!pVideoInfo->realtime)
		{
			int64_t seek_target = pVideoInfo->seek_pos;
			int64_t seek_min    =pVideoInfo->seek_rel > 0 ? seek_target - pVideoInfo->seek_rel + 2: INT64_MIN;//pVideoInfo->seek_pos-10 * AV_TIME_BASE; //
			int64_t seek_max    =pVideoInfo->seek_rel < 0 ? seek_target - pVideoInfo->seek_rel - 2: INT64_MAX;//=pVideoInfo->seek_pos+10 * AV_TIME_BASE; //


			if(pVideoInfo->seek_req==2){
				seek_min=seek_target;
				if(pVideoInfo->audio_st!=0)
                      seek_max =seek_target + 1000000;
			}
			//int64_t seek_min    =pVideoInfo->seek_pos-10 * AV_TIME_BASE; //
			//int64_t seek_max    =pVideoInfo->seek_pos+10 * AV_TIME_BASE; //
			EKKPlayerErr kkerr=KKSeekOk;
			ret = avformat_seek_file(pFormatCtx, -1, seek_min, seek_target, seek_max, pVideoInfo->seek_flags);
			if (ret < 0) {
                 kkerr=KKSeekErr;
			     assert(0);
				//ʧ��
			}else{
				Avflush(seek_target);
			}
			 if(m_pPlayUI!=NULL){
		       m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,kkerr);
		     }

			pVideoInfo->seek_req=0;
		}

		m_PktSerial=pVideoInfo->viddec.pkt_serial;

		

		/******��������*******/
		int countxx=0;
		
		///�������ӳٴ���Ƶ�豸
		if( pVideoInfo->paused==0&&m_bLastOpenAudio&&pVideoInfo->pKKAudio==NULL)
		{
			OpenAudioDev();
			m_bLastOpenAudio=false;
		}
		///seek ��ն���
		if(m_nSeekSegId>-1)
		{
			this->ForceFlushQue();
		}
		while(1)
		{
			AVQueSize=pVideoInfo->audioq.size + pVideoInfo->videoq.size + pVideoInfo->subtitleq.size;
			if(pVideoInfo->abort_request)
			{
				break;
			}else if( AVQueSize> MAX_QUEUE_SIZE){
				// LOGE_KK("catch full");
				 av_usleep(5000);;//�ȴ�һ��
				 countxx++;
				 if(pVideoInfo->ioflags == AVFMT_FLAG_CUSTOM_IO&&pVideoInfo->pKKPluginInfo!=NULL&&countxx%200==0){
					AVIOContext *KKIO=pFormatCtx->pb;
					if(KKIO!=NULL){
                         KKPlugin* kk= (KKPlugin*)KKIO->opaque;
						 if(kk->GetCacheTime!=NULL){
                                pVideoInfo->nCacheTime=kk->GetCacheTime(kk);
							    if(pVideoInfo->nCacheTime>0&&pVideoInfo->nCacheTime>m_AVCacheInfo.MaxTime)
								      m_AVCacheInfo.MaxTime=pVideoInfo->nCacheTime;
						 }
                    }
					countxx=0;
				 }
				 continue;
			}else{
				break;
			}
		}


        if(pVideoInfo->nCacheTime>0&&pVideoInfo->nCacheTime>m_AVCacheInfo.MaxTime)
			m_AVCacheInfo.MaxTime=pVideoInfo->nCacheTime;

		if(pVideoInfo->realtime&&pVideoInfo->nRealtimeDelayCount>1000){	
			 pVideoInfo->OpenTime+=pVideoInfo->nRealtimeDelay;
			 pVideoInfo->nRealtimeDelayCount=0;
		}

		
		bool forceOver=false;

		  ///seek.
		if(m_nSeekSegId==-1)
		     InterSeek(pFormatCtx);
		else if(m_nSeekSegId>=-1)
		{
		
		}
		ret = av_read_frame(pFormatCtx, pkt);
		
        if(ret>=0&&m_nSeekSegId>-1)
		{
			av_packet_unref(pkt);
			forceOver=true;
			ret=AVERROR_EOF;
			//pVideoInfo->eof=1;
		}
	/*	if(pVideoInfo->pSegFormatCtx!=0&&pVideoInfo->SegStreamState==0)
			{
				avformat_close_input(&pVideoInfo->pSegFormatCtx);
			}*/


		if (ret < 0) {
			 if(pVideoInfo->bTraceAV)
			 LOGE_KK("readAV ret=%d \n",ret);

			 if(pVideoInfo->abort_request)
				 break;
			 if ((ret == AVERROR_EOF || avio_feof(pFormatCtx->pb)) && !pVideoInfo->eof)
			 {
			        if(pVideoInfo->bTraceAV) 
				    LOGE_KK("ret == AVERROR_EOF || avio_feof(pFormatCtx->pb)) && !pVideoInfo->eof \n");
                    pVideoInfo->eof=1;
					EKKPlayerErr err=KKEOF;
					if(pVideoInfo->realtime&&m_pPlayUI!=NULL&&!m_AvIsSeg){ 
						pVideoInfo->nRealtimeDelay=0;
						pVideoInfo->IsReady=0;
						err=KKRealTimeOver;
					}else if(-pFormatCtx->pb->error== ERROR_BROKEN_PIPE){
					    err=KKEOF;
						pVideoInfo->abort_request=1;
					}
					m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,err);

			 }else if (pFormatCtx->pb && pFormatCtx->pb->error&&ret != AVERROR_EOF){
					 pVideoInfo->eof=1;
					 pVideoInfo->abort_request=1;
					 pVideoInfo->nRealtimeDelay=0;
					/* if(pVideoInfo->realtime)
					 {
						 pVideoInfo->IsReady=0;
						 if(m_pPlayUI!=NULL&&!m_AvIsSeg)
						     m_pPlayUI->AutoMediaCose(this,-2,AVQueSize,m_AVNextInfo);
					 }else {
						 if(m_pPlayUI!=NULL&&!m_AvIsSeg)
						     m_pPlayUI->AutoMediaCose(this,pFormatCtx->pb->error,AVQueSize,m_AVNextInfo);
					 }*/
				 if(pVideoInfo->bTraceAV)
					 LOGE_KK("pFormatCtx->pb && pFormatCtx->pb->error \n");
				 break;
				
			 }else if(ret == AVERROR_EOF && pVideoInfo->eof||forceOver)
			 {
				        ///������Ƶ��Ƭ������Ƶ������ͬһ���				      
				       
			            if(m_pPlayUI!=NULL&&pVideoInfo->pSegFormatCtx==NULL)
						{
							if(AVQueSize==0&&(pVideoInfo->pictq.size<=1|| pVideoInfo->sampq.size<=1)){
								AVQueSize=-1;///�ļ��Ѿ���ȡ����
							}
							if(m_nSeekSegId>-1){
							   loadSeg(&pFormatCtx,0,m_nSeekSegId,true);
							   m_nSeekSegId=-1;
							}else{
							   loadSeg(&pFormatCtx,AVQueSize);
							}
							
						}else if(pVideoInfo->SegStreamState!=0&&pVideoInfo->pSegFormatCtx!=NULL){
							    int retxx=pVideoInfo->SegStreamState;
						        for (int i = 0; i <pVideoInfo->pSegFormatCtx->nb_streams; i++) 
								{
									AVStream *st = pVideoInfo->pSegFormatCtx->streams[i];
									enum AVMediaType type = st->codec->codec_type;
									if(AVMEDIA_TYPE_VIDEO==type){
										retxx^= 0x10;
									}else if(AVMEDIA_TYPE_AUDIO==type){
										retxx^= 0x01;
									}else if(AVMEDIA_TYPE_SUBTITLE==type){
										retxx^= 0x100;
									}
								}
								if(retxx==0){
									if(m_PlayerLock.TryLock())
									{
										AVFormatContext *pTmpCtx=pVideoInfo->pFormatCtx;
										pVideoInfo->pFormatCtx=pVideoInfo->pSegFormatCtx;
										avformat_close_input(&pTmpCtx);
										pVideoInfo->SegStreamState=0;
										pVideoInfo->pSegFormatCtx=NULL;
									
										m_PlayerLock.Unlock();
										
										continue;
									}
								}
						}else if(pVideoInfo->pSegFormatCtx!=pFormatCtx&&pVideoInfo->SegStreamState==0)
						{
							int xx=0;
						      if(pVideoInfo->pSegFormatCtx==pFormatCtx)
							   { 
							      xx++;
							   }
							   xx++;
						}
						
			 }
			av_usleep(10000);
			continue;
		} else {
			pVideoInfo->eof = 0;
			if(AVReadyOk==1&&m_pPlayUI!=NULL)
			{
				AVReadyOk=0;
				m_pPlayUI->OpenMediaStateNotify(pVideoInfo->filename,KKAVReady);
			}
		}

       
		
		/* check if packet is in play range specified by user, then queue, otherwise discard */
		stream_start_time = pFormatCtx->streams[pkt->stream_index]->start_time;
		
		INT64 xx=(INT64)pkt->dts ;
		xx=AV_NOPTS_VALUE;
		pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
		
		int64_t  a1=(pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0));
		int64_t a2= av_q2d(pFormatCtx->streams[pkt->stream_index]->time_base);
		pkt_in_play_range = duration == AV_NOPTS_VALUE ||
			a1 * a2-(double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000 <= ((double)duration / 1000000);
		pkt_in_play_range =1;

		

		//��Ƶ
		if (pkt->stream_index == pVideoInfo->audio_stream && pkt_in_play_range&&pkt->data!=NULL) {
			packet_queue_put(&pVideoInfo->audioq, pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
			
			m_AVCacheInfo.AudioSize=pVideoInfo->audioq.PktMemSize;
			if(pVideoInfo->audio_st!=NULL)
			{
				pkt_ts=pkt_ts*av_q2d(pVideoInfo->audio_st->time_base);
			}
			if(m_AVCacheInfo.MaxTime<pkt_ts)
			{
				m_AVCacheInfo.MaxTime=pkt_ts;
			}
			m_nhasVideoAudio|=0x0001;
		} else if (//��Ƶ
			pkt->stream_index == pVideoInfo->video_stream && pkt_in_play_range
			&& !(pVideoInfo->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC&&pkt->data!=NULL)
			) {
			
			packet_queue_put(&pVideoInfo->videoq, pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
			m_AVCacheInfo.VideoSize=pVideoInfo->videoq.PktMemSize;
			if(pVideoInfo->video_st!=NULL){
				pkt_ts=pkt_ts*av_q2d(pVideoInfo->video_st->time_base);
			}
			if(m_AVCacheInfo.MaxTime<pkt_ts)
			{
                 m_AVCacheInfo.MaxTime=pkt_ts;
			}
			m_nhasVideoAudio|=0x0010;
		}//��Ļ
		else if (pkt->stream_index == pVideoInfo->subtitle_stream && pkt_in_play_range&&pkt->data!=NULL) 
		{
			//printf("subtitleq\n");
			packet_queue_put(&pVideoInfo->subtitleq, pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
			m_nhasVideoAudio|=0x0100;
		} else
		{
			av_packet_unref(pkt);
		}
		
	}
	av_dict_free(&format_opts);
	m_ReadThreadInfo.Addr=0;
	LOGE_KK("readAV Over \n");
	
}
//�ͷŶ�������
void KKPlayer::PacketQueuefree()
{
   if(pVideoInfo!=NULL)
   {
	    LOGE_KK("pVideoInfo->videoq \n");
		if(pVideoInfo->videoq.pLock->TryLock())
		{
              pVideoInfo->videoq.pLock->Unlock();
		}else{
               pVideoInfo->videoq.pLock->Unlock();
		}
	    packet_queue_flush(&pVideoInfo->videoq);

		LOGE_KK("pVideoInfo->audioq \n");
		if(pVideoInfo->audioq.pLock->TryLock())
		{
			pVideoInfo->audioq.pLock->Unlock();
		}else{
			pVideoInfo->audioq.pLock->Unlock();
		}
		packet_queue_flush(&pVideoInfo->audioq);

		LOGE_KK("pVideoInfo->subtitleq \n");
		if(pVideoInfo->subtitleq.pLock->TryLock())
		{
			pVideoInfo->subtitleq.pLock->Unlock();
		}else{
			pVideoInfo->subtitleq.pLock->Unlock();
		}
		packet_queue_flush(&pVideoInfo->subtitleq);


		LOGE_KK("pVideoInfo->pictq \n");
        frame_queue_destory(&pVideoInfo->pictq);

		LOGE_KK("pVideoInfo->subpq \n");
		frame_queue_destory(&pVideoInfo->subpq);

		LOGE_KK("pVideoInfo->sampq \n");
		frame_queue_destory(&pVideoInfo->sampq);
   }
}

void KKPlayer::SetVolume(long value)
{ 
	if(m_pSound!=NULL)
	{
	   m_pSound->SetVolume(value);
	}
}
long KKPlayer::GetVolume()
{
	if(m_pSound!=NULL)
	     return m_pSound->GetVolume();
	return 0;
}
void KKPlayer::Pause()
{
	this->m_PlayerLock.Lock();
		if(pVideoInfo!=NULL)
         {
			if(pVideoInfo->paused & 0x01){
				int lx=pVideoInfo->paused & 0x0;
				pVideoInfo->paused ^=0x01;
			}else{
				pVideoInfo->paused|=0x01;
			}
		}
	this->m_PlayerLock.Unlock();
}
//���
int KKPlayer::KKSeek( SeekEnum en,int value)
{
	if(pVideoInfo!=NULL&&m_nPreFile==3&&!pVideoInfo->realtime){
	   double incr, pos;
	   incr=value;
	   pos = get_master_clock(pVideoInfo);
	   if (isNAN(pos))
		   pos = (double)pVideoInfo->seek_pos / AV_TIME_BASE;
       pos+=incr;
	   if (pVideoInfo->pFormatCtx->start_time != AV_NOPTS_VALUE && pos < pVideoInfo->pFormatCtx->start_time / (double)AV_TIME_BASE)
		   pos = pVideoInfo->pFormatCtx->start_time / (double)AV_TIME_BASE;
	   stream_seek(pVideoInfo, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
       return 0;
  }
  return -1; 
}

int KKPlayer::AVSeek(int value,short segid)
{
	m_PktSerial=-1;
	if(pVideoInfo!=NULL&&m_nPreFile==3&&!pVideoInfo->realtime)
    {
	   m_nSeekTime=value;
	   m_nSeekSegId=segid;
	   return 0;
	}
	return -1; 
}

void KKPlayer::AvflushRealTime(int Avtype)
{
	if(pVideoInfo==NULL)
		return;
	if (pVideoInfo->video_stream >= 0&&Avtype==1) 
	{
		packet_queue_put(&pVideoInfo->videoq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
	}else if(pVideoInfo->audio_st!=NULL&&pVideoInfo->audio_stream >= 0&&Avtype==2) {
		packet_queue_put(&pVideoInfo->audioq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt,pVideoInfo->segid);
	}else if(pVideoInfo->subtitle_stream >= 0&&Avtype==3) {
		packet_queue_put(&pVideoInfo->subtitleq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
	}
}

/******����ʵʱ��ý����С�ӳ�**********/
int KKPlayer::SetMaxRealtimeDelay(double Delay)
{
     if(pVideoInfo!=NULL&&Delay>=1.0)
	 {
		 pVideoInfo->nMaxRealtimeDelay=Delay;
		 return 1;
	 }
	 return 0;
}
void KKPlayer::Avflush(int64_t seek_target)
{
	if (pVideoInfo->video_stream >= 0) 
	{
		packet_queue_put(&pVideoInfo->videoq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
		pVideoInfo->pictq.mutex->Lock();
		pVideoInfo->pictq.size=0;
		pVideoInfo->pictq.rindex=0;
		pVideoInfo->pictq.windex=0;
		pVideoInfo->pictq.rindex_shown=1;

		pVideoInfo->pictq.m_pWaitCond->SetCond();
		pVideoInfo->pictq.mutex->Unlock();
	}

	if (pVideoInfo->audio_st!=NULL&&pVideoInfo->audio_stream >= 0) 
	{
		packet_queue_put(&pVideoInfo->audioq,pVideoInfo->pflush_pkt, pVideoInfo->pflush_pkt,pVideoInfo->segid);
		pVideoInfo->sampq.mutex->Lock();
		pVideoInfo->sampq.size=0;
		pVideoInfo->sampq.rindex=0;
		pVideoInfo->sampq.windex=0;
		pVideoInfo->sampq.rindex_shown=1;
		pVideoInfo->sampq.m_pWaitCond->SetCond();
		pVideoInfo->sampq.mutex->Unlock();/**/
	}
	if (pVideoInfo->subtitle_stream >= 0) 
	{
		packet_queue_put(&pVideoInfo->subtitleq, pVideoInfo->pflush_pkt,pVideoInfo->pflush_pkt,pVideoInfo->segid);
	}
		if (pVideoInfo->seek_flags & AVSEEK_FLAG_BYTE) 
		{
			set_clock(&pVideoInfo->extclk, NAN, 0);
		} else 
		{
			set_clock(&pVideoInfo->extclk, seek_target / (double)AV_TIME_BASE, 0);
		}
}

#ifdef WIN32
#include <emmintrin.h>
#include <smmintrin.h>
void* gpu_memcpy(void* d, const void* s, size_t size)
{
	static const size_t regsInLoop = sizeof(size_t) * 2; // 8 or 16

	if (d == NULL|| s == NULL) 
		return NULL;

	// If memory is not aligned, use memcpy
	bool isAligned = (((size_t)(s) | (size_t)(d)) & 0xF) == 0;
	if (!isAligned)
	{
		return memcpy(d, s, size);
	}

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
#ifdef _M_X64
	__m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
#endif

	size_t reminder = size & (regsInLoop * sizeof(xmm0) - 1); // Copy 128 or 256 bytes every loop
	size_t end = 0;

	__m128i* pTrg = (__m128i*)d;
	__m128i* pTrgEnd = pTrg + ((size - reminder) >> 4);
	__m128i* pSrc = (__m128i*)s;

	// Make sure source is synced - doesn't hurt if not needed.
	_mm_sfence();

	while (pTrg < pTrgEnd)
	{
		// _mm_stream_load_si128 emits the Streaming SIMD Extensions 4 (SSE4.1) instruction MOVNTDQA
		// Fastest method for copying GPU RAM. Available since Penryn (45nm Core 2 Duo/Quad)
		xmm0  = _mm_stream_load_si128(pSrc);
		xmm1  = _mm_stream_load_si128(pSrc + 1);
		xmm2  = _mm_stream_load_si128(pSrc + 2);
		xmm3  = _mm_stream_load_si128(pSrc + 3);
		xmm4  = _mm_stream_load_si128(pSrc + 4);
		xmm5  = _mm_stream_load_si128(pSrc + 5);
		xmm6  = _mm_stream_load_si128(pSrc + 6);
		xmm7  = _mm_stream_load_si128(pSrc + 7);
#ifdef _M_X64 // Use all 16 xmm registers
		xmm8  = _mm_stream_load_si128(pSrc + 8);
		xmm9  = _mm_stream_load_si128(pSrc + 9);
		xmm10 = _mm_stream_load_si128(pSrc + 10);
		xmm11 = _mm_stream_load_si128(pSrc + 11);
		xmm12 = _mm_stream_load_si128(pSrc + 12);
		xmm13 = _mm_stream_load_si128(pSrc + 13);
		xmm14 = _mm_stream_load_si128(pSrc + 14);
		xmm15 = _mm_stream_load_si128(pSrc + 15);
#endif
		pSrc += regsInLoop;
		// _mm_store_si128 emit the SSE2 intruction MOVDQA (aligned store)
		_mm_store_si128(pTrg     , xmm0);
		_mm_store_si128(pTrg +  1, xmm1);
		_mm_store_si128(pTrg +  2, xmm2);
		_mm_store_si128(pTrg +  3, xmm3);
		_mm_store_si128(pTrg +  4, xmm4);
		_mm_store_si128(pTrg +  5, xmm5);
		_mm_store_si128(pTrg +  6, xmm6);
		_mm_store_si128(pTrg +  7, xmm7);
#ifdef _M_X64 // Use all 16 xmm registers
		_mm_store_si128(pTrg +  8, xmm8);
		_mm_store_si128(pTrg +  9, xmm9);
		_mm_store_si128(pTrg + 10, xmm10);
		_mm_store_si128(pTrg + 11, xmm11);
		_mm_store_si128(pTrg + 12, xmm12);
		_mm_store_si128(pTrg + 13, xmm13);
		_mm_store_si128(pTrg + 14, xmm14);
		_mm_store_si128(pTrg + 15, xmm15);
#endif
		pTrg += regsInLoop;
	}

	// Copy in 16 byte steps
	if (reminder >= 16)
	{
		size = reminder;
		reminder = size & 15;
		end = size >> 4;
		for (size_t i = 0; i < end; ++i)
		{
			pTrg[i] = _mm_stream_load_si128(pSrc + i);
		}
	}

	// Copy last bytes - shouldn't happen as strides are modulu 16
	if (reminder)
	{
		__m128i temp = _mm_stream_load_si128(pSrc + end);

		char* ps = (char*)(&temp);
		char* pt = (char*)(pTrg + end);

		for (size_t i = 0; i < reminder; ++i)
		{
			pt[i] = ps[i];
		}
	}
	return d;
}


#endif