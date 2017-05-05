#include "KKVP.h"
//errno.h
#include "../../QyIPC/Qy_Ipc_Manage.h"
#include "KKV_ReceiveData.h"
#include <map>
#include <string> 
#include "./../KKPlayer/KKPlayerCore/KKError.h"
#include "../../KKPlayer/KKPlayerCore/KKPlugin.h"
#include "json/json.h"

#include <process.h>
#include <tlhelp32.h>

#define KKMKTAG(a,b,c,d)                 ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define KKPERRTAG(a, b, c, d)            (-(int)KKMKTAG(a, b, c, d))
#define AVERROR_EOF                       KKPERRTAG( 'E','O','F',' ') ///< End of file
#define AVERROR(e)                       (-(e))
#define AVERROR_HTTP_NOT_FOUND           FFERRTAG(0xF8,'4','0','4')


Qy_IPC::Qy_Ipc_Manage *G_pInstance=NULL;
typedef unsigned char      uint8_t;
typedef long long          int64_t;

//���е�Ipc�����Ľ��
std::map<std::string,IPC_DATA_INFO>                 G_guidBufMap;
std::map<std::string,unsigned int>                  G_CacheTimeMap;
std::map<std::string,HANDLE>                        G_guidHMap;
std::map<std::string,std::map<std::string,HANDLE>*> G_URLRequestInfoMap;
std::map<std::string,std::string>                   G_SpeedInfoMap;

Qy_IPC::CKKV_ReceiveData                            *G_pKKV_Rec=NULL;
Qy_IPC::CKKV_DisConnect                             *G_pKKV_Dis=NULL;
Qy_IPC::Qy_IPc_InterCriSec                           G_KKMapLock;
//IPC ״̬����
int G_IPC_Read_Write=-1;
int OpenIPc();

bool GetSpeedInfo(const char *strurl,char *jsonBuf,int len)
{
	bool aaa=false;
	G_KKMapLock.Lock();
	std::map<std::string,std::string>::iterator It=G_SpeedInfoMap.find(strurl);
	if(It!=G_SpeedInfoMap.end()){
		if(len>It->second.length())
		{
		    strcpy(jsonBuf,It->second.c_str());
			aaa=true;
		}
	}
	G_KKMapLock.Unlock();
    return aaa;
}
static std::basic_string<TCHAR> g_strModuleFileName;
const  std::basic_string<TCHAR>& XGetModuleFilename()
{

	if (g_strModuleFileName.empty())
	{
		if(g_strModuleFileName.empty())
		{
			TCHAR filename[MAX_PATH] = { 0 };
			::GetModuleFileName(NULL, filename, _countof(filename));
			g_strModuleFileName = filename;
		}
	}
	return g_strModuleFileName;
}
std::basic_string<TCHAR> GetModulePath()
{
	std::basic_string<TCHAR> strModuleFileName = XGetModuleFilename();
	unsigned int index = strModuleFileName.find_last_of(L"\\");
	if (index != std::string::npos)
	{
		return strModuleFileName.substr(0, index);
	}
	return L"";
}


void AddIPCGuid(std::string& strGuid,IPC_DATA_INFO &xxda)
{
	G_KKMapLock.Lock();
	G_guidBufMap.insert(std::pair<std::string,IPC_DATA_INFO>(strGuid,xxda));
	G_KKMapLock.Unlock();
}



void GetIPCOpRet(std::string& strGuid,bool& Ok,IPC_DATA_INFO &OutInfo)
{
	memset(&OutInfo,0,sizeof(OutInfo));
	G_KKMapLock.Lock();
	std::map<std::string,IPC_DATA_INFO>::iterator Itx=G_guidBufMap.find(strGuid);
	if(Itx!=G_guidBufMap.end())
	{
		Ok=true;
		OutInfo=Itx->second;
		G_guidBufMap.erase(Itx);
	}
	G_KKMapLock.Unlock();
}

