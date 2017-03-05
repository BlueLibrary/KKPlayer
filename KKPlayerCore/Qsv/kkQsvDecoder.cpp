#include "../Includeffmpeg.h"
#include <tchar.h>
#include "kkQsvDecoder.h"


extern "C"{
#include <mfx/mfxdefs.h>
#include <mfx/mfxvideo.h>

}
#include "qsv.h"
void *KK_Malloc_(size_t size);
void  KK_Free_(void *ptr);

#define msdk_printf   _tprintf
#define MSDK_STRING(x) _T(x)
#define MSDK_CHAR(x) _T(x)
#define MSDK_CHECK_POINTER(P, ...)               {if (!(P)) {return __VA_ARGS__;}}
#define MSDK_PRINT_RET_MSG(ERR) {msdk_printf(MSDK_STRING("\nReturn on error: error code %d,\t%s\t%d\n\n"), (int)ERR, MSDK_STRING(__FILE__), __LINE__);}
#define MSDK_CHECK_ERROR(P, X, ERR)              {if ((X) == (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_SAFE_DELETE_ARRAY(P)                {if (P) {KK_Free_(P); P = NULL;}}
#define MSDK_ALIGN32(X) (((mfxU32)((X)+31)) & (~ (mfxU32)31))
#define MSDK_MEMCPY_VAR(dstVarName, src, count) memcpy_s(&(dstVarName), sizeof(dstVarName), (src), (count))
#define ID_BUFFER MFX_MAKEFOURCC('B','U','F','F')
#define ID_FRAME  MFX_MAKEFOURCC('F','R','M','E')

struct sBuffer
{
    mfxU32      id;
    mfxU32      nbytes;
    mfxU16      type;
};
struct sFrame
{
    mfxU32          id;
    mfxFrameInfo    info;
};
//QSVӲ���뻷��
typedef struct KKQSVDecCtx
{
	//�汾��
	mfxVersion            mfx_ver;
    mfxSession            mfx_session;
	
	//work surfaces
    mfxFrameSurface1     *surfaces;
    mfxMemId             *surface_ids;
    int                  *surface_used;

    int                   nb_surfaces;
	//�������
    mfxVideoParam         dec_param;
    mfxFrameInfo          frame_info;
	mfxSyncPoint          syncp;

	mfxFrameAllocator     frame_allocator;
	bool                  de_header;
	int                   picw;
	int                   pich;
	KK_AVQSVContext          *hw_ctx;
	
	mfxFrameAllocResponse resp;
	mfxFrameAllocRequest request;
}KKQSVDecCtx;

mfxStatus AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid)
{
    if (!mid)
        return MFX_ERR_NULL_PTR;

    if (0 == (type & MFX_MEMTYPE_SYSTEM_MEMORY))
        return MFX_ERR_UNSUPPORTED;

    mfxU32 header_size = MSDK_ALIGN32(sizeof(sBuffer));
    mfxU8 *buffer_ptr = (mfxU8 *)calloc(header_size + nbytes + 32, 1);

    if (!buffer_ptr)
        return MFX_ERR_MEMORY_ALLOC;

    sBuffer *bs = (sBuffer *)buffer_ptr;
    bs->id = ID_BUFFER;
    bs->type = type;
    bs->nbytes = nbytes;
    *mid = (mfxHDL) bs;
    return MFX_ERR_NONE;
}

