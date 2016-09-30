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
#define DEF_MAX_SOCKET_RECV (10) //��ͬһ��socketͶ�ݵ����������
#define DEF_MAX_SOCKET_ACC (10)//��ͬһ��socketͶ�ݵ����accpet��
#define DEF_MAX_CLIENT_CONNECT (100)
//֪ͨ��
class INotify{
public:
	virtual bool RecvData(SOCKET socket, CStringBuffer *pBuffer)=0;
	virtual void DealClosePackage(SOCKET socket,char szbuf[],long lBuflen) = 0;
	virtual void DealDataChanelPackage(SOCKET socket,char szbuf[],long lBuflen) = 0;
	virtual void NotifyKernelClientCount(int n) = 0;
};
class IIOCP{
//�ӿ��࣬�ṩ�����õĽӿڴ���
public:
	virtual long SendData(SOCKET socket,char szbuf[],long lbuflen)=0;
	virtual bool InitNet(INotify *pNotify)=0;
	virtual bool unInitNet()=0;
	virtual long GetHostIP() = 0;			//���ر���32λ��IPV4��ַ
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
		//��ʼ�����ݣ�
		RecvInfo():m_type(enum_unknown),m_sock(NULL),m_nBufLength(0)
		{
			memset(&m_ole,0,sizeof(m_ole));
			memset(m_szbuf,0,sizeof(m_szbuf));
			m_ole.hEvent = (HANDLE)WSACreateEvent();
		}
		WSAOVERLAPPED m_ole; //�����ļ����
		NET_TYPE m_type;			//�Զ�����������
		SOCKET m_sock;
		int m_nBufLength;
		char m_szbuf[DEF_MAX_BUF_LEN];
		int nIndex ;//��¼ÿ�ε��������Ա��ڵ�����ʱ�򽫸������������
	};
public:
	INotify *m_pNotify;
private:
	bool InnerInitNet();				//�ڲ����ó�ʼ���ķ���
	bool PostAccept();
	bool PostRecv(RecvInfo *pRev);
	bool  PostRecv(SOCKET socket);//ÿ�δ��� һ���µ�RecvInfo�ṹ
	bool PostSend(RecvInfo *pRev);
	long GetHostIP();			//���ر���32λ��IPV4��ַ
	bool NotifyIOCPCloseSocket(SOCKET socket) ;
	static   unsigned  _stdcall ThreadProc(  void * );		//�̺߳���
private :
	map<SOCKET,CStringBuffer *> m_mp_stringbuffer; 
	MyLock m_lock_stringbuffer; 
private:
	SOCKET m_listenSock;		//����socket
	DWORD  m_dwThreadCount;			//�������̸߳���
	HANDLE m_manage;			//IOCP�Ĺ�����
	HANDLE m_quit;		//�˳��ź�
	bool  m_bInit; 
	//-------------------------------------
			//�ڴ�ؼ������صĲ���
	CLockQueue<int> m_queue; //Ͷ������
	RecvInfo * m_pRecvInfo;//�ڴ������ָ��
	//--------------------------------------------
public:
	friend unsigned  _stdcall CIOCP::ThreadProc(  void * lpvoid );
};



#endif 