bool KillProcessFromName(std::wstring strProcessName)  
{  
	transform(strProcessName.begin(), strProcessName.end(), strProcessName.begin(),  toupper);   
	//�������̿���(TH32CS_SNAPPROCESS��ʾ�������н��̵Ŀ���)  
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);  

	//PROCESSENTRY32���̿��յĽṹ��  
	PROCESSENTRY32 pe;  

	//ʵ������ʹ��Process32First��ȡ��һ�����յĽ���ǰ�����ĳ�ʼ������  
	pe.dwSize = sizeof(PROCESSENTRY32);  


	//�����IFЧ��ͬ:  
	//if(hProcessSnap == INVALID_HANDLE_VALUE)   ��Ч�ľ��  
	if(!Process32First(hSnapShot,&pe))  
	{  
		return false;  
	}  

	//���ַ���ת��ΪСд  
	//strProcessName.MakeLower();  

	//��������Ч  ��һֱ��ȡ��һ�����ѭ����ȥ  
	while (Process32Next(hSnapShot,&pe))  
	{  

		//pe.szExeFile��ȡ��ǰ���̵Ŀ�ִ���ļ�����  
		std::wstring scTmp = pe.szExeFile;


		//����ִ���ļ���������Ӣ����ĸ�޸�ΪСд  
		transform(scTmp.begin(), scTmp.end(), scTmp.begin(),  toupper);   

		//�Ƚϵ�ǰ���̵Ŀ�ִ���ļ����ƺʹ��ݽ������ļ������Ƿ���ͬ  
		//��ͬ�Ļ�Compare����0  
		if(scTmp==strProcessName)  
		{  

			//�ӿ��ս����л�ȡ�ý��̵�PID(������������е�PID)  
			DWORD dwProcessID = pe.th32ProcessID;  
			HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE,FALSE,dwProcessID);  
			::TerminateProcess(hProcess,0);  
			CloseHandle(hProcess);  
			return true;  
		}  

	}  
	return false;  
}  

//FILE *TestFb=NULL;
int OpenIPc()
{
	int recount=0;
ReOpen:
	if(G_pInstance==NULL)
		return 0;
	bool Ok=G_pInstance->OpenServerPipe("\\\\.\\Pipe\\KKPlayer_Res_70ic");
	if(!Ok){
		if(recount<20){
			recount++;
			Sleep(20);
			goto ReOpen;
		}
		G_IPC_Read_Write=0;
		return 0;
	}else{
		G_IPC_Read_Write=1;
	}
	G_pInstance->Start();
	/*if(TestFb!=NULL)
		fclose(TestFb);
	TestFb=fopen("D:/sssssss.mp4","wb");*/
	return 1;
}

//��ϢͨѶ��ʽ�涨��
//buflen+data;
int   InitIPC(){
	
#ifndef _DEBUG
	{
		std::wstring kkres=GetModulePath();
		kkres+=L"//kkres//kkRes.exe";


		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		memset(&pi,0,sizeof(pi));
		memset(&si,0,sizeof(si));
		KillProcessFromName(L"kkres.exe");
		BOOL ret = CreateProcess(kkres.c_str(),NULL, NULL, NULL, FALSE, 0, NULL, NULL,&si, &pi);
	
	}
#endif
	if(G_pKKV_Rec==NULL)
	{
         G_pKKV_Rec= new Qy_IPC::CKKV_ReceiveData();
	}
	if(G_pKKV_Dis==NULL)
	{
		G_pKKV_Dis= new Qy_IPC::CKKV_DisConnect();
	}
	if(G_pInstance!=NULL)
	{
		delete G_pInstance;
		G_pInstance=NULL;
	}
	
	G_pInstance =new Qy_IPC::Qy_Ipc_Manage();
	G_pInstance->Init(G_pKKV_Rec,Qy_IPC::QyIpcClient,G_pKKV_Dis);
	int lx=OpenIPc();
	if(lx==0){
		delete G_pInstance;
		G_pInstance=NULL;
	}
	
	return lx;
}

void  CreatStrGuid(std::string &strGuid)
{
	GUID stGuid;
	::CoCreateGuid(&stGuid);
	char abcd[256]="";
	sprintf(abcd,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", stGuid.Data1,stGuid.Data2,stGuid.Data3,
		stGuid.Data4[0],stGuid.Data4[1],stGuid.Data4[2],stGuid.Data4[3],
		stGuid.Data4[4],stGuid.Data4[5],stGuid.Data4[6],stGuid.Data4[7]);
	strGuid=abcd;
}

