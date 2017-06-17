
#ifndef KK_AVCODEC_QSV_H
#define KK_AVCODEC_QSV_H
#include "../Includeffmpeg.h"
//typedef unsigned char	Uint8;
extern "C"{
#include <mfx/mfxvideo.h>
#include "libavutil/buffer.h"
}

typedef struct KK_AVQSVContext {
   
    mfxSession                    session;
    int                           iopattern;
    mfxExtBuffer                  **ext_buffers;
    int                           nb_ext_buffers;
    int opaque_alloc;
    int                           nb_opaque_surfaces;

   
    int opaque_alloc_type;

	//�������
	mfxVideoParam param;
} KK_AVQSVContext;
KK_AVQSVContext *kk_av_qsv_alloc_context(void);
int kk_qsv_profile_to_mfx(enum AVCodecID codec_id, int profile);
int kk_qsv_map_pixfmt(enum AVPixelFormat format, uint32_t *fourcc);

#endif
