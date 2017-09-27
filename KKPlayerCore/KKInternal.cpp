/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
#include "KKInternal.h"
#include <math.h>
#include <assert.h>
#include <time.h>
#include "render/render.h"
#ifndef PRIx64 
#define PRIx64       "I64x"
#endif
int nKKH264Codec=0;
int nKKH265Codec=0;
void SetKKplayerH264HardCodec(int value)
{
nKKH264Codec=value;
}
void SetKKplayerH265HardCodec(int value)
{
nKKH265Codec=value;
}
//#include "rtmp/AV_FLv.h"
void *KK_Malloc_(size_t size)
{
    size=size+128-size%64;
	void *p= av_malloc(size);
	memset(p,0,size);
	return p;
}
void  KK_Free_(void *ptr)
{
	if(ptr!=NULL)
	   av_free(ptr);
}
#define PixelFormat AVPixelFormat
#ifdef WIN32
      //AV_PIX_FMT_BGRA; //AVPixelFormat::AV_PIX_FMT_RGB24;////AV_PIX_FMT_RGBA;//AV_PIX_FMT_RGBA;//AV_PIX_FMT_YUV420P;
      //DXV2
	  int  BindDxva2Module(	AVCodecContext  *pCodecCtx);
      int  GetD3d9RestDevState(void* opaque);
	  void* DxvaSurCopy(void* opaque,void* Data);
	  void Dxva2ResetDevCall(void* UserData,int state);
	  void FroceClose_Kk_Va_Dxva2(void *kk_va);

	  //inter QSV���롣
	  int BindQsvModule(AVCodecContext  *pCodecCtx);
	  void KKFreeQsv(AVCodecContext *avct);
#endif
	  

	 // AVPixelFormat DstAVff=AV_PIX_FMT_BGRA;
 //<tgmath.h> 
/***********KKPlaye �ڲ�ʵ��*************/
static int lowres = 0;
int cxlp=0;
static int64_t sws_flags = SWS_BICUBIC;
unsigned __stdcall  Audio_Thread(LPVOID lpParameter);
unsigned __stdcall  Video_thread(LPVOID lpParameter);
unsigned __stdcall  Subtitle_thread(LPVOID lpParameter);
#ifdef WIN32_KK
inline long rint(double x) 
{ 
	if(x >= 0.)
		return (long)(x + 0.5); 
	else 
		return (long)(x - 0.5); 
}
#endif