int   KKVWritePipe(unsigned char *pBuf,int Len,HANDLE hPipeInst)
{
	int lx=0;
	G_KKMapLock.Lock();
	if(G_pInstance==NULL||G_IPC_Read_Write==2)
	{
		lx=InitIPC();
		if(lx==1)
		{
			G_pInstance->WritePipe(pBuf,Len,hPipeInst);
		}
	}else{
		if(G_IPC_Read_Write==1)
		{
            G_pInstance->WritePipe(pBuf,Len,hPipeInst);
			lx=1;
		}
	}
	G_KKMapLock.Unlock();
	return lx;
}

extern "C"{

char __declspec(dllexport)KKCloseAVFile(char *strUrl);
char __declspec(dllexport) GetPtlHeader(char *buf,char len)
{
	
	memset(buf,0,len);
	memcpy(buf,"kkv",4);
	return 0;
}

void __declspec(dllexport) KKFree(void* p)
{
   if(p!=NULL)
	   free(p);
}




int  __declspec(dllexport) Kkv_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	
LOOP1:
	KKPlugin* KKP=(KKPlugin*)opaque;
	KKP->SetNeedWait(KKP->PlayerOpaque,true);

	std::string strGuid;
	CreatStrGuid(strGuid);

    memset(buf,0,buf_size);
	HANDLE hRead=CreateEvent(NULL, TRUE, FALSE, NULL);
	IPC_DATA_INFO xxda;
	xxda.pBuf=buf;
	xxda.BufLen=buf_size;
	xxda.DataSize=-1000;
	xxda.hWait=hRead;
	xxda.CacheTime=0;
	
	AddIPCGuid(strGuid,xxda);

	Json::Value jsonValue;
	jsonValue["IPCMSG"]=IPCRead;
	jsonValue["Guid"]=strGuid;
	jsonValue["Url"]=KKP->URL;
    jsonValue["HRW"]=(int)hRead;
	jsonValue["FirstRead"]=KKP->FirstRead;
	KKP->FirstRead=0;
	if(buf_size>32768)
		 jsonValue["BufLen"]=32768;
	else
    jsonValue["BufLen"]=buf_size;


	strGuid=jsonValue.toStyledString();
	int buflen=strGuid.length()+1024;
	unsigned char *IPCbuf=(unsigned char*)::malloc(buflen);
	memset(IPCbuf,0,buflen);
	memcpy(IPCbuf,strGuid.c_str(),strGuid.length());

	int rext=KKVWritePipe(IPCbuf,buflen,0);
	while(1&&G_IPC_Read_Write==1&&rext==1)
	{
		DWORD ret=::WaitForSingleObject( hRead,50);
        if(ret==WAIT_OBJECT_0)
		{
			break;
		}
		if(KKP->opaque==NULL&&KKP->kkirq!=NULL&&KKP->kkirq(KKP->PlayerOpaque)==1)
		{
			break;
		}
	}
	
	
	::free(IPCbuf);
	::CloseHandle(hRead);
	
	if(KKP->opaque==NULL&&KKP->kkirq!=NULL&&KKP->kkirq(KKP->PlayerOpaque)==1||rext==0)
	{

		if(rext==0)
			return  AVERROR(ERROR_BROKEN_PIPE);

		return AVERROR(errno);
	}
	
	IPC_DATA_INFO OutInfo;
	bool RetOk=false;
	strGuid=jsonValue["Guid"].asString();
	GetIPCOpRet(strGuid,RetOk,OutInfo);
	if(RetOk)
	{
		int ret=OutInfo.DataSize;
		if(ret==-1001){
		    return AVERROR_EOF;
		}
			
		if(KKP->CalPlayerDelay!=NULL)
		{
			KKP->CalPlayerDelay(KKP->PlayerOpaque,OutInfo.CacheTime,2);
		}
		return ret;
	}
	

	return AVERROR_EOF;
}

