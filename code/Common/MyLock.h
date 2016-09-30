#pragma once
#include <windows.h>
#ifndef ____INCLUDE__MYLOCK__H____
#define ____INCLUDE__MYLOCK__H____
class MyLock{
private:
	CRITICAL_SECTION m_cs;
	//HANDLE  m_handle;
public:
	MyLock()
	{
	//	m_handle = ::CreateMutex(NULL,FALSE,NULL);
		InitializeCriticalSectionAndSpinCount(&m_cs,0);
	}
	~MyLock()
	{
		DeleteCriticalSection(&m_cs);
		//CloseHandle(m_handle);
	}
	bool Lock()
	{
		//WaitForSingleObject(m_handle,INFINITE);
		EnterCriticalSection(&m_cs);
		return true;
	}
	bool UnLock()
	{
		//ReleaseMutex(m_handle);
		LeaveCriticalSection(&m_cs);
		return true;
	}
};
#endif