int GetW(AVCodecContext  *pCodecCtx);
int GetH(AVCodecContext  *pCodecCtx);
void packet_queue_init(SKK_PacketQueue  *q) 
{
	memset(q, 0, sizeof(SKK_PacketQueue));
	q->pLock = new CKKLock();
	q->abort_request = 1;
}
//�������
int packet_queue_put(SKK_PacketQueue *q, AVPacket *pkt,AVPacket *flush_pkt,short SegId,bool bflush) 
{

	SKK_AVPacketList *pkt1;
	if(av_dup_packet(pkt) < 0) 
	{
		return -1;
	}
	q->PktMemSize+=pkt->size;
	pkt1 = (SKK_AVPacketList *)KK_Malloc_(sizeof(SKK_AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;
	if (pkt == flush_pkt&&bflush)
		q->serial++;

	pkt1->serial = q->serial;
	pkt1->seg=SegId;
	q->pLock->Lock();

	if (!q->last_pkt)
	{
		q->first_pkt = pkt1;//��һ����
	}
	else
		q->last_pkt->next = pkt1;
	
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	
	q->pLock->Unlock();
	return 0;
}

int packet_queue_get(SKK_PacketQueue *q, AVPacket *pkt, int block, int *serial,short *segid)
{
	SKK_AVPacketList *pkt1=NULL;
	int ret=-1;

	q->pLock->Lock();

	for(;;) 
	{
		pkt1 = q->first_pkt;
		if (pkt1) 
		{
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			q->PktMemSize-=pkt->size;
			*segid=pkt1->seg;
			*serial=pkt1->serial;
			
			av_free(pkt1);
			pkt1=0;
			ret = 1;
			break;
		} else if (!block) 
		{
			ret = 0;
			break;
		} else 
		{
			ret = -1;
			break;
		}
	}
	q->pLock->Unlock();
	return ret;
}

void frame_queue_destory(SKK_FrameQueue *f)
{
	for (int i = 0; i < f->max_size; i++) 
	{
		SKK_Frame *vp = &f->queue[i];
		frame_queue_unref_item(vp);
		vp->uploaded=1;
		av_frame_free(&vp->frame);
		if(vp->allocated==1)
		{
		    av_free(vp->buffer);
			vp->buffer=NULL;
		}
	}
}
/**********��ն���***********/
void packet_queue_flush(SKK_PacketQueue *q)
{
	SKK_AVPacketList *pkt, *pkt1;

	q->pLock->Lock();
	for (pkt = q->first_pkt; pkt; pkt = pkt1) 
	{
		pkt1 = pkt->next;
		av_packet_unref(&pkt->pkt);
		av_freep(&pkt);
	}
	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->nb_packets = 0;
	q->size = 0;

	q->pLock->Unlock();
}



void Packet_Queue_All_Flush(SKK_VideoState *pVideoInfo)
{
	if (pVideoInfo->video_stream >= 0) 
	{
		pVideoInfo->videoq.serial++;
	}

	if (pVideoInfo->audio_st!=NULL&&pVideoInfo->audio_stream >= 0) 
	{
		pVideoInfo->videoq.serial++;
	}
	if (pVideoInfo->subtitle_stream >= 0) 
	{
		pVideoInfo->videoq.serial++;
	}
}
//ʱ�Ӳ���
double get_clock(SKK_Clock *c)
{
	/*if (*c->queue_serial <c->serial&&*c->queue_serial>0&& c->serial>0)
		return NAN;*/
	if (*c->queue_serial !=c->serial)
		return NAN;
	if (c->paused) 
	{
		return c->pts;
	} 
	else 
	{
		//��ǰϵͳ����
		double time = av_gettime_relative() / 1000000.0;
		double a=c->pts_drift + time;
		double b=(time - c->last_updated) * (1.0 - c->speed);
		return a-b;
		//return  c->pts_drift + time- (time - c->last_updated) * (1.0 - c->speed);
	}
}

void set_clock_at(SKK_Clock *c, double pts, int serial, double time)
{
	c->pts = pts;
	c->last_updated = time;
	c->pts_drift = c->pts - time;
	c->serial = serial;
}

void set_clock(SKK_Clock *c, double pts, int serial)
{
		double time = av_gettime_relative() / 1000000.0;
		set_clock_at(c, pts, serial, time);
}

void set_clock_speed(SKK_Clock *c, double speed)
{
	set_clock(c, get_clock(c), c->serial);
	c->speed = speed;
}

void init_clock(SKK_Clock *c, int *queue_serial)
{
	c->speed = 1.0;
	c->paused = 0;
	c->queue_serial = queue_serial;
	set_clock(c, NAN, -1);
}

static void sync_clock_to_slave(SKK_Clock *c, SKK_Clock *slave)
{
	double clock = get_clock(c);
	double slave_clock = get_clock(slave);
	if (!isNAN(slave_clock) && (isNAN(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set_clock(c, slave_clock, slave->serial);
}

int get_master_sync_type(SKK_VideoState *is)
{
	if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) 
	{
		if (is->video_st)
			return AV_SYNC_VIDEO_MASTER;
		else
			return AV_SYNC_AUDIO_MASTER;
	} else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) 
	{
		if (is->audio_st)
			return AV_SYNC_AUDIO_MASTER;
		/*else if(is->video_st)
			return AV_SYNC_VIDEO_MASTER;*/
		else
			return AV_SYNC_EXTERNAL_CLOCK;
	} else 
	{
		return AV_SYNC_EXTERNAL_CLOCK;
	}
}

/* get the current master clock value */
double get_master_clock(SKK_VideoState *is)
{
	double val;

	switch (get_master_sync_type(is)) 
	{
		case AV_SYNC_VIDEO_MASTER:
			val = get_clock(&is->vidclk);
			break;
		case AV_SYNC_AUDIO_MASTER:
			val = get_clock(&is->audclk);
			break;
		default:
			val = get_clock(&is->extclk);
			break;
	}
	return val;
}
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001
 void check_external_clock_speed(SKK_VideoState *is) {
   if (is->video_stream >= 0 && is->videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES ||
       is->audio_stream >= 0 && is->audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) {
       set_clock_speed(&is->extclk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN, is->extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
   } else if ((is->video_stream < 0 || is->videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
              (is->audio_stream < 0 || is->audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES)) {
       set_clock_speed(&is->extclk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX, is->extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
   } else {
       double speed = is->extclk.speed;
       if (speed != 1.0)
           set_clock_speed(&is->extclk, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
   }
}
static int synchronize_audio(SKK_VideoState *is, int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER) 
	{
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_clock(&is->audclk) - get_master_clock(is);

        if (!isNAN(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) 
		{
					is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
					if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) 
					{
						/* not enough measures to have a correct estimate */
						is->audio_diff_avg_count++;
					} else 
					{
						/* estimate the A-V difference */
						avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

						if (fabs(avg_diff) >= is->audio_diff_threshold) 
						{
							wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
							min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
							max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
							wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
						}
					}
				} else 
		         {
					/* too big difference : may be initial PTS errors, so
					   reset A-V filter */
					is->audio_diff_avg_count = 0;
					is->audio_diff_cum       = 0;
				}
      }

    return wanted_nb_samples;
}


int frame_queue_init(SKK_FrameQueue *f, SKK_PacketQueue *pktq, int max_size, int keep_last)
{
	int i;
	memset(f, 0, sizeof(SKK_FrameQueue));

	f->mutex = new CKKLock();
	f->pktq = pktq;
	f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
	f->keep_last = keep_last;
	for (i = 0; i < f->max_size; i++){
		if (!(f->queue[i].frame = av_frame_alloc()))
			return AVERROR(ENOMEM);
		f->queue[i].uploaded=1;
	}
	return 0;
}



//��Ƶ���ص�
int audio_fill_frame( SKK_VideoState *pVideoInfo) 
{ 

	bool Afterdelay=false;
DELXXX:
	int n=0;
	AVCodecContext *aCodecCtx=pVideoInfo->auddec.avctx;	
	SKK_VideoState *is=pVideoInfo;
    int audio_pkt_size = 0;
	
    int64_t dec_channel_layout;
	int len1, resampled_data_size, data_size = 0;
    int wanted_nb_samples;

	AVFrame *frame=NULL;
	SKK_Frame *af=NULL;
	

   
   do 
   {
#if defined(_WIN32)
	   while (frame_queue_nb_remaining(&is->sampq) <= 0) {
		   if ((av_gettime_relative() - is->audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
			   return -1;
	   }
#else
	    if(frame_queue_nb_remaining(&is->sampq) <= 0) 
	   {
		   return -1;
	   }/*  */
#endif
	  if( !(af = frame_queue_peek_readable(&is->sampq)))
		  return -1;

	   frame_queue_next(&is->sampq,true);
	   pVideoInfo->audio_clock_serial=af->serial;
	   //pVideoInfo->audio_clock_serial=is->auddec.pkt_serial;
	   pVideoInfo->cursegid=af->segid;
   } while (af->serial!=is->auddec.pkt_serial&&!is->abort_request);
	
	frame=af->frame;

	//��Ƶת��
	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(frame)/*aCodecCtx->channels*/,frame->nb_samples,
		(AVSampleFormat)frame->format,1);
	dec_channel_layout =
		(frame->channel_layout && av_frame_get_channels(frame) == av_get_channel_layout_nb_channels(frame->channel_layout)) ?
		 frame->channel_layout : av_get_default_channel_layout(av_frame_get_channels(frame));
	AVSampleFormat bc=(AVSampleFormat)frame->format;
    wanted_nb_samples = synchronize_audio(pVideoInfo, frame->nb_samples);
	//ʵʱ�Ƚ���
	if (
		 frame->format        != pVideoInfo->audio_src.fmt            ||
		 dec_channel_layout       != pVideoInfo->audio_src.channel_layout ||
		 frame->sample_rate   != pVideoInfo->audio_src.freq           ||
		 (wanted_nb_samples       != frame->nb_samples )
		 ) 
	 {	
			  swr_free(&pVideoInfo->swr_ctx);
			  pVideoInfo->swr_ctx =swr_alloc_set_opts(NULL,
				  pVideoInfo->audio_tgt.channel_layout, pVideoInfo->audio_tgt.fmt,pVideoInfo->audio_tgt.freq,
				  dec_channel_layout,           (AVSampleFormat)frame->format, frame->sample_rate,
				  0, NULL);

			 if (!pVideoInfo->swr_ctx || swr_init(pVideoInfo->swr_ctx) < 0) 
			 {
				 swr_free(&pVideoInfo->swr_ctx);
				 pVideoInfo->swr_ctx=NULL;
			 }
			 pVideoInfo->audio_src.channel_layout = dec_channel_layout;
			 pVideoInfo->audio_src.channels       = av_frame_get_channels(frame);
			 pVideoInfo->audio_src.freq =frame->sample_rate;
			 pVideoInfo->audio_src.fmt = (AVSampleFormat)frame->format;
	 }


	 if (pVideoInfo->swr_ctx) 
	 {
		 //����ָ��  &frame.data[0];//
		 const uint8_t **inextended_data = (const uint8_t **)frame->extended_data;
		 //�����ַ
		 uint8_t **OutData = &pVideoInfo->audio_buf1;

		 int out_count = (int64_t)wanted_nb_samples * pVideoInfo->audio_tgt.freq / frame->sample_rate + 256;
		 //�����С
		 int out_size  = av_samples_get_buffer_size(NULL, pVideoInfo->audio_tgt.channels, out_count, pVideoInfo->audio_tgt.fmt, 0);
		 int len2;
		 if (out_size < 0) 
		 {
			// av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
			 return -1;
		 }
		 if (wanted_nb_samples != frame->nb_samples) 
		 {
			 if (swr_set_compensation(pVideoInfo->swr_ctx, (wanted_nb_samples - frame->nb_samples) * pVideoInfo->audio_tgt.freq / frame->sample_rate,
				 wanted_nb_samples * pVideoInfo->audio_tgt.freq / frame->sample_rate) < 0) 
			 {
					assert(0);
			 }
		 }
		
		static int lxxx=out_size;
		 //�����ڴ�
		 av_fast_malloc(&pVideoInfo->audio_buf1, &pVideoInfo->audio_buf_size, out_size);
		
		 if (!pVideoInfo->audio_buf1)
		 {
			 return AVERROR(ENOMEM);
		 }
		 memset(pVideoInfo->audio_buf1,0,out_size);

		 

		int ll=out_size / pVideoInfo->audio_tgt.channels  / av_get_bytes_per_sample(pVideoInfo->audio_tgt.fmt);
		 
		//��Ƶת��
		 len2 = swr_convert(pVideoInfo->swr_ctx,OutData,out_count, inextended_data, frame->nb_samples);
		 if(len2<0)
		 {
			 return -1;
		 }
		 if (len2 >= out_count) 
		 {
			 //av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
			 if (swr_init(pVideoInfo->swr_ctx) < 0)
				 swr_free(&pVideoInfo->swr_ctx);
		 }

		
		 pVideoInfo->audio_buf =(uint8_t *)pVideoInfo->audio_buf1;
		 resampled_data_size = len2 *pVideoInfo->audio_tgt.channels * av_get_bytes_per_sample(pVideoInfo->audio_tgt.fmt);
		 data_size=resampled_data_size;

	 }else 
	 {
		 is->audio_buf = af->frame->data[0];
		 resampled_data_size = data_size;
		 //�����ڴ�
		// av_fast_malloc(&pVideoInfo->audio_buf1, &pVideoInfo->audio_buf_size, data_size);
		
		 //pVideoInfo->audio_buf = pVideoInfo->audio_buf1;
	 }

	 if (!isNAN(af->pts))
	 {
		// n = 2 * pVideoInfo->auddec.avctx->channels;
		// int llxx=data_size/n;
		// pVideoInfo->audio_clock =af->pts+ (double)data_size/(double)(n * pVideoInfo->auddec.avctx->sample_rate);/**/
		 double ll=(double) af->frame->nb_samples / af->frame->sample_rate;
	     is->audio_clock = af->pts + ll;
		 if(data_size>0&&!is->abort_request&&is->realtime&&!is->abort_request)
		 {
			
			 if(pVideoInfo->nRealtimeDelay>is->nMaxRealtimeDelay||Afterdelay)
			 { 
				 pVideoInfo->nRealtimeDelay-=ll;
				 is->nRealtimeDelayCount++;
				 Afterdelay=true;
				 if(pVideoInfo->nRealtimeDelay<=0.0000001)
				 {
				     pVideoInfo->nRealtimeDelay=0.0000001;
					  Afterdelay=false;
				 }
				 goto DELXXX;
			 }else{
				 is->nRealtimeDelayCount=0;
			 }
		 }
		 
	 }
	 else
		 is->audio_clock = NAN;

	
	return data_size;
}


/* copy samples for viewing in editor window */
static void update_sample_display(SKK_VideoState *is, short *samples, int samples_size)
{
    int size, len;

    size = samples_size / sizeof(short);
    while (size > 0) {
        len = SAMPLE_ARRAY_SIZE - is->sample_array_index;
        if (len > size)
            len = size;
        memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
        samples += len;
        is->sample_array_index += len;
        if (is->sample_array_index >= SAMPLE_ARRAY_SIZE)
            is->sample_array_index = 0;
        size -= len;
    }
}
/* prepare a new audio buffer */
void audio_callback(void *userdata, char *stream, int len)
{
	
	SKK_VideoState *pVideoInfo=(SKK_VideoState *)userdata;
	memset(stream,0,len);
	if (pVideoInfo->paused||pVideoInfo->IsReady!=1||!pVideoInfo->audio_st)
	{
		return;
	}
	int audio_size=0, len1=0;
	int slen=len;
	//��ȡ���ڵ�ʱ��
	pVideoInfo->audio_callback_time = av_gettime_relative();
	int silencelen=0;
	while (len > 0) 
	{
		if (pVideoInfo->audio_buf_index >= pVideoInfo->audio_buf_size) 
		{
            audio_size = audio_fill_frame(pVideoInfo);	
			if (audio_size < 0)
			{
				pVideoInfo->audio_buf = pVideoInfo->silence_buf;
                pVideoInfo->audio_buf_size =512 / pVideoInfo->audio_tgt.frame_size * pVideoInfo->audio_tgt.frame_size;
				silencelen+=512;
			  
			} else 
			{		
				if (pVideoInfo->show_mode != SKK_VideoState::SHOW_MODE_VIDEO)
                   update_sample_display(pVideoInfo, (int16_t *)pVideoInfo->audio_buf, audio_size);
				pVideoInfo->audio_buf_size = audio_size;
			}
			pVideoInfo->audio_buf_index = 0;
		}

		len1 = pVideoInfo->audio_buf_size - pVideoInfo->audio_buf_index;

		if (len1 > len)
			len1 = len;


		memcpy(stream, (uint8_t *)pVideoInfo->audio_buf + pVideoInfo->audio_buf_index, len1);
		len -= len1;
		stream += len1;
		pVideoInfo->audio_buf_index += len1;
	}
	pVideoInfo->audio_write_buf_size = pVideoInfo->audio_buf_size - pVideoInfo->audio_buf_index;
	if (!isNAN(pVideoInfo->audio_clock)) 
	{
		if(silencelen>0){
		   double sle=(double)(2 * silencelen)/ pVideoInfo->audio_tgt.bytes_per_sec;
		pVideoInfo->nRealtimeDelay+=sle;
		}
		set_clock_at(&pVideoInfo->audclk,     
			pVideoInfo->audio_clock - (double)(2 * pVideoInfo->audio_hw_buf_size + pVideoInfo->audio_write_buf_size) / pVideoInfo->audio_tgt.bytes_per_sec, 
			pVideoInfo->audio_clock_serial, 
			pVideoInfo->audio_callback_time / 1000000.0);
		sync_clock_to_slave(&pVideoInfo->extclk, &pVideoInfo->audclk);
	}
}


//����Ƶ
static int audio_open2( void *opaque,                               int wanted_channel_layout, 
					    int wanted_nb_channels,                     int wanted_sample_rate, 
                        struct SKK_AudioParams *audio_hw_params)
{
	
	
	SKK_VideoState *is=(SKK_VideoState *)opaque;
	if(is->pKKAudio!=NULL)
	{
		is->pKKAudio->OpenAudio( wanted_channel_layout, wanted_nb_channels,wanted_sample_rate);
		//is->pKKAudio->Start();
	}
	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
	audio_hw_params->freq = wanted_sample_rate;
	audio_hw_params->channel_layout = wanted_channel_layout;
	audio_hw_params->channels = 2;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0)
	{
		//av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
		return -1;
	}
	return 1024;
}


//��ʼ��������
static void decoder_init(SKK_Decoder *d, AVCodecContext *avctx, SKK_PacketQueue *queue) 
{
	memset(d, 0, sizeof(SKK_Decoder));
	d->avctx = avctx;
	d->pQueue = queue;
	d->start_pts = AV_NOPTS_VALUE;
}

//��ʼ����
static void decoder_start(SKK_Decoder *d,unsigned (__stdcall* _StartAddress) (void *),SKK_VideoState* is)
{
	d->pQueue->abort_request = 0;
	packet_queue_put(d->pQueue, is->pflush_pkt,is->pflush_pkt,is->segid);
#ifdef WIN32_KK
	d->decoder_tid.ThreadHandel=(HANDLE)_beginthreadex(NULL, NULL, _StartAddress, (LPVOID)is, 0,&d->decoder_tid.Addr);
#else
	d->decoder_tid.Addr = pthread_create(&d->decoder_tid.Tid_task, NULL, (void* (*)(void*))_StartAddress, (LPVOID)is);
#endif
}



static inline int64_t get_valid_channel_layout(int64_t channel_layout, int channels);
//{
//	if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
//		return channel_layout;
//	else
//		return 0;
//}




int seg_stream_component_open(SKK_VideoState *is, int stream_index)
{
	AVFormatContext *ic = is->pSegFormatCtx;
    AVCodecContext *avctx=NULL;
	AVCodecContext  *aCodecCtx = NULL;
    AVCodec *codec=NULL;
    const char *forced_codec_name = NULL;
    AVDictionary *opts=NULL;
    AVDictionaryEntry *t = NULL;
    int sample_rate, nb_channels;
    int channel_layout;
	int stream_lowres = lowres;
    int ret = 0;
   

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
   

	avctx = avcodec_alloc_context3(NULL);
	if (!avctx)
		return AVERROR(ENOMEM);

	ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
	if (ret < 0)
		goto fail;

	
    codec = avcodec_find_decoder(avctx->codec_id);


    switch(avctx->codec_type)
	{
        case AVMEDIA_TYPE_AUDIO   : 
									 {
										is->audio_filter_src.freq           = avctx->sample_rate;
										is->audio_filter_src.channels       = avctx->channels;
										is->audio_filter_src.channel_layout = get_valid_channel_layout(avctx->channel_layout, avctx->channels);
										is->audio_filter_src.fmt            = avctx->sample_fmt;
									 }
			                         is->last_audio_stream= stream_index;
									 break;
        case AVMEDIA_TYPE_SUBTITLE: 
			                         //LOGE("Code:AVMEDIA_TYPE_SUBTITLE");              
					                 is->last_subtitle_stream = stream_index;
									 break;
        case AVMEDIA_TYPE_VIDEO   : 
			                         //LOGE("Code:AVMEDIA_TYPE_VIDEO");  
			                         is->last_video_stream= stream_index;
									 break;
    }
    
    if (!codec) 
	{
			LOGE_KK("Code:-1");  
			return -1;
    }

    avctx->codec_id = codec->id;
   
	if(stream_lowres > av_codec_get_max_lowres(codec))
	{
	
		av_codec_get_max_lowres(codec);
		stream_lowres = av_codec_get_max_lowres(codec);
	}
	av_codec_set_lowres(avctx, stream_lowres);

	if(stream_lowres) 
		avctx->flags |= CODEC_FLAG_EMU_EDGE;
	
	if(codec->capabilities & CODEC_CAP_DR1)
		avctx->flags |= CODEC_FLAG_EMU_EDGE;

	if(avctx->codec_type==AVMEDIA_TYPE_VIDEO)
	{
       #ifdef WIN32
		       avctx->codec_id=codec->id;
             
			   if(is->Hard_Code==is->HARDCODE::HARD_CODE_DXVA){
				   if(BindDxva2Module(avctx)<0){
					   is->Hard_Code=is->HARDCODE::HARD_CODE_NONE;
				   }else{
				      
				   }
			   }else if(is->Hard_Code==is->HARDCODE::HARD_CODE_QSV){	   
			       if(BindQsvModule(avctx)>-1){
					   codec = avcodec_find_decoder_by_name("kk_h264_qsv"); 
				   }else{
				       is->Hard_Code=is->HARDCODE::HARD_CODE_NONE;
				   }		
			   }
       #endif
	}

	
	//�򿪽�����
	if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) 
	{
		LOGE_KK("avcodec_open2 %d",avctx->codec_type);
	}
  
    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
	{
			case AVMEDIA_TYPE_AUDIO:
									sample_rate    = avctx->sample_rate;
									nb_channels    = avctx->channels;
									channel_layout = avctx->channel_layout;
									/* prepare audio output */
									if ((ret = audio_open2(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
									{
										assert(0);
										goto fail;
									}
									is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;
									is->audio_stream = stream_index;
									is->audio_st = ic->streams[stream_index];
									is->auddec.avctx=avctx;
									break;
			case AVMEDIA_TYPE_VIDEO:
									is->video_stream = stream_index;
									is->video_st = ic->streams[stream_index];
									is->viddec.avctx = avctx;
									break;
			case AVMEDIA_TYPE_SUBTITLE:
									is->subtitle_stream = stream_index;
									is->subtitle_st = ic->streams[stream_index];
                                    is->subdec.avctx = avctx;
									break;
			default:
				                    break;
    }

fail:
    av_dict_free(&opts);

    return ret;
}
//#ifdef Android_Plat
static enum AVPixelFormat mediacodec_GetFormat( AVCodecContext *p_context,const enum AVPixelFormat *pi_fmt )
{

	for( int i = 0; pi_fmt[i] != AV_PIX_FMT_NONE; i++ )
	{
		if( pi_fmt[i] == AV_PIX_FMT_MEDIACODEC )
			return pi_fmt[i];
	}
	return avcodec_default_get_format( p_context, pi_fmt );
}
//#endif
//����
int stream_component_open(SKK_VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->pFormatCtx;
    AVCodecContext *avctx=NULL;
	AVCodecContext  *aCodecCtx = NULL;
    AVCodec *codec=NULL;
    const char *forced_codec_name = NULL;
    AVDictionary *opts=NULL;
    AVDictionaryEntry *t = NULL;
    int sample_rate, nb_channels;
    int channel_layout;
	int stream_lowres = lowres;
    int ret = 0;
   

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
   

	avctx = avcodec_alloc_context3(NULL);
	if (!avctx)
		return AVERROR(ENOMEM);

	ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
	if (ret < 0)
		goto fail;

	
    codec = avcodec_find_decoder(avctx->codec_id);


    switch(avctx->codec_type)
	{
        case AVMEDIA_TYPE_AUDIO   : 
									{
										is->audio_filter_src.freq           = avctx->sample_rate;
										is->audio_filter_src.channels       = avctx->channels;
										is->audio_filter_src.channel_layout = get_valid_channel_layout(avctx->channel_layout, avctx->channels);
										is->audio_filter_src.fmt            = avctx->sample_fmt;
										
									}
			                         is->last_audio_stream= stream_index;
									 break;
        case AVMEDIA_TYPE_SUBTITLE: 
			                         //LOGE("Code:AVMEDIA_TYPE_SUBTITLE");              
					                 is->last_subtitle_stream = stream_index;
									 break;
        case AVMEDIA_TYPE_VIDEO   : 
			                         //LOGE("Code:AVMEDIA_TYPE_VIDEO");  
			                         is->last_video_stream= stream_index;
									 break;
    }
    
    if (!codec) 
	{
									//LOGE("Code:-1");  
									return -1;
    }

    avctx->codec_id = codec->id;
   
	if(stream_lowres > av_codec_get_max_lowres(codec))
	{
	
		av_codec_get_max_lowres(codec);
		stream_lowres = av_codec_get_max_lowres(codec);
	}
	av_codec_set_lowres(avctx, stream_lowres);

	if(stream_lowres) 
		avctx->flags |= CODEC_FLAG_EMU_EDGE;
	//if (fast)   
		//avctx->flags2 |= CODEC_FLAG2_FAST;
	if(codec->capabilities & CODEC_CAP_DR1)
		avctx->flags |= CODEC_FLAG_EMU_EDGE;

	
	if(avctx->codec_type==AVMEDIA_TYPE_VIDEO)
	{ 
	   avctx->codec_id=codec->id;
	   if(avctx->codec_id==AV_CODEC_ID_H264)
	   {
	      is->Hard_Code=(SKK_VideoState::HARDCODE)nKKH264Codec;
	   }else if (avctx->codec_id==AV_CODEC_ID_HEVC)
	   {
	      is->Hard_Code=(SKK_VideoState::HARDCODE)nKKH265Codec;
	   }
       #ifdef WIN32
	           //is->Hard_Code=is->HARDCODE::HARD_CODE_QSV;
	        //   is->Hard_Code=is->HARDCODE::HARD_CODE_DXVA;
			   is->IRender->renderLock();
			   is->IRender->SetResetHardInfoCall(NULL,NULL);
			   if(is->Hard_Code==SKK_VideoState::HARD_CODE_DXVA)
			   {
		           
				   if(BindDxva2Module(avctx)<0){
					   is->Hard_Code=SKK_VideoState::HARD_CODE_NONE; 
					   is->IRender->SetResetHardInfoCall(NULL,NULL);
				   }else
				   {
				        is->IRender->SetResetHardInfoCall(Dxva2ResetDevCall,avctx);
				   }
			   }else if(is->Hard_Code==SKK_VideoState::HARD_CODE_QSV){	   
			       if(BindQsvModule(avctx)>-1){
					   if(avctx->codec_id==AV_CODEC_ID_H264)
					       codec = avcodec_find_decoder_by_name("kk_h264_qsv"); 
					   if(avctx->codec_id==AV_CODEC_ID_HEVC)
					       codec = avcodec_find_decoder_by_name("kk_hevc_qsv"); 
				   }else{
				       is->Hard_Code=SKK_VideoState::HARD_CODE_NONE;
				   }
			   }
			   is->IRender->renderUnLock();
			   
			   
      #else 
	           LOGE_KK("kkmediacodec \n"); 
			   if(is->Hard_Code==SKK_VideoState::HARD_CODE_MEDIACODEC){
			       if(avctx->codec_id==AV_CODEC_ID_H264){
			            codec = avcodec_find_decoder_by_name("h264_mediacodec"); 
				   }else if(avctx->codec_id==AV_CODEC_ID_VP8){
					   codec = avcodec_find_decoder_by_name("vp8_mediacodec");
				   }else if(avctx->codec_id==AV_CODEC_ID_VP9){
					   codec = avcodec_find_decoder_by_name("vp9_mediacodec");
				   }else if(avctx->codec_id==AV_CODEC_ID_MPEG4){
					   codec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
				   }else  if(avctx->codec_id==AV_CODEC_ID_HEVC){ 
					   codec = avcodec_find_decoder_by_name("hevc_mediacodec");
				   }
				   
				   if(codec!=NULL){
					    avctx->get_format=mediacodec_GetFormat;
				        AVMediaCodecContext *mc = av_mediacodec_alloc_context();

					     mc->surface =is->ViewSurface;
                    /* avctx->hwaccel_context = mc;*/
                         avctx->hwaccel_context =mc;
       //                int retx=av_mediacodec_default_init(avctx, mc,is->ViewSurface);
					  //if(retx==AVERROR_EXTERNAL)
					  // LOGE_KK("mediacodec ViewSurface err %d \n",(int)is->ViewSurface); 

					  // LOGE_KK("mediacodec ViewSurface %d \n",(int)is->ViewSurface); 
					  // LOGE_KK("mc ViewSurface %d \n",(int)mc->surface); 

					   ///pix_fmt = ff_get_format(avctx, pix_fmts); ��֪��Ϊɶû�з��� AV_PIX_FMT_MEDIACODEC�����޸�һ��ffmpeg��Դ�뿴�����в��С�
					   char abcd[64]="";
						snprintf(abcd,64,"%d",AV_PIX_FMT_MEDIACODEC);
						av_dict_set(&opts, "pixel_format",abcd,0);
                      
				   }else{
				       LOGE_KK("mediacodec no find\n");  
				   }
			   }else{
			      LOGE_KK("mediacodec no\n"); 
			   }
	      
       #endif
			   //av_get_format
	   if(codec==NULL){
		   
	        codec = avcodec_find_decoder(avctx->codec_id);
			is->Hard_Code=SKK_VideoState::HARD_CODE_NONE;
	   }
	}


	if (avctx->codec_type == AVMEDIA_TYPE_AUDIO)
            av_dict_set(&opts, "refcounted_frames", "1", 0);
	else if(is->Hard_Code==SKK_VideoState::HARD_CODE_DXVA||(is->Hard_Code==SKK_VideoState::HARD_CODE_MEDIACODEC&&is->ViewSurface!=0)){
	       LOGE_KK("refcounted_frames  no %d \n",is->Hard_Code ); 
	}else if (avctx->codec_type == AVMEDIA_TYPE_VIDEO){
			av_dict_set(&opts, "refcounted_frames", "1", 0);
			
	}

	//�򿪽�����
	if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) 
	{
	
		//if(is->bTraceAV)
		LOGE_KK("avcodec_open2 %d",avctx->codec_type);  
		//ʧ��
		assert(0);
	}else{
#ifndef WIN32
		if(avctx->codec_type == AVMEDIA_TYPE_VIDEO){
	       AVMediaCodecContext *mc=(AVMediaCodecContext *)avctx->hwaccel_context;
           LOGE_KK("mc ViewSurface %d \n",(int)mc->surface); 
		}
#endif
	}
	
 // 
	if(is->Hard_Code==SKK_VideoState::HARD_CODE_MEDIACODEC &&avctx->codec_type == AVMEDIA_TYPE_VIDEO){
	                   enum AVPixelFormat pix_fmt;
                       const enum AVPixelFormat pix_fmts[2] = {AV_PIX_FMT_MEDIACODEC,AV_PIX_FMT_NONE};
                       pix_fmt =(AVPixelFormat) av_get_format(avctx, pix_fmts);
					   LOGE_KK("mediacodec pix_fmt avctx->codec->pix_fmts:%d  avctx->pix_fmt:%d  avctx->sw_pix_fmt:%d pix_fmt:%d AV_PIX_FMT_MEDIACODEC:%d\n",
						   pix_fmt,avctx->codec->pix_fmts, avctx->pix_fmt,avctx->sw_pix_fmt,pix_fmt,AV_PIX_FMT_MEDIACODEC);  
					   LOGE_KK("codec %s  %d  \n",codec->name);  
					  // avctx->sw_pix_fmt=AV_PIX_FMT_MEDIACODEC;
					  //  avctx->pix_fmt
	}

    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
	{
        case AVMEDIA_TYPE_AUDIO:
							sample_rate    = avctx->sample_rate;
							nb_channels    = avctx->channels;
							channel_layout = avctx->channel_layout;
							/* prepare audio output */
							if ((ret = audio_open2(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
							{
								assert(0);
								goto fail;
							}
							is->audio_hw_buf_size = ret;
							is->audio_src = is->audio_tgt;
							is->audio_buf_size  = 0;
							is->audio_buf_index = 0;

							/* init averaging filter */
							is->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
							is->audio_diff_avg_count = 0;
							/* since we do not have a precise anough audio fifo fullness,
							   we correct audio sync only if larger than this threshold */
							is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;

							is->audio_stream = stream_index;
							is->audio_st = ic->streams[stream_index];

							decoder_init(&is->auddec, avctx, &is->audioq);
							if ((ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && 
								!ic->iformat->read_seek) 
							{
								is->auddec.start_pts = is->audio_st->start_time;
								is->auddec.start_pts_tb = is->audio_st->time_base;
							}
							is->auddec.decoder_tid.ThOver=false;
							decoder_start(&is->auddec,&Audio_Thread,is);
                            break;
    case AVMEDIA_TYPE_VIDEO:
							is->video_stream = stream_index;
							is->video_st = ic->streams[stream_index];

							
							decoder_init(&is->viddec, avctx, &is->videoq);
							//is->need_width; //FFALIGN(pFrame->width, 64);
				            //is->need_width;//FFALIGN(pFrame->height, 2);
							is->queue_attachments_req = 1;
							is->viddec.decoder_tid.ThOver=false;
							decoder_start(&is->viddec,&Video_thread,is);
                            break;
    case AVMEDIA_TYPE_SUBTITLE:
							is->subtitle_stream = stream_index;
							is->subtitle_st = ic->streams[stream_index];

							decoder_init(&is->subdec, avctx, &is->subtitleq);
							is->subdec.decoder_tid.ThOver=false;
							decoder_start(&is->subdec,&Subtitle_thread,is);
							break;
    default:
        break;
    }

fail:
    av_dict_free(&opts);

    return ret;
}







//avcodec_decode_subtitle2



/*******ͬ����Ƶ********/
double synchronize_video(SKK_VideoState *is, AVFrame *src_frame, double pts)  
{  
	double frame_delay;  

	if(pts != 0)  
	{  
		/* if we have pts, set video clock to it */  
		is->audclk.pts = pts;  
	}  
	else  
	{  
		/* if we aren't given a pts, set it to the clock */  
		pts = is->audclk.pts;
	}  
	/* update the video clock */  
	frame_delay = av_q2d(is->video_st->codec->time_base);  
	/* if we are repeating a frame, adjust clock accordingly */  
	frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);  
	//is->video_clock += frame_delay;  
	return pts;  
} 



//�����Ƿ���д
static SKK_Frame *frame_queue_peek_writable(SKK_FrameQueue *f)
{
	f->mutex->Lock();
	//bool mm=true;
	if(f->size >= f->max_size &&
		!f->pktq->abort_request) 
	{
		/*****���ź�******/
		//f->m_pWaitCond->ResetCond();
	//	mm=false;
		//f->mutex->Unlock();
		f->m_pWaitCond->WaitCond(1,f->mutex);
	}
//	if(mm)
	f->mutex->Unlock();

	if (f->pktq->abort_request)
		return NULL;
	//if(f->windex==f->rindex)
	//	assert(0);
	return &f->queue[f->windex];
}


SKK_Frame *frame_queue_peek_readable(SKK_FrameQueue *f)
{

	f->mutex->Lock();
	//bool mm=true;
	if(f->size - f->rindex_shown <= 0 &&
		!f->pktq->abort_request) 
	{
		//f->m_pWaitCond->ResetCond();
		//f->mutex->Unlock();
		//mm=false;
		f->m_pWaitCond->WaitCond(1,f->mutex);
	}
	//if(mm)
		f->mutex->Unlock();

	if (f->pktq->abort_request)
		return NULL;

	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/********��д���λ��*******/
SKK_Frame *frame_queue_peek(SKK_FrameQueue *f)
{
	int ll=(f->rindex + f->rindex_shown) % f->max_size;
	return &f->queue[ll];
}
/* return the number of undisplayed frames in the queue */
/********δ��ʾ��ͼƬ��**********/
int frame_queue_nb_remaining(SKK_FrameQueue *f)
{
	return f->size - f->rindex_shown;
}
/**********���һ�ζ���λ��*************/
SKK_Frame *frame_queue_peek_last(SKK_FrameQueue *f)
{
	return &f->queue[f->rindex];
}
/* Unreference all the buffers referenced by frame and reset the frame fields.*/
void frame_queue_unref_item(SKK_Frame *vp)
{
	av_frame_unref(vp->frame);
    if(!vp->uploaded)
	   vp->uploaded=1;
	/*vp->Bmp.data[0]=0;
	vp->Bmp.data[1]=0;
	vp->Bmp.data[2]=0;
	vp->Bmp.linesize[0]=0;
	vp->Bmp.linesize[1]=1;
	vp->Bmp.linesize[0]=2;*/
	avsubtitle_free(&vp->sub);
}
/****ˢ�¶���,���¶��еĴ�С****/
void frame_queue_push(SKK_FrameQueue *f)
{
	f->mutex->Lock();
	if (++f->windex >= f->max_size)
		f->windex = 0;
	
	f->size++;
	f->m_pWaitCond->CondSignal();
	f->mutex->Unlock();
}
void frame_queue_next(SKK_FrameQueue *f,bool NeedLock)
{
	/*****�Ƿ񱣴���һ�ε�ֵ******/
	if (f->keep_last && !f->rindex_shown)
	{
		f->rindex_shown = 1;
		return;
	}
	
	frame_queue_unref_item(&f->queue[f->rindex]);
	/******һ�������Ѿ���ȡ��****/
	if(NeedLock)
		f->mutex->Lock();
	
	if (++f->rindex == f->max_size)
	{
		f->rindex = 0;
	}
	/*if(f->rindex==f->windex&&f->windex!=0)
	{
		f->rindex++;
	}*/
	f->size--;
	if(f->size<0)
		f->size=0;/**/

	//if(f->size<f->max_size)
	//{
	//	//���¼���Ч
	//	f->m_pWaitCond->SetCond();
	//}
	f->m_pWaitCond->CondSignal();
	if(NeedLock)
	    f->mutex->Unlock();
}
SKK_Frame *frame_queue_peek_next(SKK_FrameQueue *f)
{
	return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}



double vp_duration(SKK_VideoState *is, SKK_Frame *vp, SKK_Frame *nextvp) 
{
	if (vp->serial == nextvp->serial) 
	{
		double duration = nextvp->pts - vp->pts;
		if (isNAN(duration) || duration <= 0 || duration > is->max_frame_duration)
			return vp->duration;
		else
			return duration;
	} else 
	{
		return 0.0;
	}
}

//������ʾʱ��
void update_video_pts(SKK_VideoState *is, double pts, int64_t pos, int serial) 
{
	/* update current video pts */
	set_clock(&is->vidclk, pts, serial);
	sync_clock_to_slave(&is->extclk, &is->vidclk);
}
/*****�����ӳ�ʱ��******/
double compute_target_delay(double delay, SKK_VideoState *is)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) 
	{
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
		double a=get_clock(&is->vidclk);
		double b=get_master_clock(is);
        diff =  a- b;
        if(diff<0)
		{
			int i=0;
			i++;
			//return diff;/**/
		}
        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isNAN(diff) && fabs(diff) < is->max_frame_duration) 
		{
            if (diff <= -sync_threshold)
                delay = FFMAX(0, delay + diff);
            else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }


    return delay;
}


struct SwsContext *BMPimg_convert_ctx=NULL;

int DxPictureCopy(struct AVCodecContext *avctx,AVFrame *src,AVFrame **Out);

int QsvNv12toFrameI420(AVFrame *frame1,AVFrame *frame);


//ͼƬ���� ͼƬ,�����кܴ���Ż��ռ�
int queue_picture(SKK_VideoState *is, AVFrame *pFrame, double pts,double duration, int64_t pos, int serial)
{  

	SKK_FrameQueue *pPictq=&is->pictq;
	///***�ҵ�һ�����õ�SKK_Frame***/
	SKK_Frame *vp =NULL;
		
	
	vp=frame_queue_peek_writable(pPictq);
    if(vp==NULL)
	   return -1;

	int copydata=0;
#ifdef Android_Plat
	if(is->Hard_Code!=SKK_VideoState::HARD_CODE_MEDIACODEC)
	{
		if(pFrame->linesize[0]!=pFrame->width)
			 copydata=1;
	}
#endif
	//if()
	pPictq->mutex->Lock();
	vp->frame->sample_aspect_ratio = pFrame->sample_aspect_ratio;
	vp->frame->pts=pFrame->pts;
	vp->frame->format=pFrame->format;
	vp->frame->width=pFrame->width;
	vp->frame->height=pFrame->height;

    is->video_pitch    =pFrame->width;
	is->viddec_height  =pFrame->height;
	is->viddec_width   =pFrame->width;

    vp->duration=duration;
	vp->pos=pos;
	vp->serial=serial;
	vp->pts=pts;
	vp->PktNumber=is->PktNumber++;
   
   // is->viddec_width=pFrame->width;
    //is->viddec_height=pFrame->height;
	

	AVFrame *pOutAV=NULL;
//AV_PIX_FMT_NV12
#ifdef WIN32
//	return 0;
	/* if(is->Hard_Code==is->HARDCODE::HARD_CODE_DXVA)
	 {
	       DWORD t_start = GetTickCount();
	       DxPictureCopy(is->viddec.avctx,pFrame,&pOutAV);
	       DWORD t_end = GetTickCount();
	       int ll=t_end-t_start;
         
	 }else{*/
           pOutAV=pFrame;
	// }
#else
       pOutAV=pFrame;
#endif

	   if(!vp->uploaded){
	       int ii=0;
		   ii++;
	   }
	   if(is->last_height!=pOutAV->height ||is->last_width!=pOutAV->width){

		   if( is->img_convert_ctx!=NULL){
	           sws_freeContext(is->img_convert_ctx);
		       is->img_convert_ctx=NULL;
		   }
	   }

	   PixelFormat format=(PixelFormat)pOutAV->format;
	   if( vp->buffer==NULL || vp->height!=pOutAV->height ||vp->width!=pOutAV->width){
			
			 is->last_width=pFrame->width;
			 is->last_height=pFrame->height;
			 vp->width =   is->last_width; 
			 vp->height=   is->last_height;//FFALIGN(pFrame->height, 2);
			 vp->pitch =   is->last_width;
			
			 if(is->DstAVff!=format||copydata)
			 {
				 vp->allocated = 1;
				 int numBytes=avpicture_get_size(is->DstAVff, vp->width,vp->height); //pFrame->width,pFrame->height
				 numBytes=numBytes*sizeof(uint8_t)+100;
				 if(vp->buflen<numBytes)
				 {
					 vp->buflen=numBytes;
						if(vp->buffer)
					 av_free(vp->buffer);
					 vp->buffer=(uint8_t *)KK_Malloc_(vp->buflen); 
					 avpicture_fill((AVPicture *)&vp->Bmp, vp->buffer,is->DstAVff, vp->width,vp->height);
				 }
				
			 }else{
			        if(vp->frame==NULL)
					      vp->frame= av_frame_alloc();
			 }
		}
		
		pPictq->mutex->Unlock();
	
#ifdef _WINDOWS
		if(pOutAV->format== (int)AV_PIX_FMT_DXVA2_VLD){

			vp->picformat=format;

			//DxvaSurCopy(is->viddec.avctx->opaque,pOutAV->data[3]);
			is->IRender->renderLock();
			int resetdev=GetD3d9RestDevState( is->viddec.avctx->opaque);
			 if(resetdev==0){
				 kkAVPicInfo picinfo;
				 memcpy(picinfo.data,pOutAV->data,sizeof(picinfo.data));
				 memcpy(picinfo.linesize,pOutAV->linesize,sizeof(picinfo.linesize));
				 picinfo.width=vp->width;
				 picinfo.height=vp->height;
				 picinfo.picformat=AV_PIX_FMT_DXVA2_VLD;
				 is->IRender->renderUnLock();
				 is->IRender->render(&picinfo,false);
			 }else{
			    is->IRender->renderUnLock();
			 }
		}else
#endif/**/
		if(pOutAV->format==AV_PIX_FMT_MEDIACODEC){///AndroidӲ�����ʽ
		      av_frame_move_ref(vp->frame,pOutAV);
			  vp->picformat=AV_PIX_FMT_MEDIACODEC;
			  LOGE_KK("dex AV_PIX_FMT_MEDIACODE\n");

		}else if(is->DstAVff!=format&&is->Hard_Code!=SKK_VideoState::HARD_CODE_QSV&&format!=AV_PIX_FMT_NV12||copydata)
		{
			
			is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
			 pOutAV->width,       pOutAV->height ,              format ,
			 pOutAV->width,       pOutAV->height,               is->DstAVff,                
			 SWS_FAST_BILINEAR,
			 NULL, NULL, NULL);
			 if (is->img_convert_ctx == NULL) {
				 if(is->bTraceAV)
					fprintf(stderr, "Cannot initialize the conversion context\n");
				 assert(0);
				 
			 }
			 //�����Ӳ�����٣�ת�������ˡ�
		     sws_scale(is->img_convert_ctx, pOutAV->data, pOutAV->linesize,0,pOutAV->height,vp->Bmp.data, vp->Bmp.linesize);
			 vp->picformat= is->DstAVff;
            // LOGE_KK("dex sws_scale\n");
		}else{
		      av_frame_move_ref(vp->frame,pOutAV);
			  memcpy(vp->Bmp.data,vp->frame->data,sizeof(vp->Bmp.data));
			  memcpy(vp->Bmp.linesize,vp->frame->linesize,sizeof(vp->Bmp.linesize));
			  vp->picformat=(int)format;
			  
			//  LOGE_KK("dex no copy\n");
		}
		pPictq->mutex->Lock();
		vp->uploaded=0;
        pPictq->mutex->Unlock();
        //
		/*if(is->bTraceAV) {
			int  OpenTime2= av_gettime ()/1000-OpenTime;
		    LOGE("dex:%d ,%d,%d\n",OpenTime2,pOutAV->width,pOutAV->height );
		}*/
   
	frame_queue_push(&is->pictq);
//	is->IRender->renderLock();
	return 0;
   
}  
//��Ƶ�߳�
unsigned __stdcall  Video_thread(LPVOID lpParameter)
{
	//LOGE("Video_thread start");
	SKK_VideoState *is=(SKK_VideoState*)lpParameter;
	AVPacket pkt1, *packet = &pkt1;  
	int len1, got_frame,ret;  
	AVFrame *pFrame=NULL;  
	//double pts;  
	//double duration;
	//��ŵ�����
    AVRational frame_rate =  av_guess_frame_rate(is->pFormatCtx, is->video_st, NULL);
	if(is->realtime)
	{
	   
	}else{
	   
	}

	pFrame = av_frame_alloc();//avcodec_alloc_frame();  

	short segid=0;
    short lastsegid=0;
	double lastpts=0;
	for(;!is->abort_request;)  
	{
		    if(is->abort_request){
				if(is->bTraceAV)
				LOGE_KK("Video_thread break");
                break;
			}	
			
			do
			{
LXXXX:
//				LOGE(" 2 \n");
				if(is->abort_request){
					break;
				
				}else if(packet_queue_get(&is->videoq, packet, 1,&is->viddec.pkt_serial,&segid) <= 0) 
				{
					av_usleep(5000);
					goto LXXXX;
				}
				

				if(is->audio_st==NULL)
				{
				   is->cursegid=segid; 
				}
				if(is->videoq.serial!=is->viddec.pkt_serial)
				{
                    av_free_packet(packet); 
					
				}
				is->video_clock_serial=is->viddec.pkt_serial;
			}while(is->videoq.serial!=is->viddec.pkt_serial);
         

			SKK_Decoder* d=&is->viddec;
#ifdef _WINDOWS
			///�ڷ�Ƭ����ʱ���ܼ�ⲻ��,�豸���ú����ݻָ��Ǽ����¡�
			if(is->Hard_Code==is->HARDCODE::HARD_CODE_DXVA){
				      int resetdev=1;
					  while(resetdev){
						  is->IRender->renderLock();
						  resetdev=GetD3d9RestDevState( is->viddec.avctx->opaque);
						  
						  if(resetdev==1){
							  long long Tick1=::GetTickCount64();
							  long long Tick2= is->IRender->GetOnSizeTick();
							  if(Tick1-Tick2>800)
							  {
							       Dxva2ResetDevCall(is->viddec.avctx,2);
							  }else{
							       
								   is->IRender->renderUnLock();
								   Sleep(500);
								   continue;
							  }
							  is->IRender->renderUnLock();
							  ///����Ӳ�����ô����Ľ�����ˢ�¡�
							 // Sleep(1000);
							 
						  }else if(resetdev==2){
							    Dxva2ResetDevCall(is->viddec.avctx,3);
						  }
						  is->IRender->renderUnLock();
					  }
			}
#endif
			d->pts=packet->pts;
			d->dts=packet->dts;
			
			if (packet->data != is->pflush_pkt->data) //&&is->videoq.serial==is->viddec.pkt_serial
			{
					
					if(is->Hard_Code==SKK_VideoState::HARD_CODE_DXVA)
					     is->IRender->renderLock();
					//��Ƶ����
					ret = avcodec_decode_video2(d->avctx, pFrame, &got_frame, packet);
					if(is->Hard_Code==SKK_VideoState::HARD_CODE_DXVA)
					    is->IRender->renderUnLock();
					if(got_frame)  
					{  
							//�ҵ�pts
							double pts = av_frame_get_best_effort_timestamp(pFrame); 
							pFrame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->pFormatCtx, is->video_st, pFrame);

							pFrame->pts =pts *av_q2d(is->video_st->time_base);
							if(is->AVRate!=100)
							{
								pts=pts/((float)is->AVRate/100);
							}
							pts *= av_q2d(is->video_st->time_base);
							

							  AVRational  fun={frame_rate.den, frame_rate.num};
							  is->duration = (frame_rate.num && frame_rate.den ? av_q2d(fun) : 0);
							 /* if( pts<lastpts)
							  {
							       is->frame_drops_early++;
								   av_frame_unref(pFrame);
								   got_frame = 0;
							  }else{
							      lastpts=pts;
							  }*/
						  //    double dpts =pts;
							 // if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER){
								//if (pFrame->pts != AV_NOPTS_VALUE) {
								//	double diff = dpts - get_master_clock(is);
								//	if (!isNAN(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
								//		diff - is->frame_last_filter_delay < 0     && 
								//		is->viddec.pkt_serial == is->vidclk.serial &&
								//		is->videoq.nb_packets) 
								//	{
								//		is->frame_drops_early++;
								//		//av_frame_unref(pFrame);
								//		got_frame = 0;
								//	}
								//}
							 // }
						if(queue_picture(is, pFrame, pts, is->duration , av_frame_get_pkt_pos(pFrame), is->viddec.pkt_serial) < 0)  
						{  
							//break;  
						}  
						av_frame_unref(pFrame);
					}
					
			}else {
				//av_frame_unref(pFrame);
				LOGE_KK(" video2 avcodec_flush_buffers \n");
				avcodec_flush_buffers(d->avctx);
				
				///�����˷�Ƭ�л�
				if(lastsegid!=segid&&is->pSegFormatCtx!=NULL){
					    
							for (int i = 0; i <is->pSegFormatCtx->nb_streams; i++) 
							{
								AVStream *st = is->pSegFormatCtx->streams[i];
								enum AVMediaType type = st->codec->codec_type;
								if(AVMEDIA_TYPE_VIDEO==type){
									AVCodecContext *tempavctx=is->viddec.avctx;
									seg_stream_component_open(is,i);
									if(tempavctx!=0){
										  avcodec_flush_buffers(tempavctx);
										  avcodec_close(tempavctx); 
										  avcodec_free_context(&tempavctx);
									}
									is->SegStreamState|= 0x10;
									break;
								}
							}
						
				}
				d->finished = 0;
				d->next_pts = d->start_pts;
				d->next_pts_tb = d->start_pts_tb;
			}
			
			lastsegid=segid;
			av_free_packet(packet); 
			
			
	}


	LOGE_KK("XXXX video Over \n");
	avcodec_flush_buffers(is->viddec.avctx);
	av_frame_unref(pFrame);
	av_frame_free(&pFrame);

#ifdef WIN32
	 if(is->Hard_Code==is->HARDCODE::HARD_CODE_DXVA){
	        FroceClose_Kk_Va_Dxva2(is->viddec.avctx->opaque);
	        is->viddec.avctx->opaque=NULL;
	        is->viddec.avctx->hwaccel_context=NULL;
			is->IRender->renderLock();
	        is->IRender->SetResetHardInfoCall(0,0);
	        is->IRender->renderUnLock();
	 }else if(is->Hard_Code==is->HARDCODE::HARD_CODE_QSV){
	        /*KKFreeQsv(is->viddec.avctx);
			is->viddec.avctx->opaque=NULL;
	        is->viddec.avctx->hwaccel_context=NULL;*/
	 }
#else
	if(is->Hard_Code==SKK_VideoState::HARD_CODE_MEDIACODEC){
			av_mediacodec_default_free(is->viddec.avctx);
	}
#endif
	LOGE_KK("Video_thread Over");
	
	is->viddec.decoder_tid.Addr=0;
	is->viddec.decoder_tid.ThOver=true;
	return 0;
}



//��Ļ����
unsigned __stdcall  Subtitle_thread(LPVOID lpParameter)
{
	SKK_VideoState *is=(SKK_VideoState *)lpParameter;

    SKK_Frame *sp=NULL;
	AVPacket pkt1, *packet = &pkt1;  
    int got_subtitle=0,ret=0;
	short segid=0;
    short lastsegid=0;
    double pts=0.0;
	for (;;) {

		sp=frame_queue_peek_writable(&is->subpq);
		if(sp==NULL)
						     break;
		do{
LXXXX:
//				LOGE(" 2 \n");
				if(is->abort_request){
					break;
				
				}else if(packet_queue_get(&is->subtitleq, packet, 1,&is->subdec.pkt_serial,&segid) <= 0) 
				{
					av_usleep(5000);
					goto LXXXX;
				}
				

				
				if(is->subtitleq.serial!=is->subdec.pkt_serial)
				{
                    av_free_packet(packet); 
					
				}
				
			}while(is->subtitleq.serial!=is->subdec.pkt_serial);

		SKK_Decoder* d=&is->subdec;
        pts = 0;

		if (packet->data != is->pflush_pkt->data)
		{	
			
			        
					ret=avcodec_decode_subtitle2(d->avctx, &sp->sub, &got_subtitle,packet);
					if (got_subtitle && sp->sub.format == 0) 
					{

						SKK_FrameQueue *pSubpq=&is->subpq;
					

						if (sp->sub.pts != AV_NOPTS_VALUE)
							pts = sp->sub.pts / (double)AV_TIME_BASE;
						sp->pts = pts;
						sp->serial = is->subdec.pkt_serial;
						sp->width =  is->subdec.avctx->width;
						sp->height = is->subdec.avctx->height;

						sp->uploaded = 0;

						/* now we can update the picture count */
						frame_queue_push(&is->subpq);
					} else if (got_subtitle) {
						avsubtitle_free(&sp->sub);
					}
		}else {
				//av_frame_unref(pFrame);
				LOGE_KK(" subtittle avcodec_flush_buffers \n");
				avcodec_flush_buffers(d->avctx);
				
				///�����˷�Ƭ�л�
				if(lastsegid!=segid&&is->pSegFormatCtx!=NULL){
					    
							for (int i = 0; i <is->pSegFormatCtx->nb_streams; i++) 
							{
								AVStream *st = is->pSegFormatCtx->streams[i];
								enum AVMediaType type = st->codec->codec_type;
								if(AVMEDIA_TYPE_SUBTITLE==type){
									AVCodecContext *tempavctx=is->viddec.avctx;
									seg_stream_component_open(is,i);
									if(tempavctx!=0){
										  avcodec_flush_buffers(tempavctx);
										  avcodec_close(tempavctx); 
										  avcodec_free_context(&tempavctx);
									}
									is->SegStreamState|= 0x100;
									break;
								}
							}
						
				}
				d->finished = 0;
				d->next_pts = d->start_pts;
				d->next_pts_tb = d->start_pts_tb;
			}
			
			lastsegid=segid;
			av_free_packet(packet); 
    }

	is->subdec.decoder_tid.Addr=0;
	is->subdec.decoder_tid.ThOver=true;
	return 1;
}


//������Ƶ
int audio_decode_frame( SKK_VideoState *pVideoInfo,AVFrame* frame,short *segId,short *lastsegid,int reconfigure=0) 
{  
	int n=0;
	AVCodecContext *aCodecCtx=pVideoInfo->auddec.avctx;	
	AVPacket pkt;
	memset(&pkt,0,sizeof(pkt));
	int audio_pkt_size = 0;

	int64_t dec_channel_layout;
	int len1, resampled_data_size, data_size = 0;
	int wanted_nb_samples;

	int got_frame = 0;
	do
	{
LOXXXX:
		//�Ӷ��л�ȡ����
		if(pVideoInfo->abort_request)
		{
			break;
		}else if(packet_queue_get(&pVideoInfo->audioq, &pkt, 1,&pVideoInfo->auddec.pkt_serial,segId) <= 0) 
		{
		
			av_usleep(5000);
			goto LOXXXX;
		}
		
		if(pVideoInfo->audioq.serial!=pVideoInfo->auddec.pkt_serial)
		{
			av_packet_unref(&pkt);
		}
	}while(pVideoInfo->audioq.serial!=pVideoInfo->auddec.pkt_serial);
		
	if(pkt.data == pVideoInfo->pflush_pkt->data)
	{

		if(*segId!=*lastsegid)
		{
			if(pVideoInfo->pSegFormatCtx!=0){
				for (int i = 0; i <pVideoInfo->pSegFormatCtx->nb_streams; i++) 
				{
					AVStream *st = pVideoInfo->pSegFormatCtx->streams[i];
					enum AVMediaType type = st->codec->codec_type;
					if(AVMEDIA_TYPE_AUDIO==type)
					{
						AVCodecContext *tempavctx=pVideoInfo->auddec.avctx;
						seg_stream_component_open(pVideoInfo,i);
						if(tempavctx!=0){
							  avcodec_flush_buffers(tempavctx);
							  avcodec_close(tempavctx); 
							  avcodec_free_context(&tempavctx);
						}
						pVideoInfo->SegStreamState|= 0x1;
						reconfigure=1;
						break;
					}
				}
			}
		
		}
		avcodec_flush_buffers(pVideoInfo->auddec.avctx);
		pVideoInfo->auddec.Isflush=1;
		pVideoInfo->auddec.finished = 0;

	}else{
		audio_pkt_size=pkt.size;
		len1 = avcodec_decode_audio4(aCodecCtx, frame, &got_frame, &pkt);	
		frame->pts = pkt.pts;

		if(got_frame>0)
		{
			AVRational tb =pVideoInfo->audio_st->time_base;
			if (frame->pts != AV_NOPTS_VALUE&&!pVideoInfo->realtime)
				frame->pts = av_rescale_q(frame->pts,tb , aCodecCtx->time_base);
			else /*if (frame->pkt_pts != AV_NOPTS_VALUE)
				frame->pts = av_rescale_q(frame->pkt_pts, av_codec_get_pkt_timebase(aCodecCtx), tb);
			else*/
				frame->pts=pkt.pts;
		}
	}	

	
    *lastsegid= *segId;
	av_packet_unref(&pkt);
	return got_frame;
}
static int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
								 AVFilterContext *source_ctx, AVFilterContext *sink_ctx)
{
	int ret, i;
	int nb_filters = graph->nb_filters;
	AVFilterInOut *outputs = NULL, *inputs = NULL;

	if (filtergraph) {
		outputs = avfilter_inout_alloc();
		inputs  = avfilter_inout_alloc();
		if (!outputs || !inputs) {
			ret = AVERROR(ENOMEM);
			goto fail;
		}

		outputs->name       = av_strdup("in");
		outputs->filter_ctx = source_ctx;
		outputs->pad_idx    = 0;
		outputs->next       = NULL;

		inputs->name        = av_strdup("out");
		inputs->filter_ctx  = sink_ctx;
		inputs->pad_idx     = 0;
		inputs->next        = NULL;

		if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0)
			goto fail;
	} else {
		if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
			goto fail;
	}

	/* Reorder the filters to ensure that inputs of the custom filters are merged first */
	for (i = 0; i < graph->nb_filters - nb_filters; i++)
		FFSWAP(AVFilterContext*, graph->filters[i], graph->filters[i + nb_filters]);

	ret = avfilter_graph_config(graph, NULL);
fail:
	avfilter_inout_free(&outputs);
	avfilter_inout_free(&inputs);
	return ret;
}

static void ffp_show_dict(const char *tag, AVDictionary *dict)
{
	AVDictionaryEntry *t = NULL;

	while ((t = av_dict_get(dict, "", t, AV_DICT_IGNORE_SUFFIX))) 
	{
		
	}
}

static int configure_audio_filters(SKK_VideoState *is, const char *afilters, int force_output_format)
{
	static  enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
	int sample_rates[2] = { 0, -1 };
	int64_t channel_layouts[2] = { 0, -1 };
	int channels[2] = { 0, -1 };
	AVFilterContext *filt_asrc = NULL, *filt_asink = NULL;
	char aresample_swr_opts[512] = "";
	AVDictionaryEntry *e = NULL;
	AVFilter*  Avin;
	int ret;
	char asrc_args[256];
	memset(asrc_args,0,256);
	
	

	avfilter_free(is->InAudioSrc);
	is->InAudioSrc=NULL;
	avfilter_free(is->OutAudioSink);
	is->OutAudioSink=NULL;
	
	avfilter_graph_free(&is->AudioGraph);
	is->AudioGraph=NULL;
	if (!(is->AudioGraph= avfilter_graph_alloc()))
		return AVERROR(ENOMEM);

	
	AVDictionary *swr_opts=NULL;
    ffp_show_dict("swr-opts   ", swr_opts);
	while ((e = av_dict_get(swr_opts, "", e, AV_DICT_IGNORE_SUFFIX)))
		av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);/**/

	if (strlen(aresample_swr_opts))
		aresample_swr_opts[strlen(aresample_swr_opts)-1] = '\0';
	av_opt_set(is->AudioGraph, "aresample_swr_opts", aresample_swr_opts, 0);

	
	ret = snprintf(asrc_args, sizeof(asrc_args),
		"sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d",
		is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
		is->audio_filter_src.channels,
		1, is->audio_filter_src.freq);
	
	if (is->audio_filter_src.channel_layout)
#ifdef WIN32
		//snprintf(asrc_args + ret, sizeof(asrc_args) - ret,
		//":channel_layout=0x%d",  is->audio_filter_src.channel_layout);/**/
		snprintf(asrc_args + ret, sizeof(asrc_args) - ret,
		":channel_layout=0x%"PRIx64,  is->audio_filter_src.channel_layout);/**/
#else
	snprintf(asrc_args + ret, sizeof(asrc_args) - ret,
		":channel_layout=0x%lld",  is->audio_filter_src.channel_layout);/**/
#endif

	
	ret = avfilter_graph_create_filter(&filt_asrc,
		avfilter_get_by_name("abuffer"), "ffplay_abuffer",
		asrc_args, NULL, is->AudioGraph);
	
	if (ret < 0)
		goto endXX;
   
	Avin=avfilter_get_by_name("abuffersink");

	ret = avfilter_graph_create_filter(&filt_asink,
		 Avin, "ffplay_abuffersink",
		NULL, NULL, is->AudioGraph);
	if (ret < 0)
		goto endXX;

	
	if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts,  AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
		goto endXX;
	if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0)
		goto endXX;

	if (force_output_format) {
		channel_layouts[0] = is->audio_filter_src.channel_layout;
		channels       [0] = is->audio_filter_src.channels;
		sample_rates   [0] = is->audio_filter_src.freq;
	//	sample_fmts[0]=is->audio_filter_src.fmt;
		if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0)
			goto endXX;
		if ((ret = av_opt_set_int_list(filt_asink, "channel_layouts", channel_layouts,  -1, AV_OPT_SEARCH_CHILDREN)) < 0)
			goto endXX;
		if ((ret = av_opt_set_int_list(filt_asink, "channel_counts" , channels       ,  -1, AV_OPT_SEARCH_CHILDREN)) < 0)
			goto endXX;
		if ((ret = av_opt_set_int_list(filt_asink, "sample_rates"   , sample_rates   ,  -1, AV_OPT_SEARCH_CHILDREN)) < 0)
			goto endXX;
		/*if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts,  AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
			goto endXX;*/
	}


	if ((ret = configure_filtergraph(is->AudioGraph, afilters, filt_asrc, filt_asink)) < 0)
		goto endXX;

	is->InAudioSrc  = filt_asrc;
	is->OutAudioSink = filt_asink;
endXX:if (ret < 0)
	{
		avfilter_free(is->InAudioSrc);
		avfilter_free(is->OutAudioSink);
		avfilter_graph_free(&is->AudioGraph);
	}
	return ret;
}

static inline int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
	if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
		return channel_layout;
	else
		return 0;
}
static inline int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
				   enum AVSampleFormat fmt2, int64_t channel_count2)
{
	/* If channel count == 1, planar and non-planar formats are the same */
	if (channel_count1 == 1 && channel_count2 == 1)
	return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
	else
		return channel_count1 != channel_count2 || fmt1 != fmt2;
}

 char Audioabcd[256]="";


 
