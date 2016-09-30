#pragma once
#ifndef ____INCLUDE__CLIENTNETWORK__H____
#define ____INCLUDE__CLIENTNETWORK__H____
#ifndef DEF_IP_LEN
#define DEF_IP_LEN (20)
#endif
#ifndef DEF_DISTRACT_LEN
#define DEF_DISTRACT_LEN (60)
#endif
#ifndef DEF_IDC_TYPE_LEN
#define DEF_IDC_TYPE_LEN (15)
#endif
#include "NetWork.h"
#include <map>
#include <iostream>
#define DEF_START_PORT (8000)
using namespace std; 
class INotify;
class IClientNet{
public :
	virtual int SendData(SOCKET,char szbuf[],long lBufLen)=0;
	virtual long GetValidIp()=0;//�õ�������ʵ��IP��ַ
	virtual bool Init(INotify *)=0;
	virtual bool UnInit()=0;
	virtual int ConnectNodeServer(const char *ip,short port,SOCKET & sock,int & nIndex) = 0;
	virtual int OpenDataChanel(SOCKET socket,int nIndex,DWORD dwKey,HANDLE quitHandle ) = 0;// ��������ȥ������
	virtual long RecvNodeServer(SOCKET socket,int nIndex,HANDLE quitHandle)= 0;
	virtual long RecvMainServer(const char *ip,short port)=0;
	virtual long GetIP() = 0;
	virtual unsigned long IPatoi(char *pIp) = 0;
	 virtual long SendMainServer(char szbuf[],long lBufLen) = 0;
	virtual char *GetIPAddr(long l_ipaddr) = 0;
	virtual int RecvOnce(SOCKET socket)  = 0;
	virtual  bool GetIpInfo(char *pIp,char *pDistract,char *pIDCType) = 0;
};
class INotify{
public:
	virtual void RecvData(SOCKET,int nType ,CStringBuffer *pBuffer)=0; //�������ͣ�Buffer
	virtual bool RecvDataChannel(DWORD dwKey,SOCKET socket,char szbuf[],long lBuflen)=0;//�ص���kernel�յ�����ͨ��������
	virtual void NofityKernelSeriazeKey(long ip, DWORD dwKey,char szbuf[],long)=0;
	//֪ͨKernel���Ǹ�Socket 
};
class ClientNetImpl :public CClientNet,public IClientNet{
public :
	ClientNetImpl();
	~ClientNetImpl();
	 long  SendMainServer(char szbuf[],long lBufLen);
	 int OpenDataChanel(SOCKET socket,int nIndex,DWORD dwKey,HANDLE quitHandle);// ��������ȥ������
	 long RecvNodeServer(SOCKET socket,int nIndex,HANDLE quitHandle);
	 int ConnectNodeServer(const char *ip,short port,SOCKET & sock,int & nIndex);
	 long RecvMainServer(const char *pIp , short shport);
	 char *GetIPAddr(long l_ipaddr);
	 bool Init(INotify *pNotify);
	 bool UnInit();
	 long GetValidIp();
	 int RecvOnce(SOCKET socket);
	 int SendData(SOCKET,char szbuf[],long lBufLen);
	 bool GetIpInfo(char *pIp,char *pDistract,char *pIDCType);
	  long GetIP()
	  {
		return l_myIp;
	  }
	  unsigned long IPatoi(char *pIp)
	 {
		long ip = 0;

		ip = inet_addr(pIp);
		return ip;
	 }
	 enum SOCKET_TYPE{enum_main=3333,enum_node};
private :
	INotify *m_pNotify;
	SOCKET m_sockNode ; //���ӽڵ�������� socket
	SOCKET m_sockServer;//���ӷ�������Server 
	SOCKET m_arrSock[DEF_MAX_SOCKET];//����SOCKET�أ����ڴ���Data��Socket����
	CLockQueue<int> m_IndexQueue;  //������
	CErrCode m_errcode;
	map<SOCKET ,CStringBuffer *> m_mp_stringbuffer;
	MyLock m_lock_stringbuffer ;
	CStringBuffer m_buffer_main;
	long l_myIp;
	bool m_bQuit;

};
#endif //____INCLUDE__CLIENTNETWORK__H____