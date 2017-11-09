#include "AndPlayerStateNotifyMgr.h"



 CAndPlayerStateNotifyMgr::CAndPlayerStateNotifyMgr()
 {
 m_nThOver=1;
 m_nAddr=0;
 }
CAndPlayerStateNotifyMgr::~CAndPlayerStateNotifyMgr()
{

}

void CAndPlayerStateNotifyMgr::Start()
{
	m_Lock.Lock();
	m_nThOver=0;
    m_nAddr= pthread_create(&m_thTid_task, NULL, (void* (*)(void*))handleState_thread, (LPVOID)this);
	m_Lock.Unlock();
}

void CAndPlayerStateNotifyMgr::Stop()
{
	m_nAddr=0;
	
	while(m_nThOver==0)
	{
	     usleep(20000);
	}
	m_Lock.Lock();
	m_PlayerNotifyMap.clear();
	m_Lock.Unlock();
}
void CAndPlayerStateNotifyMgr::PutAState(int player,int state)
{
	m_Lock.Lock();
	m_playerstateQue.push(std::pair<int,int>(player,state));
	m_Lock.Unlock();
}
void CAndPlayerStateNotifyMgr::SetPlayerNotify(int obj,jobject notify)
{
	m_Lock.Lock();
	std::map<int,jobject>::iterator It=m_PlayerNotifyMap.find(obj);
	if(It!=m_PlayerNotifyMap.end()){
	    It->second=notify;
	}else{
		m_PlayerNotifyMap.insert(std::pair<int,jobject>(obj,notify));
	}
	m_Lock.Unlock();
}
unsigned CAndPlayerStateNotifyMgr::handleState_thread(LPVOID lpParameter)
{
	
	CAndPlayerStateNotifyMgr *mgr=(CAndPlayerStateNotifyMgr *)lpParameter;
	mgr->handleState();
	mgr->m_nThOver=1;
	return 0;
}

void* kk_jni_attach_env();
int kk_jni_detach_env();
void  CAndPlayerStateNotifyMgr::handleState()
{
    JNIEnv *env =(JNIEnv*)kk_jni_attach_env();
	const char *stInPath = "com/ic70/kkplayer/kkplayer/IKKPlayerErrNotify";
    const jclass IPlayerErrNotify = env->FindClass(stInPath);
    if (IPlayerErrNotify == 0) {
     
    }

//    // find the constructor that takes an int
    const jmethodID OpenMediaStateNotifyId = env->GetMethodID( IPlayerErrNotify, "OpenMediaStateNotify", "(II)V" );

    while(m_nAddr!=0){
		int count=0;
	    m_Lock.Lock();
		count=m_playerstateQue.size();
		if(count>0){
			playerstate ss=m_playerstateQue.front();
			std::map<int,jobject>::iterator It=m_PlayerNotifyMap.find(ss.first);
			if(It!=m_PlayerNotifyMap.end()){
				env->CallVoidMethod(It->second,OpenMediaStateNotifyId,ss.first,ss.second);
			}
			m_playerstateQue.pop();
		}
		m_Lock.Unlock();
		if(count==0){
		    usleep(20000);
		}
	}
	kk_jni_detach_env();
}