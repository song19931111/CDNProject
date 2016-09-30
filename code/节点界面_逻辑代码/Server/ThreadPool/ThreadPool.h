#pragma once

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include "LockQueue.h"
	class ITask	//接口
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
		//创建线程池
		bool CreatePool(const long lMinThreadCount, const long lMaxThreadCount, 
			const long lMaxTaskCount);
		//销毁线程池
		void DestroyPool();
		//投递任务
		bool PushTask(ITask *pTask);
		//获取当前任务数
		int GetSize();
	private:
		//任务队列
		CLockQueue<ITask *> m_oTaskQueue;
		//最小线程数
		long m_lMinThreadCount;
		//最大线程数
		long m_lMaxThreadCount;
		//用来通知线程运行的信号量
		HANDLE m_hSemaphore;
		//运行线程数
		long m_lRunThreadCount;
		//已经创建的线程数
		long m_lCreateThreadCount;
		//退出通知事件
		HANDLE m_hQuitEvent;
	};
#endif // !__THREADPOOL_H