//buflen+data;
//data:guidlen +guid+ msgId+h+FileURLSize+FileURL+offset+whence
//Reply:buflen+data;
//data:guidlen+guid+msgId+h+FileInfoSize+FileInfo+Pos; 
int64_t  __declspec(dllexport) Kkv_seek(void *opaque, int64_t offset, int whence)//guidlen+guid+msgId+h+FileURL+FileURL+Buf_Size
{
LOOP1:
	KKPlugin* KKP=(KKPlugin*)opaque;
	

	std::string strGuid;
	CreatStrGuid(strGuid);


	HANDLE hRead=CreateEvent(NULL, TRUE, FALSE, NULL);
	IPC_DATA_INFO xxda;
	xxda.pBuf=0;
	xxda.BufLen=0;
	xxda.DataSize=-1000;
	xxda.hWait=hRead;
	xxda.CacheTime=0;
	AddIPCGuid(strGuid,xxda);

	Json::Value jsonValue;
	jsonValue["IPCMSG"]=IPCSeek;
	jsonValue["Guid"]=strGuid;
	jsonValue["Url"]=KKP->URL;
    jsonValue["HRW"]=(int)hRead;
    jsonValue["offset"]=offset;
    jsonValue["whence"]=whence;

	strGuid=jsonValue.toStyledString();
	int buflen=strGuid.length()+1024;
	unsigned char *IPCbuf=(unsigned char*)::malloc(buflen);
	memset(IPCbuf,0,buflen);
	memcpy(IPCbuf,strGuid.c_str(),strGuid.length());

    int rext=KKVWritePipe(IPCbuf,buflen,0);
	while(1&&G_IPC_Read_Write==1&&rext==1)
	{
		DWORD ret=::WaitForSingleObject( hRead,50);
		if(ret==WAIT_OBJECT_0)
		{
			break;
		}
		if(KKP->opaque==NULL&&KKP->kkirq!=NULL&&KKP->kkirq(KKP->PlayerOpaque)==1)
		{
			break;
		}
	}


	//::WaitForSingleObject( hRead,INFINITE);
	::free(IPCbuf);
	::CloseHandle(hRead);
	
	if(KKP->opaque==NULL&&KKP->kkirq!=NULL&&KKP->kkirq(KKP->PlayerOpaque)==1||rext==0)
	{

		if(rext==0)
			return  AVERROR(109);
		//KKCloseAVFile(KKP->URL);
		return AVERROR(errno);
	}

	IPC_DATA_INFO OutInfo;
	bool RetOk=false;
	strGuid=jsonValue["Guid"].asString();
	GetIPCOpRet(strGuid,RetOk,OutInfo);
	if(RetOk)
	{
		long long ret=OutInfo.DataSize;
		return ret;
	}
	return AVERROR(errno);
}



unsigned int  Kkv_GetCacheTime(void *opaque)//guidlen+guid+msgId+h+FileURL+FileURL+Buf_Size
{

	return 0;
}
//����һ�����ʵ��
KKPlugin __declspec(dllexport) *CreateKKPlugin()
{
	KKPlugin* p= (KKPlugin*)::malloc(sizeof(KKPlugin));
	p->GetCacheTime=Kkv_GetCacheTime;
	p->kkread=Kkv_read_packet;
	p->kkseek=Kkv_seek;
	p->opaque=NULL;
	
	return p;
}

//ɾ��һ�����ʵ��
void __declspec(dllexport) DeleteKKPlugin(KKPlugin* p)
{
	::free(p);
}

//�����ļ���
char __declspec(dllexport)KKDownAVFile(char *strUrl)
{
	  
	  return 0;
}

//��ͣ�����ļ�
void __declspec(dllexport)KKPauseDownAVFile(char *strUrl,bool Pause)
{
	if(G_IPC_Read_Write!=1)
		return ;
	Json::Value jsonValue;
	if(Pause)
	    jsonValue["IPCMSG"]=IPCDownPause;
	else
		jsonValue["IPCMSG"]=IPCDownResume;
	jsonValue["Guid"]="";
	jsonValue["Url"]=strUrl;
    jsonValue["HRW"]=0;
	jsonValue["FirstRead"]=0;
	std::string strGuid=jsonValue.toStyledString();
    int buflen=strGuid.length()+1024;
	
	unsigned char *IPCbuf=(unsigned char*)::malloc(buflen);
	memset(IPCbuf,0,buflen);
	
	
	memcpy(IPCbuf,strGuid.c_str(),strGuid.length());
	KKVWritePipe(IPCbuf,buflen,0);
	::free(IPCbuf);

	return;
}

