typedef unsigned char      uint8_t;
typedef long long          int64_t;
#ifndef KKPlugin_H_
#define KKPlugin_H_

#define KK_MKTAG(a,b,c,d)                 ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define KK_PERRTAG(a, b, c, d)            (-(int)KK_MKTAG(a, b, c, d))
#define KK_AVERROR_EOF                       KK_PERRTAG( 'E','O','F',' ') ///< End of file
#define KK_AVERROR(e)                       (-(e))
#define KK_ERROR_HTTP_NOT_FOUND           FFERRTAG(0xF8,'4','0','4')
extern "C"
{  
	
	//�õ�Э��ͷ
	typedef char (*fGetPtlHeader)(char *buf,char len);
	/***********�����ͷ��ڴ�**************/
	typedef void (*fFree)(void*);
    /****���غ���****/
    typedef char (*fKKDownAVFile)(char *strUrl);
	/********��ͣ���ػ��������������غ���**********/
	typedef void (*fKKPauseDownAVFile)(char *strUrl,bool Pause);
	
	
	/************�õ������ٶ���Ϣ*********/
    typedef bool (*fKKDownAVFileSpeedInfo)(const char *strurl,char *jsonBuf,int len);
    //ˢ�¶���
	typedef void (*fFlushPlayerQue)(void *opaque);
	//�����ӳٻص�����AVType 0,��Ƶ��1��Ƶ
	typedef void (*fCalPlayerDelay)(void *opaque,int64_t Pts,int AVType);

	typedef int  (*fIo_read_packet)(void *opaque, uint8_t *buf, int buf_size);
    //�ļ�seek
	typedef int64_t  (*fIo_seek)(void *opaque, int64_t offset, int whence);
	
	//�õ�����ʱ��
	typedef unsigned int     (*fGetCacheTime)(void *opaque);

	//url��������
	typedef char* (*fKKUrlParser)(const char *strurl,int *abort_request);
	//
	typedef int  (*fKKIRQ)(void *opaque);  //ǿ���жϺ���
    typedef void (*fSetNeedWait)(void* PlayerOpaque,bool wait);
	typedef struct __KKPlugin
	{
        void *opaque;                               //�û����ݣ�      �������
		fIo_read_packet kkread;                     //��������        �������
		fIo_seek        kkseek;                     //seek����        �������
		fFlushPlayerQue FlushQue;                   //�������� kkirqOpaque,ˢ�¶��к���
		fCalPlayerDelay CalPlayerDelay;             //�ⲿ�ص�������Player����
		fGetCacheTime   GetCacheTime;
		fKKIRQ          kkirq;                      //�ⲿ����,�������ڲ��������Ӧ���ã�����1�жϣ���������kkirqOpaque
		fSetNeedWait    SetNeedWait;                //����������
        void  *PlayerOpaque;                        //����������
		char  *URL;                                 //ȥ��Э��ͷ�ĵ�ַ
		int   RealTime;                             //1Ϊʵʱ������ӦΪ0
		int   FirstRead;                            //��һ�ζ���1,����Ϊ0�����ֶ�
	}KKPlugin;
	
	//����һ�����ʵ��
	typedef KKPlugin*  (*fCreateKKPlugin)();
	typedef void (*fDeleteKKPlugin)(KKPlugin*p);
	typedef struct __KKPluginInfo
	{
		/***�������***/
		char ptl[32];
		/******����һ�����******/
		fCreateKKPlugin        CreKKP;
		/**********ɾ��һ�����**************/
		fDeleteKKPlugin        DelKKp;
		/***********�����ļ�*****************/
		fKKDownAVFile          KKDownAVFile;
		/************��ͣ������������*****************/
		fKKPauseDownAVFile     KKPauseDownAVFile;
		/*************�õ������ٶ���Ϣ************/
		fKKDownAVFileSpeedInfo KKDownAVFileSpeedInfo;
		/*****************�ͷ��ڴ�*************/
		fFree KKFree;
		/***********����url***************/
		fKKUrlParser           KKUrlParser;
#ifdef WIN32
		HMODULE	Handle;
#else
		void*  Handle;/*****************/
#endif
	}KKPluginInfo;

	
};
#endif