/**************************kkplayer*********************************************/
/*******************Copyright (c) Saint ******************/
/******************Author: Saint *********************/
/*******************Author e-mail: lssaint@163.com ******************/
/*******************Author qq: 784200679 ******************/
/*******************KKPlayer  WWW: http://www.70ic.com/KKplayer ********************************/
/*************************date��2015-6-25**********************************************/
//#define __STDC_HOSTED__ 0
#include "stdafx.h"
#ifndef Includeffmpeg_H_
#define Includeffmpeg_H_
//typedef unsigned char	Uint8;
extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#define UINT64_C(x)  (x ## ULL)
//#include <stdint.h>
#include "../libx86/ffmpeg/include/libavutil/avutil.h"
#include "../libx86/ffmpeg/include/libavutil/avstring.h"
#include "../libx86/ffmpeg/include/libavutil/intreadwrite.h"
#include "../libx86/ffmpeg/include/libavutil/eval.h"
#include "../libx86/ffmpeg/include/libavutil/mathematics.h"
#include "../libx86/ffmpeg/include/libavutil/pixdesc.h"
#include "../libx86/ffmpeg/include/libavutil/imgutils.h"
#include "../libx86/ffmpeg/include/libavutil/dict.h"
#include "../libx86/ffmpeg/include/libavutil/parseutils.h"
#include "../libx86/ffmpeg/include/libavutil/samplefmt.h"
#include "../libx86/ffmpeg/include/libavutil/avassert.h"
#include "../libx86/ffmpeg/include/libavutil/time.h"
#include "../libx86/ffmpeg/include/libavformat/avformat.h"
#include "../libx86/ffmpeg/include/libavdevice/avdevice.h"
#include "../libx86/ffmpeg/include/libswscale/swscale.h"
#include "../libx86/ffmpeg/include/libavutil/opt.h"
#include "../libx86/ffmpeg/include/libavcodec/avfft.h"
#include "../libx86/ffmpeg/include/libswresample/swresample.h"
#include "../libx86/ffmpeg/include/libavfilter/avfilter.h"

# include "../libx86/ffmpeg/include/libavfilter/avcodec.h"
# include "../libx86/ffmpeg/include/libavfilter/avfilter.h"
# include "../libx86/ffmpeg/include/libavfilter/buffersink.h"
# include "../libx86/ffmpeg/include/libavfilter/buffersrc.h"
#include  "../libx86/ffmpeg/include/libavutil/mathematics.h"
#ifdef WIN32
	#ifdef _DEBUG
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\avcodec.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\avdevice.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\avfilter.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\avformat.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\avutil.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\postproc.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\swresample.lib")
		#pragma comment (lib,"..\\libx86\\ffmpeg\\lib\\swscale.lib")
	#else
		#pragma comment (lib,"..\\libx86\\ffmpeg\\vs2008lib\\avcodec-56.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\avdevice-56.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\avfilter-5.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\avformat-56.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\avutil-54.lib")
		//#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\postproc.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\swresample-1.lib")
		#pragma comment (lib,"../libx86/ffmpeg/vs2008lib\\swscale-3.lib")
	#endif
#endif
}

#define MAX_QUEUE_SIZE (1024 * 1024 * 8)  //1048576*8   8M
#define MIN_FRAMES 5

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512*8
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
#define AV_NOSYNC_THRESHOLD 10.0
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1


#endif

#ifndef Nan_h__
#define Nan_h__


//#define NAN log(-1.0) //��ЧֵNaN
#define isNAN(x) ((x)!=(x))
#define NINF log(0.0) //�������
#define INF -NINF //�����
#define PINF -NINF //�������
#define isINF(x) (((x)==PINF)||((x)==NINF))
#define isPINF(x) ((x)==PINF)
#define isNINF(x) ((x)==NINF)
#endif // Nan_h__