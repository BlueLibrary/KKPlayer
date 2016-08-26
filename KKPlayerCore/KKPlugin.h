typedef unsigned char      uint8_t;
typedef long long          int64_t;
#ifndef KKPlugin_H_
#define KKPlugin_H_
extern "C"
{   typedef char (*fGetPtlHeader)(char *buf,char len);
    /****���غ���****/
    typedef char (*fKKDownAVFile)(char *strUrl);
	/********ֹͣ���غ���**********/
	typedef char (*fKKStopDownAVFile)(char *str);
	
	
	/************�����ٶȻص�����*********/
    //typedef char (*fKKDownAVFileSpeed)(char *str,int DownSpeed);

	typedef void (*fFlushPlayerQue)(void *opaque);
	typedef int  (*fIo_read_packet)(void *opaque, uint8_t *buf, int buf_size);
    //�ļ�seek
	typedef int64_t  (*fIo_seek)(void *opaque, int64_t offset, int whence);
	//
	typedef int  (*fKKIRQ)(void *opaque);  //ǿ���жϺ���

	typedef struct __KKPlugin
	{
        void *opaque;                 //�û�����
		fIo_read_packet kkread;
		fIo_seek kkseek;
		fFlushPlayerQue FlushQue;    //�������� kkirqOpaque
		fKKIRQ kkirq;                //�ⲿ����,�������ڲ��������Ӧ���ã�����1�жϣ���������kkirqOpaque
        void *kkirqOpaque;           //�ⲿ����
		char *URL;
	}KKPlugin;
	
	//����һ�����ʵ��
	typedef KKPlugin*  (*fCreateKKPlugin)();
	typedef void (*fDeleteKKPlugin)(KKPlugin*p);
	typedef struct __KKPluginInfo
	{
		char ptl[32];
		fCreateKKPlugin CreKKP;
		fDeleteKKPlugin DelKKp;
		fKKDownAVFile   KKDownAVFile;
		fKKStopDownAVFile KKStopDownAVFile;
	//	fKKDownAVFileSpeed KKDownAVFileSpeed;
#ifdef WIN32
		HMODULE	Handle;
#else
		void*  Handle;/*****************/
#endif
	}KKPluginInfo;

	
};
#endif