//typedef bool (*fKKDownAVFileSpeedInfo)(const char *strurl,char *jsonBuf,int len);

///��ȡ�����ٶ���Ϣ
bool __declspec(dllexport) KKDownAVFileSpeedInfo(const char *strurl,char *jsonBuf,int len)
{
	if(G_IPC_Read_Write!=1)
		return false;
	Json::Value jsonValue;
	jsonValue["IPCMSG"]=IPCSpeed;
	jsonValue["Guid"]="";
	jsonValue["Url"]=strurl;
    jsonValue["HRW"]=0;
	jsonValue["FirstRead"]=0;

	std::string strGuid=jsonValue.toStyledString();
    int buflen=strGuid.length()+1024;
	
	unsigned char *IPCbuf=(unsigned char*)::malloc(buflen);
	memset(IPCbuf,0,buflen);
	
	
	memcpy(IPCbuf,strGuid.c_str(),strGuid.length());
	KKVWritePipe(IPCbuf,buflen,0);
	::free(IPCbuf);
	
    return GetSpeedInfo(strurl,jsonBuf,len);
}



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
char* GetIPCUrlParserRet(const char*Url,int *abort_request);
char __declspec(dllexport) *KKUrlParser(const char *strurl,int *abort_request)
{
	char *ret=NULL;
	char* pos=(char*)strstr(strurl,":") ;
	if(pos!=NULL)
	{
		///int buflen=strGuid.length()+1024;
		int Len=strlen(strurl)+1024;
		char *ProName=(char*)::malloc(Len);
        memset(ProName,0,Len);
		int lll=pos-strurl;
		c_left(ProName,(char *)strurl,lll);
		if(strcmp(ProName,"kkv")==0){
		      strcpy(ProName,pos+1);
              ret=GetIPCUrlParserRet(ProName,abort_request);
		}
		::free(ProName);
	}
	return ret; 
}
};


///�õ�URL�����Ľ��
char* GetIPCUrlParserRet(const char*Url,int *abort_request)
{

    std::string strGuid;
	CreatStrGuid(strGuid);

 
	HANDLE hRead=CreateEvent(NULL, TRUE, FALSE, NULL);
	IPC_DATA_INFO xxda;
	xxda.pBuf=0;
	xxda.BufLen=0;
	xxda.DataSize=0;
	xxda.hWait=hRead;
	xxda.CacheTime=0;
	
	AddIPCGuid(strGuid,xxda);

	Json::Value jsonValue;
	jsonValue["IPCMSG"]=IPCURLParser;
	jsonValue["Guid"]=strGuid;
	jsonValue["Url"]=Url;
    jsonValue["HRW"]=(int)hRead;
	jsonValue["FirstRead"]=0;
	

	strGuid=jsonValue.toStyledString();
	int buflen=strGuid.length()+1024;
	unsigned char *IPCbuf=(unsigned char*)::malloc(buflen);
	memset(IPCbuf,0,buflen);
	memcpy(IPCbuf,strGuid.c_str(),strGuid.length());

	int rext=KKVWritePipe(IPCbuf,buflen,0);
	while(rext==1&&*abort_request==0)
	{
		DWORD ret=::WaitForSingleObject( hRead,50);
        if(ret==WAIT_OBJECT_0)
		{
			break;
		}
	}
	
	
	::free(IPCbuf);
	::CloseHandle(hRead);
	
	
	IPC_DATA_INFO OutInfo;
	bool RetOk=false;
	strGuid=jsonValue["Guid"].asString();
	GetIPCOpRet(strGuid,RetOk,OutInfo);

	if(RetOk){
		return (char*)OutInfo.pBuf;
	}
	return NULL;
}