mfxStatus LockBuffer(mfxMemId mid, mfxU8 **ptr)
{
    if (!ptr)
        return MFX_ERR_NULL_PTR;

    sBuffer *bs = (sBuffer *)mid;

    if (!bs)
        return MFX_ERR_INVALID_HANDLE;
    if (ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    *ptr = (mfxU8*)((size_t)((mfxU8 *)bs+MSDK_ALIGN32(sizeof(sBuffer))+31)&(~((size_t)31)));
    return MFX_ERR_NONE;
}

mfxStatus UnlockBuffer(mfxMemId mid)
{
    sBuffer *bs = (sBuffer *)mid;

    if (!bs || ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    return MFX_ERR_NONE;
}

mfxStatus FreeBuffer(mfxMemId mid)
{
    sBuffer *bs = (sBuffer *)mid;
    if (!bs || ID_BUFFER != bs->id)
        return MFX_ERR_INVALID_HANDLE;

    free(bs);
    return MFX_ERR_NONE;
}

mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    
    mfxU32 numAllocated = 0;

    mfxU32 Width2 = MSDK_ALIGN32(request->Info.Width);
    mfxU32 Height2 = MSDK_ALIGN32(request->Info.Height);
    mfxU32 nbytes;

    switch (request->Info.FourCC)
    {
    case MFX_FOURCC_YV12:
    case MFX_FOURCC_NV12:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2>>1) + (Width2>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_NV16:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2) + (Width2>>1)*(Height2);
        break;
    case MFX_FOURCC_RGB3:
        nbytes = Width2*Height2 + Width2*Height2 + Width2*Height2;
        break;
    case MFX_FOURCC_RGB4:
        nbytes = Width2*Height2 + Width2*Height2 + Width2*Height2 + Width2*Height2;
        break;
    case MFX_FOURCC_UYVY:
    case MFX_FOURCC_YUY2:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2) + (Width2>>1)*(Height2);
        break;
    case MFX_FOURCC_R16:
        nbytes = 2*Width2*Height2;
        break;
    case MFX_FOURCC_P010:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2>>1) + (Width2>>1)*(Height2>>1);
        nbytes *= 2;
        break;
    case MFX_FOURCC_A2RGB10:
        nbytes = Width2*Height2*4; // 4 bytes per pixel
        break;
    case MFX_FOURCC_P210:
        nbytes = Width2*Height2 + (Width2>>1)*(Height2) + (Width2>>1)*(Height2);
        nbytes *= 2; // 16bits
        break;
    default:
        return MFX_ERR_UNSUPPORTED;
    }

    mfxMemId* mids=new mfxMemId[request->NumFrameSuggested];
   

    for (numAllocated = 0; numAllocated < request->NumFrameSuggested; numAllocated ++)
    {
        mfxStatus sts = AllocBuffer(nbytes + MSDK_ALIGN32(sizeof(sFrame)), request->Type, &(mids[numAllocated]));

        if (MFX_ERR_NONE != sts)
            break;

        sFrame *fs;
        sts =LockBuffer(mids[numAllocated], (mfxU8 **)&fs);

        if (MFX_ERR_NONE != sts)
            break;

        fs->id = ID_FRAME;
        fs->info = request->Info;
        UnlockBuffer(mids[numAllocated]);
    }

    // check the number of allocated frames
    if (numAllocated < request->NumFrameSuggested)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    response->NumFrameActual = (mfxU16) numAllocated;
    response->mids = mids;

    return MFX_ERR_NONE;
}




static mfxStatus frame_alloc(KKQSVDecCtx * pthis, mfxFrameAllocRequest *req,mfxFrameAllocResponse *resp)
{
    KKQSVDecCtx *decode = (KKQSVDecCtx *)pthis;
    int err, i;

    if (decode->surfaces) {
        fprintf(stderr, "Multiple allocation requests.\n");
        return MFX_ERR_MEMORY_ALLOC;
    }
    /*if (!(req->Type & MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET)) {
        fprintf(stderr, "Unsupported surface type: %d\n", req->Type);
        return MFX_ERR_UNSUPPORTED;
    }*/
    if (req->Info.BitDepthLuma != 8 || req->Info.BitDepthChroma != 8 ||req->Info.Shift || req->Info.FourCC != MFX_FOURCC_NV12 ||req->Info.ChromaFormat != MFX_CHROMAFORMAT_YUV420) 
	{
        fprintf(stderr, "Unsupported surface properties.\n");
        return MFX_ERR_UNSUPPORTED;
    }

    decode->surfaces     = (mfxFrameSurface1*)av_malloc_array (req->NumFrameSuggested, sizeof(mfxFrameSurface1));
    decode->surface_ids  = (mfxMemId*)av_malloc_array (req->NumFrameSuggested, sizeof(mfxMemId));
    decode->surface_used = (int*)av_mallocz_array(req->NumFrameSuggested, sizeof(int));
    if (!decode->surfaces || !decode->surface_ids || !decode->surface_used)
        goto fail;

   
    decode->nb_surfaces = req->NumFrameSuggested;

	 AllocImpl(req,resp);
    for (i = 0; i < decode->nb_surfaces; i++)
	{
		mfxMemId iix=resp->mids[i];
		decode->surface_ids[i] =resp->mids[i];
		MSDK_MEMCPY_VAR(decode->surfaces[i].Info, &( req->Info), sizeof(mfxFrameInfo));
   
	}

    resp->mids           = decode->surface_ids;
    resp->NumFrameActual = decode->nb_surfaces;
    decode->frame_info = req->Info;

    return MFX_ERR_NONE;
fail:
    av_freep(&decode->surfaces);
    av_freep(&decode->surface_ids);
    av_freep(&decode->surface_used);

    return MFX_ERR_MEMORY_ALLOC;
}



