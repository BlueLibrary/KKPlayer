typedef unsigned char      uint8_t;
typedef long long          int64_t;
#ifndef KKPlugin_H_
#define KKPlugin_H_
extern "C"
{   typedef char (*fGetPtlHeader)(char *buf,char len);
	typedef int  (*fIo_read_packet)(void *opaque, uint8_t *buf, int buf_size);
	typedef int64_t  (*fIo_seek)(void *opaque, int64_t offset, int whence);
	typedef int  (*fKKIRQ)(void *opaque);  //ǿ���жϺ���

	typedef struct __KKPlugin
	{
        void *opaque;                 //ָ��һ�����ʵ������   pointer a instance of KKPlugin.
		fIo_read_packet kkread;
		fIo_seek kkseek;
		fKKIRQ kkirq;                //�ⲿ����
        void *kkirqOpaque;   
		char *URL;
	}KKPlugin;
	
	typedef KKPlugin*  (*fCreateKKPlugin)();
	typedef void (*fDeleteKKPlugin)(KKPlugin*p);
	typedef struct __KKPluginInfo
	{
		char ptl[32];
		fCreateKKPlugin CreKKP;
		fDeleteKKPlugin DelKKp;
		
#ifdef WIN32
		HMODULE	Handle;
#else
		void*  Handle;/*****************/
#endif
	}KKPluginInfo;

	
};
#endif