//��Ƶ�����߳�
unsigned __stdcall  Audio_Thread(LPVOID lpParameter)
{
	LOGE_KK("Audio_Thread Ok");

	//return 1;
	SKK_VideoState *is=(SKK_VideoState*)lpParameter;
	AVFrame *frame = av_frame_alloc();
	
	SKK_FrameQueue *sampq=&is-> sampq;
	time_t t_start, t_end;
	t_start = time(NULL) ;
	///***�ҵ�һ�����õ�SKK_Frame***/
	SKK_Frame *af = NULL;
    SKK_Frame *af2 = NULL;
	int got_frame = 0;
	AVRational tb;
	AVRational tb2;
	int ret = 0;
    int last_serial = -1;

	if (!frame)
		return AVERROR(ENOMEM);

	
	int64_t dec_channel_layout;
	int reconfigure=0;
	short segid=0;
	short lastsegid=0;
	//�������
	do {
		
		if(is->abort_request)
			 break;
		
		if ((got_frame = audio_decode_frame(is, frame,&segid,&lastsegid,reconfigure)) < 0)
			goto the_end;
    
		if (got_frame)
		{
		
		   
           int64_t srcpts =frame->pts * av_q2d(is->auddec.avctx->time_base);
		  
		   if(is->auddec.Isflush==1)
		   {
			   if(is->AVRate!=100)
			   {
				   float aa=(float)is->AVRate/100;
				   is->Baseaudio_clock= srcpts/aa;
			   }else
			   {
                   is->Baseaudio_clock= 0;
			   }
			
			   if(isNAN(is->Baseaudio_clock))
				   is->Baseaudio_clock =0;
			   is->auddec.Isflush=0;
		   }

			dec_channel_layout = get_valid_channel_layout(frame->channel_layout, av_frame_get_channels(frame));
			if(!is->realtime)
			{
				reconfigure = cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.channels,
					(AVSampleFormat)frame->format, av_frame_get_channels(frame))    ||
					is->audio_filter_src.channel_layout != dec_channel_layout ||
					is->audio_filter_src.freq           != frame->sample_rate||
					is->auddec.pkt_serial               != last_serial;

				if(is->LastAVRate!=is->AVRate)
				{
					is->LastAVRate=is->AVRate;
					reconfigure =true;
				}
			}
		
				if (reconfigure&&!is->realtime)// 
				{

					char buf1[1024], buf2[1024];
					av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audio_filter_src.channel_layout);
					av_get_channel_layout_string(buf2, sizeof(buf2), -1, dec_channel_layout);
					av_log(NULL, AV_LOG_DEBUG,
						"Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
						is->audio_filter_src.freq, is->audio_filter_src.channels, av_get_sample_fmt_name(is->audio_filter_src.fmt), buf1, last_serial,
						frame->sample_rate, av_frame_get_channels(frame), av_get_sample_fmt_name((AVSampleFormat)frame->format), buf2, is->auddec.pkt_serial);/**/

					is->audio_filter_src.fmt            = (AVSampleFormat)frame->format;
					is->audio_filter_src.channels       = av_frame_get_channels(frame);
					is->audio_filter_src.channel_layout = dec_channel_layout;
					is->audio_filter_src.freq           = frame->sample_rate;
					last_serial                         = is->auddec.pkt_serial;
                   
					char *au=NULL;
					if(is->AVRate!=100)
					{
						 memset(Audioabcd,0,256);
                         strcat(Audioabcd,is->Atempo);
						 au=Audioabcd;
					}
					

					if ((ret = configure_audio_filters(is, au, 1)) < 0)
					{
						
						goto the_end;
					}
				
				}

		    if(!is->realtime)
			{
				if ((ret = av_buffersrc_add_frame(is->InAudioSrc, frame)) < 0)
				{
					     goto the_end;
				}

				while ((ret = av_buffersink_get_frame_flags(is->OutAudioSink, frame, 0)) >= 0) 
				{

					if(is->abort_request)
						break;
					tb = is->OutAudioSink->inputs[0]->time_base;
					if (!(af = frame_queue_peek_writable(&is->sampq)))
					{
						assert(0);
					}

					//tb �� tb2��ʱ����ȣ�ptsû�ж�Ӧת����֪Ϊɶ
					af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN :  frame->pts* av_q2d(tb);
					af->pts 	+=is->Baseaudio_clock;
					af->pos = av_frame_get_pkt_pos(frame);
					af->serial = is->auddec.pkt_serial;
                    af->segid=segid;
					AVRational avr={frame->nb_samples, frame->sample_rate};
					af->duration = av_q2d(avr);

					AVSampleFormat ff=(AVSampleFormat)frame->format;
					av_frame_unref(af->frame);
					av_frame_move_ref(af->frame, frame);
					frame_queue_push(&is->sampq);
				}
			}else{
				tb=is->audio_st->time_base;
				
				if (!(af = frame_queue_peek_writable(&is->sampq)))
				{
					assert(0);
				}

				//is->sampq.mutex->Lock();
				af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
				af->pos = av_frame_get_pkt_pos(frame);
				af->serial = is->auddec.pkt_serial;
				af->segid=segid;
				AVRational avr={frame->nb_samples, frame->sample_rate};
				af->duration = av_q2d(avr);
				AVSampleFormat ff=(AVSampleFormat)frame->format;

				av_frame_unref(af->frame);
				av_frame_move_ref(af->frame, frame);
				frame_queue_push(&is->sampq);
			}
			
		}
	} while ((ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)&&!is->abort_request);

