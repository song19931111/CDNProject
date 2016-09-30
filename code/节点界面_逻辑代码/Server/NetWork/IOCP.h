#pragma once
#ifndef  ____INCLUDE__IOCP__H____
#define  ____INCLUDE__IOCP__H____
#include<list>
#include <process.h>
#include <winsock2.h>
#include <mswsock.h>
#include <map>
#include "LockQueue.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")
#include "StringBuffer.h"
#include "MyLock.h"
using namespace std; 
enum NET_TYPE{enum_unknown,enum_accept,enum_recv,enum_send};
#define DEF_MAX_BUF_LEN (1200)
#define DEF_MAX_RECVINFO (1024)
#define DEF_SERVER_PORT (6667)
#define DEF_MAX_SOCKET_RECV (10) //对同一个socket投递的最大连接数
#define DEF_MAX_SOCKET_ACC (10)//对同一个socket投递的最大accpet数
#define DEF_MAX_CLIENT_CONNECT (100)
//通知类
class INotify{
public:
	virtual bool RecvData(SOCKET socket, CStringBuffer *pBuffer)=0;
	virtual void DealClosePackage(SOCKET socket,char szbuf[],long lBuflen) = 0;
	virtual void DealDataChanelPackage(SOCKET socket,char szbuf[],long lBuflen) = 0;
	virtual void NotifyKernelClientCount(int n) = 0;
};
class IIOCP{
//接口类，提供外界调用的接口处理
public:
	virtual long SendData(SOCKET socket,char szbuf[],long lbuflen)=0;
	virtual bool InitNet(INotify *pNotify)=0;
	virtual bool unInitNet()=0;
	virtual long GetHostIP() = 0;			//返回本机32位的IPV4地址
	virtual bool NotifyIOCPCloseSocket(SOCKET socket) = 0;
};


class CIOCP:public IIOCP
{
public:
	virtual long SendData(SOCKET socket,char szbuf[],long lbuflen);
	virtual bool InitNet(INotify *pNotify=NULL);
	virtual bool unInitNet();
public:
	CIOCP(void);
	~CIOCP(void);
private:
	struct RecvInfo
	{
		//初始化数据：
		RecvInfo():m_type(enum_unknown),m_sock(NULL),m_nBufLength(0)
		{
			memset(&m_ole,0,sizeof(m_ole));
			memset(m_szbuf,0,sizeof(m_szbuf));
			m_ole.hEvent = (HANDLE)WSACreateEvent();
		}
		WSAOVERLAPPED m_ole; //定义文件句柄
		NET_TYPE m_type;			//自定义数据类型
		SOCKET m_sock;
		int m_nBufLength;
		char m_szbuf[DEF_MAX_BUF_LEN];
		int nIndex ;//记录每次的索引，以便在弹出的时候将该索引加入队列
	};
public:
	INotify *m_pNotify;
private:
	bool InnerInitNet();				//内部调用初始化的方法
	bool PostAccept();
	bool PostRecv(RecvInfo *pRev);
	bool  PostRecv(SOCKET socket);//每次创建 一个新的RecvInfo结构
	bool PostSend(RecvInfo *pRev);
	long GetHostIP();			//返回本机32位的IPV4地址
	bool NotifyIOCPCloseSocket(SOCKET socket) ;
	static   unsigned  _stdcall ThreadProc(  void * );		//线程函数
private :
	map<SOCKET,CStringBuffer *> m_mp_stringbuffer; 
	MyLock m_lock_stringbuffer; 
private:
	SOCKET m_listenSock;		//监听socket
	DWORD  m_dwThreadCount;			//创建的线程个数
	HANDLE m_manage;			//IOCP的管理者
	HANDLE m_quit;		//退出信号
	bool  m_bInit; 
	//-------------------------------------
			//内存池加索引池的操作
	CLockQueue<int> m_queue; //投入索引
	RecvInfo * m_pRecvInfo;//内存池数组指针
	//--------------------------------------------
public:
	friend unsigned  _stdcall CIOCP::ThreadProc(  void * lpvoid );
};



#endif 