static void WipeMfxBitstream(mfxBitstream* pBitstream)
{
    MSDK_CHECK_POINTER(pBitstream);

    //free allocated memory
    MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize)
{
    MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);

    MSDK_CHECK_ERROR(nSize <= pBitstream->MaxLength, true, MFX_ERR_UNSUPPORTED);

    mfxU8* pData =  (mfxU8*)KK_Malloc_(nSize);
    MSDK_CHECK_POINTER(pData, MFX_ERR_MEMORY_ALLOC);

    memmove(pData, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);

    WipeMfxBitstream(pBitstream);

    pBitstream->Data       = pData;
    pBitstream->DataOffset = 0;
    pBitstream->MaxLength  = nSize;

    return MFX_ERR_NONE;
}
mfxStatus InitMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize)
{
    //check input params
    MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);
    MSDK_CHECK_ERROR(nSize, 0, MFX_ERR_NOT_INITIALIZED);

    //prepare pBitstream
    WipeMfxBitstream(pBitstream);

    //prepare buffer
    pBitstream->Data = (mfxU8*)KK_Malloc_(nSize);
    MSDK_CHECK_POINTER(pBitstream->Data, MFX_ERR_MEMORY_ALLOC);

    pBitstream->MaxLength = nSize;

    return MFX_ERR_NONE;
}


static enum AVPixelFormat Qsv_GetFormat( AVCodecContext *p_context,const enum AVPixelFormat *pix_fmts )
{
	
	
	 while (*pix_fmts != AV_PIX_FMT_NONE)
	 {
        if(*pix_fmts == AV_PIX_FMT_QSV)
		{
            return AV_PIX_FMT_QSV;
		}
        pix_fmts++;
    }

	return avcodec_default_get_format( p_context,pix_fmts );
}

static void free_buffer(void *opaque, uint8_t *data)
{
    int *used = (int *)opaque;
    *used = 0;
    av_freep(&data);
}
static int Qsv_GetFrameBuf( struct AVCodecContext *avctx, AVFrame *frame,int flags)
{
	KKQSVDecCtx *decode = (KKQSVDecCtx*)avctx->opaque;

	if(decode->nb_surfaces==0){
	       MFXVideoDECODE_QueryIOSurf(decode->mfx_session, &decode->hw_ctx->param, &decode->request);
		   frame_alloc(decode, &decode->request,&decode->resp);
	}

    mfxFrameSurface1 *surf;
    AVBufferRef *surf_buf;
    int idx;

    for (idx = 0; idx < decode->nb_surfaces; idx++) {
        if (!decode->surface_used[idx])
		{
            break;
		}
    }
    if (idx == decode->nb_surfaces) {
        fprintf(stderr, "No free surfaces\n");
        return AVERROR(ENOMEM);
    }

	
    surf =&decode->surfaces[idx]; //( mfxFrameSurface1 *)av_mallocz(sizeof(*surf));
    if (!surf)
        return AVERROR(ENOMEM);

    surf_buf = av_buffer_create((uint8_t*)surf, sizeof(*surf), free_buffer,&decode->surface_used[idx], AV_BUFFER_FLAG_READONLY);
    if (!surf_buf) {
        av_freep(&surf);
        return AVERROR(ENOMEM);
    }

	
    frame->buf[0]  = surf_buf;
    frame->data[3] = (uint8_t*)surf;

    decode->surface_used[idx] = 1;

	return 0;
	/*mfxFrameSurface1 *surf=NULL;
    AVBufferRef *surf_buf=NULL;
    int idx;

    for (idx = 0; idx < decode->nb_surfaces; idx++) {
        if (!decode->surface_used[idx])
            break;
    }
    if (idx == decode->nb_surfaces) {
        fprintf(stderr, "No free surfaces\n");
        return AVERROR(ENOMEM);
    }

    surf =(mfxFrameSurface1 *) av_mallocz(sizeof(*surf));
    if (!surf)
        return AVERROR(ENOMEM);
    surf_buf = av_buffer_create((uint8_t*)surf, sizeof(*surf), free_buffer,
                                &decode->surface_used[idx], AV_BUFFER_FLAG_READONLY);
    if (!surf_buf) {
        av_freep(&surf);
        return AVERROR(ENOMEM);
    }

    surf->Info       = decode->frame_info;
    surf->Data.MemId = &decode->surfaces[idx];

    frame->buf[0]  = surf_buf;
    frame->data[3] = (uint8_t*)surf;

    decode->surface_used[idx] = 1;
    return 0;*/
}




