#pragma once
#include "MyLock.h"
template<typename T>
class CLockQueue
{
public :
	bool m_bIsComplete ; //是否完成了对队列的使用
//作一个标志:
public:
	bool InitQueue(int);
	bool UnInitQueue();
	bool Push(T data);
	bool Pop(T* data);
	int GetSize();		//获取当前队列的大小
	CLockQueue(void);
	~CLockQueue(void);
private:
	int m_nHeadIndex;//头指针
	int m_nEndIndex ;//尾指针
	int m_nQueueLength;//队列的长度
	int m_nSize;
	T* m_pQueue;//队列的头指针
	bool m_bInitFlag;
	MyLock m_lock;
};
template<typename T>
bool CLockQueue<T>::InitQueue(int nQueueLength)
{
	UnInitQueue();
	if(!m_bInitFlag)//如果没有初始化
	{
		m_bIsComplete = false;
		m_nQueueLength  =nQueueLength;
		m_bInitFlag = true;//只能初始化一次
		m_pQueue = new T[m_nQueueLength+1];
		m_nHeadIndex = m_nEndIndex = 0;//头尾指针指向0
		return true;
	}
	return false;
}
template<typename T>
bool  CLockQueue<T>::UnInitQueue()
{
	if(!m_bInitFlag)
	{
		return false; 
	}
	m_bIsComplete = true ;
	m_bInitFlag = false; //让标志设置为空
	Sleep(100);// 回收资源的时候，由于其他的线程可能还在对队列的操作，所以,先睡100ms,等待线程由于看到false 标记而结束操作
	if (m_pQueue!=NULL)
	{
		delete [] m_pQueue;
		m_pQueue=NULL;
		return true;
	}
	return false;
}
template<typename T>
bool  CLockQueue<T>::Push(T data)
{
	if(!m_bInitFlag)
	{
		return false;
	}
	if(m_nHeadIndex==(m_nEndIndex+1)%(m_nQueueLength+1))
	{
		//队列满
		return false; 
	}
	m_lock.Lock();
	m_pQueue[m_nEndIndex] = data;
	m_nEndIndex =(m_nEndIndex+1)%(m_nQueueLength+1);
	m_lock.UnLock();
	return true;
}
template<typename T>
int   CLockQueue<T>::GetSize()
{
	int nSize =  (m_nEndIndex-m_nHeadIndex+m_nQueueLength+1)%(m_nQueueLength+1);
	return nSize;
}

template<typename T>
bool  CLockQueue<T>::Pop(T* data)
{
	if (!m_bInitFlag)
	{
		return false;
	}
	if (m_nEndIndex==m_nHeadIndex)
	{
		//队空
		return false ;
	}
	m_lock.Lock();
	*data  = m_pQueue[m_nHeadIndex];
	m_nHeadIndex =(m_nHeadIndex+1)%(m_nQueueLength+1);
	m_lock.UnLock();
	return true;
	
}
template<typename T>
CLockQueue<T>::CLockQueue(void):m_nHeadIndex(0),m_nEndIndex(0),m_bInitFlag(false),m_nQueueLength(0),m_pQueue(NULL),m_nSize(0)
{
	m_bIsComplete = false;
}
template<typename T>
 CLockQueue<T>::~CLockQueue(void)
{
	
}
