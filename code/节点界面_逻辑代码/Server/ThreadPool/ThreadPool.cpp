#include <windows.h>
#include <process.h>
#include "ThreadPool.h"


CThreadPool::CThreadPool() : 
	m_lMinThreadCount(0), m_lMaxThreadCount(0),
	m_lRunThreadCount(0), m_lCreateThreadCount(0),
	m_hSemaphore(NULL), m_hQuitEvent(NULL)
{
	//
}

CThreadPool::~CThreadPool()
{
	DestroyPool();
}

//创建线程池
bool CThreadPool::CreatePool(const long lMinThreadCount, const long lMaxThreadCount, 
				const long lMaxTaskCount)
{
	//检查参数
	if (lMinThreadCount > lMaxThreadCount || lMaxTaskCount <= 0)
	{
		return false;
	}
	//防止重复初始化
	DestroyPool();
	//创建退出事件对象
	m_hQuitEvent = ::CreateEvent(NULL,	//安全属性(NULL不能被子进程继承)
						true,			//手动重置(true手动重置为无信号ResetEvent)
						false,			//初始状态(无信号)
						NULL);			//对象名称
	if (NULL == m_hQuitEvent)
	{
		return false;
	}
	//创建信号量
	m_hSemaphore = ::CreateSemaphore(NULL,	//安全属性
						lMinThreadCount,	//初始信号数量
						lMaxThreadCount,	//最多可以释放多少信号量
						NULL);				//名字
	if (NULL == m_hSemaphore)
	{
		return false;
	}
	//初始化任务队列
	if (false == m_oTaskQueue.InitQueue(lMaxTaskCount))
	{
		return false;
	}
	//创建线程
	for (long i=0; i<lMinThreadCount; ++i)
	{
		//增加创建线程数量
		::InterlockedIncrement(&m_lCreateThreadCount);
		HANDLE hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, 
			ThreadProc, (LPVOID)this, 0, NULL));
		if (NULL == hThread)
		{
			//减少创建线程数量
			::InterlockedDecrement(&m_lCreateThreadCount);
			return false;
		}
		CloseHandle(hThread);
	}
	//保存数据
	m_lMinThreadCount = lMinThreadCount;
	m_lMaxThreadCount = lMaxThreadCount;
	return true;
}
//销毁线程池
void CThreadPool::DestroyPool()
{
	//设置事件为有信号 (通知线程退出)
	if (m_hQuitEvent)
	{
		::SetEvent(m_hQuitEvent);
	}
	//通知所有挂起的线程退出
	//随着线程退出m_lCreateThreadCount会改变,所以要一个变量来存
	int iCount = m_lCreateThreadCount;
	for (int i=0; i<iCount; ++i)
	{
		//也可以一次释放多个, 这里让它渐进退出
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	//等待线程退出 (关系的是创建线程的数量)
	while (m_lCreateThreadCount)
	{
		::Sleep(10);
	}
	//关闭事件和信号
	if (m_hQuitEvent)
	{
		::CloseHandle(m_hQuitEvent);
		m_hQuitEvent = NULL;
	}
	if (m_hSemaphore)
	{
		::CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}
	//销毁任务队列
	m_oTaskQueue.UnInitQueue();
	//清空数据
	m_lMinThreadCount = 0;
	m_lMaxThreadCount = 0;
}
//投递任务
bool CThreadPool::PushTask(ITask *pTask)
{
	//检查参数
	if (NULL == pTask)
	{
		 return false;
	}
	//投递到任务队列中
	if (false == m_oTaskQueue.Push(pTask))
	{
		return false;
	}
	//释放信号量 （如果如果运行线程数 < 创建线程数）
	if (m_lRunThreadCount < m_lCreateThreadCount)
	{
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	//否则如果运行线程数 >= 创建线程数 那么线程不足
	else if (m_lRunThreadCount >= m_lCreateThreadCount)
	{
		//如果线程数 大于最大线程数, 没有办法了...
		if (m_lCreateThreadCount < m_lMaxThreadCount)
		{
			//增加创建线程数量
			::InterlockedIncrement(&m_lCreateThreadCount);
			//创建新的线程
			HANDLE hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, 
				ThreadProc, (LPVOID)this, 0, NULL));
			if (NULL == hThread)
			{
				//减少创建线程数量
				::InterlockedDecrement(&m_lCreateThreadCount);
				return false;
			}
			CloseHandle(hThread);
		}
		//释放信号量, 让新的线程开始工作
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	return true;
}
//获取当前任务数
 int CThreadPool::GetSize()
{
	return m_oTaskQueue.GetSize();
}

UINT WINAPI CThreadPool::ThreadProc(LPVOID lpParam)
{
	//解析线程参数
	CThreadPool *pThis = (CThreadPool *)lpParam;
	//调用普通成员函数 (pThis的成员函数)
	pThis->ThreadPool();

	return 1L;
}

void CThreadPool::ThreadPool()
{
	//循环处理任务(如果等待退出通知超时, 说明不需要退出, 继续工作)
	while (WAIT_TIMEOUT == ::WaitForSingleObject(m_hQuitEvent, 0))
	{
		//等待工作信号(这里要考虑一个问题, 线程都在这等信号,还有机会判断事件退出吗?)
		::WaitForSingleObject(m_hSemaphore, INFINITE);
		//切换线程身份为工作线程
		::InterlockedIncrement(&m_lRunThreadCount);
		//循环处理任务
		ITask *pTask = NULL;
		m_oTaskQueue.Pop(&pTask);
		if (NULL!=pTask)
		{	
			pTask->RunTask();
		}
		//切换线程为非工作线程身份
		::InterlockedDecrement(&m_lRunThreadCount);
	}
	//减少创建线程数量
	::InterlockedDecrement(&m_lCreateThreadCount);
}