the_end:
    avcodec_flush_buffers(is->auddec.avctx);
	av_frame_unref(frame);
	av_frame_free(&frame);
	
	is->auddec.decoder_tid.Addr=0;
	is->auddec.decoder_tid.ThOver=true;
	return ret;
}
/* seek in the stream */
void stream_seek(SKK_VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
	if (!is->seek_req) 
	{
		is->seek_pos = pos;
		is->seek_rel = rel;
		is->seek_flags &= ~AVSEEK_FLAG_BYTE;
		if (seek_by_bytes)
			is->seek_flags |= AVSEEK_FLAG_BYTE;
		is->seek_req = 1;
	}
}


///*****���������㷨,һ���㷨*******/
void RaiseVolume(char* buf, int size, int uRepeat, double vol)
//bufΪ��Ҫ������������Ƶ���ݿ��׵�ַָ�룬sizeΪ���ȣ�uRepeatΪ�ظ�������ͨ����Ϊ1��volΪ���汶��,����С��1  
{  
	if (!size)  
	{  
		return;  
	}  
	for (int i = 0; i < size;)  
	{  
		signed long minData = -0x8000; //�����8bit����������-0x80  
		signed long maxData = 0x7FFF;//�����8bit����������0xFF  

		signed short wData = buf[i + 1];  
		wData = MAKEWORD(buf[i], buf[i + 1]);  
		signed long dwData = wData;  

		for (int j = 0; j < uRepeat; j++)  
		{  
			dwData = dwData * vol;  
			if (dwData < -0x8000)  //-32768
			{  
				dwData = -0x8000;  
			}  
			else if (dwData > 0x7FFF)  //32767
			{  
				dwData = 0x7FFF;  
			}  
		}  

		wData = LOWORD(dwData);  
		buf[i] = LOBYTE(wData);  
		buf[i + 1] = HIBYTE(wData);  
		i += 2;  
	}  
}