void Registerkk_h264_qsv_decoder();
int BindQsvModule(AVCodecContext  *pCodecCtx)
{

    Registerkk_h264_qsv_decoder();
	if(pCodecCtx->codec_id==AV_CODEC_ID_H264)
	{
		//pCodecCtx->codec_id=
		pCodecCtx->get_format         = Qsv_GetFormat;
	    pCodecCtx->get_buffer2        = Qsv_GetFrameBuf;
	    pCodecCtx->thread_count       = 1;
	    pCodecCtx->slice_flags       |= SLICE_FLAG_ALLOW_FIELD;

		//Ӳ��
		mfxIMPL impl =MFX_IMPL_HARDWARE;
		
		KKQSVDecCtx *decCtx=(KKQSVDecCtx*)::KK_Malloc_(sizeof(KKQSVDecCtx));
		decCtx->hw_ctx =kk_av_qsv_alloc_context();
		pCodecCtx->hwaccel_context=decCtx->hw_ctx;
		pCodecCtx->opaque=decCtx;

      
		
		decCtx->picw=pCodecCtx->width;
		decCtx->pich=pCodecCtx->height;
		//h264
		decCtx->dec_param.mfx.CodecId=MFX_CODEC_AVC;
		decCtx->mfx_ver.Major = 1;
	    decCtx->mfx_ver.Minor = 0;
	    mfxStatus sts=MFXInit(impl,&decCtx->mfx_ver, &decCtx->mfx_session); 
		if(sts!= MFX_ERR_NONE){
			  return sts;
		}
	    sts = MFXQueryVersion(decCtx->mfx_session , &decCtx->mfx_ver);  //get real API version of the loaded library
		if(sts!= MFX_ERR_NONE){
		     return sts;
		}
	    MFXClose(decCtx->mfx_session);
       decCtx->mfx_session=0;

	    sts=MFXInit(impl,&decCtx->mfx_ver , &decCtx->mfx_session); 
		if(sts!= MFX_ERR_NONE){
		       return sts;
		}


		/*decCtx->frame_allocator.pthis = decCtx;
        decCtx->frame_allocator.Alloc = frame_alloc;
        decCtx->frame_allocator.Lock  = frame_lock;
        decCtx->frame_allocator.Unlock = frame_unlock;
        decCtx->frame_allocator.GetHDL = frame_get_hdl;
        decCtx->frame_allocator.Free   = frame_free;
		decCtx->de_header=false;*/
	 //   MFXVideoCORE_SetHandle(decCtx->mfx_session, MFX_HANDLE_VA_DISPLAY, decode.va_dpy);
		//MFXVideoCORE_SetFrameAllocator(decCtx->mfx_session, &decCtx->frame_allocator);
        if(sts!= MFX_ERR_NONE){
		       return sts;
		}

		/* if (MFX_CODEC_CAPTURE != decCtx->dec_param.mfx.CodecId){
            sts = InitMfxBitstream(&decCtx->hw_ctx->mfx_enc_bs, 1024 * 1024);
            if(sts!= MFX_ERR_NONE){
		       return sts;
		    }
         }*/

		 decCtx->hw_ctx->iopattern=MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
		 decCtx->hw_ctx->session=decCtx->mfx_session;
		return 0;
		//sts = MFXVideoDECODE_DecodeHeader(decCtx->mfx_session, &decCtx->mfx_enc_bs, &decCtx->dec_param);
		//if (!sts){
  //          //m_bVppIsUsed = IsVppRequired(pParams);
		//	 decCtx->de_header=true;
		//}else{
		//     decCtx->de_header=false;
		//}
		//
		////��Ƶ�Ƿ���ת
		//decCtx->dec_param.mfx.Rotation =0;// MFX_ROTATION_0;

		//decCtx->dec_param.IOPattern =MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

		//decCtx->dec_param.AsyncDepth=4;

		// sts=MFXVideoDECODE_Query(decCtx->mfx_session,&decCtx->dec_param, &decCtx->dec_param);

		// mfxFrameAllocRequest Request;
		// // calculate number of surfaces required for decoder
  //       sts = MFXVideoDECODE_QueryIOSurf(decCtx->mfx_session,&decCtx->dec_param,&Request);
  //       //��ʼ��������
		// sts =MFXVideoDECODE_Init(decCtx->mfx_session,&decCtx->dec_param);

		// sts =MFXVideoDECODE_GetVideoParam(decCtx->mfx_session,&decCtx->dec_param);


		// mfxFrameSurface1     *CurSurface=NULL;
		// mfxFrameSurface1     *pOutSurface = NULL;
		//  MFXVideoDECODE_DecodeFrameAsync(decCtx->mfx_session,&decCtx->mfx_enc_bs,CurSurface,&pOutSurface,&decCtx->syncp);
		return 0;
	}
	return -1;

}


