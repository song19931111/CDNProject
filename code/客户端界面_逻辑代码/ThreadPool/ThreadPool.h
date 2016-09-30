#pragma once

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include "LockQueue.h"
	class ITask	//�ӿ�
	{
	public:
		virtual bool RunTask()
		{
			return false;
		};
	};

	class CThreadPool
	{
	public:
		CThreadPool();
		~CThreadPool();
		static UINT WINAPI ThreadProc(LPVOID lpParam);
		void ThreadPool();

	public:
		//�����̳߳�
		bool CreatePool(const long lMinThreadCount, const long lMaxThreadCount, 
			const long lMaxTaskCount);
		//�����̳߳�
		void DestroyPool();
		//Ͷ������
		bool PushTask(ITask *pTask);
		//��ȡ��ǰ������
		int GetSize();
	private:
		//�������
		CLockQueue<ITask *> m_oTaskQueue;
		//��С�߳���
		long m_lMinThreadCount;
		//����߳���
		long m_lMaxThreadCount;
		//����֪ͨ�߳����е��ź���
		HANDLE m_hSemaphore;
		//�����߳���
		long m_lRunThreadCount;
		//�Ѿ��������߳���
		long m_lCreateThreadCount;
		//�˳�֪ͨ�¼�
		HANDLE m_hQuitEvent;
	};
#endif // !__THREADPOOL_H