int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec)
{
    AVDictionary    *ret = NULL;
    AVDictionaryEntry *t = NULL;
    int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                                      : AV_OPT_FLAG_DECODING_PARAM;
    char          prefix = 0;
    const AVClass    *cc = avcodec_get_class();

    if (!codec)
        codec            = s->oformat ? avcodec_find_encoder(codec_id)
                                      : avcodec_find_decoder(codec_id);

    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        prefix  = 'v';
        flags  |= AV_OPT_FLAG_VIDEO_PARAM;
        break;
    case AVMEDIA_TYPE_AUDIO:
        prefix  = 'a';
        flags  |= AV_OPT_FLAG_AUDIO_PARAM;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        prefix  = 's';
        flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
        break;
    }

    while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX)) {
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1)) {
            case  1: *p = 0; break;
            case  0:         continue;
            default:         return NULL;
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            !codec ||
            (codec->priv_class &&
             av_opt_find(&codec->priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}
AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts)
{
    int i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    opts = ( AVDictionary **)av_mallocz_array(s->nb_streams, sizeof(*opts));
    if (!opts) {
        av_log(NULL, AV_LOG_ERROR,
               "Could not alloc memory for stream options.\n");
        return NULL;
    }
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
                                    s, s->streams[i], NULL);
    return opts;
}