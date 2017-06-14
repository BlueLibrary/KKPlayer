#include "stdafx.h"
#include "KKMutex.h"
#include "KKLock.h"
#ifndef KKCond_t_H_
#define KKCond_t_H_
//��������ʵ��
class CKKCond_t
{
    public:
	       CKKCond_t(void);
	       ~CKKCond_t(void);
		   //����
           int ResetCond();
		   ///ǿ��
		   int SetCond();
		   void CondSignal();
		   int WaitCond(int ms,CKKLock *pLock);

		   void* operator new(size_t size );
		   void  operator delete(void *ptr);
    private:
#ifdef WIN32_KK
		HANDLE m_hWait;
#else
		pthread_cond_t  m_hWait;
        
#endif
		CKKMutex    m_Mutex;
		volatile	int m_nWaiting;
		volatile	int m_nSignals;
};
#endif
