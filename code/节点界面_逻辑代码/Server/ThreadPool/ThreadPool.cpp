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

//�����̳߳�
bool CThreadPool::CreatePool(const long lMinThreadCount, const long lMaxThreadCount, 
				const long lMaxTaskCount)
{
	//������
	if (lMinThreadCount > lMaxThreadCount || lMaxTaskCount <= 0)
	{
		return false;
	}
	//��ֹ�ظ���ʼ��
	DestroyPool();
	//�����˳��¼�����
	m_hQuitEvent = ::CreateEvent(NULL,	//��ȫ����(NULL���ܱ��ӽ��̼̳�)
						true,			//�ֶ�����(true�ֶ�����Ϊ���ź�ResetEvent)
						false,			//��ʼ״̬(���ź�)
						NULL);			//��������
	if (NULL == m_hQuitEvent)
	{
		return false;
	}
	//�����ź���
	m_hSemaphore = ::CreateSemaphore(NULL,	//��ȫ����
						lMinThreadCount,	//��ʼ�ź�����
						lMaxThreadCount,	//�������ͷŶ����ź���
						NULL);				//����
	if (NULL == m_hSemaphore)
	{
		return false;
	}
	//��ʼ���������
	if (false == m_oTaskQueue.InitQueue(lMaxTaskCount))
	{
		return false;
	}
	//�����߳�
	for (long i=0; i<lMinThreadCount; ++i)
	{
		//���Ӵ����߳�����
		::InterlockedIncrement(&m_lCreateThreadCount);
		HANDLE hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, 
			ThreadProc, (LPVOID)this, 0, NULL));
		if (NULL == hThread)
		{
			//���ٴ����߳�����
			::InterlockedDecrement(&m_lCreateThreadCount);
			return false;
		}
		CloseHandle(hThread);
	}
	//��������
	m_lMinThreadCount = lMinThreadCount;
	m_lMaxThreadCount = lMaxThreadCount;
	return true;
}
//�����̳߳�
void CThreadPool::DestroyPool()
{
	//�����¼�Ϊ���ź� (֪ͨ�߳��˳�)
	if (m_hQuitEvent)
	{
		::SetEvent(m_hQuitEvent);
	}
	//֪ͨ���й�����߳��˳�
	//�����߳��˳�m_lCreateThreadCount��ı�,����Ҫһ����������
	int iCount = m_lCreateThreadCount;
	for (int i=0; i<iCount; ++i)
	{
		//Ҳ����һ���ͷŶ��, �������������˳�
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	//�ȴ��߳��˳� (��ϵ���Ǵ����̵߳�����)
	while (m_lCreateThreadCount)
	{
		::Sleep(10);
	}
	//�ر��¼����ź�
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
	//�����������
	m_oTaskQueue.UnInitQueue();
	//�������
	m_lMinThreadCount = 0;
	m_lMaxThreadCount = 0;
}
//Ͷ������
bool CThreadPool::PushTask(ITask *pTask)
{
	//������
	if (NULL == pTask)
	{
		 return false;
	}
	//Ͷ�ݵ����������
	if (false == m_oTaskQueue.Push(pTask))
	{
		return false;
	}
	//�ͷ��ź��� �������������߳��� < �����߳�����
	if (m_lRunThreadCount < m_lCreateThreadCount)
	{
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	//������������߳��� >= �����߳��� ��ô�̲߳���
	else if (m_lRunThreadCount >= m_lCreateThreadCount)
	{
		//����߳��� ��������߳���, û�а취��...
		if (m_lCreateThreadCount < m_lMaxThreadCount)
		{
			//���Ӵ����߳�����
			::InterlockedIncrement(&m_lCreateThreadCount);
			//�����µ��߳�
			HANDLE hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, 
				ThreadProc, (LPVOID)this, 0, NULL));
			if (NULL == hThread)
			{
				//���ٴ����߳�����
				::InterlockedDecrement(&m_lCreateThreadCount);
				return false;
			}
			CloseHandle(hThread);
		}
		//�ͷ��ź���, ���µ��߳̿�ʼ����
		::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	return true;
}
//��ȡ��ǰ������
 int CThreadPool::GetSize()
{
	return m_oTaskQueue.GetSize();
}

UINT WINAPI CThreadPool::ThreadProc(LPVOID lpParam)
{
	//�����̲߳���
	CThreadPool *pThis = (CThreadPool *)lpParam;
	//������ͨ��Ա���� (pThis�ĳ�Ա����)
	pThis->ThreadPool();

	return 1L;
}

void CThreadPool::ThreadPool()
{
	//ѭ����������(����ȴ��˳�֪ͨ��ʱ, ˵������Ҫ�˳�, ��������)
	while (WAIT_TIMEOUT == ::WaitForSingleObject(m_hQuitEvent, 0))
	{
		//�ȴ������ź�(����Ҫ����һ������, �̶߳�������ź�,���л����ж��¼��˳���?)
		::WaitForSingleObject(m_hSemaphore, INFINITE);
		//�л��߳����Ϊ�����߳�
		::InterlockedIncrement(&m_lRunThreadCount);
		//ѭ����������
		ITask *pTask = NULL;
		m_oTaskQueue.Pop(&pTask);
		if (NULL!=pTask)
		{	
			pTask->RunTask();
		}
		//�л��߳�Ϊ�ǹ����߳����
		::InterlockedDecrement(&m_lRunThreadCount);
	}
	//���ٴ����߳�����
	::InterlockedDecrement(&m_lCreateThreadCount);
}