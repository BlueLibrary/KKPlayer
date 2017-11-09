#ifndef AndPlayerStateNotifyMgr_H_
#define AndPlayerStateNotifyMgr_H_
#include <map>
#include <queue>
#include "../KKLock.h"
 #include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <jni.h>
typedef std::pair<int,int> playerstate;
class CAndPlayerStateNotifyMgr
{
public:
	   CAndPlayerStateNotifyMgr();
	   ~CAndPlayerStateNotifyMgr();
	   void PutAState(int player,int state);
	   void SetPlayerNotify(int obj,jobject notify);
	   void Start();
	   void Stop();
private:
	 static unsigned  handleState_thread(LPVOID lpParameter);
	 void  handleState();
	 pthread_t m_thTid_task;
     unsigned int m_nAddr;
	 int          m_nThOver;
	 std::map<int,jobject>    m_PlayerNotifyMap;
	 std::queue<playerstate>  m_playerstateQue;
	 CKKLock m_Lock;
};
#endif