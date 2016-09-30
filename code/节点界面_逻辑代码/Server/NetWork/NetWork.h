#pragma once 
#ifndef ____INCLUDE__NETWORK__H____
#define ____INCLUDE__NETWORK__H____
#include <WinSock2.h>
#include "LockQueue.h"
#include <iostream>
 #include <urlmon.h>
#include "ErrCode.h"
#include "StringBuffer.h"
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Urlmon.lib")
enum SOCKET_TYPE{enum_node=1001,enum_server=1002,enum_data=1003};

///////////////////////////////////
#define DEF_MAX_SOCKET (100)
#define DEF_MAX_READ_INI_SIZE (1024)
#define DEF_MAX_BUF_SIZE (1024)

class CKerenlToClientNet{
public :
	virtual bool RecvData(	SOCKET socket ,CStringBuffer *pBuffer  ) = 0;
	virtual bool RecvDataChannel(long lFileKey ,SOCKET socket,char szbuf[],long lBuflen) = 0;
};
/////////////////////////////////// 

class CClientNet{
public:
	 CClientNet();
	 ~CClientNet();
	 long SendData(SOCKET,char szbuf[],long lBufLen);
	 long GetValidIp();//得到本机真实的IP地址
	 char *GetIPAddr(long l_ipaddr);
	 bool GetIpInfo(char *pIp,char *pDistract,char *pIDCType);
	 int RecvProcFun(SOCKET socket);
	//开启数据通道:
	 int  OpenDataChannel(long lFileKey,HANDLE quitHandle);

	 unsigned long IPatoi(char *pIp)
	 {
		long ip = 0;

		ip = inet_addr(pIp);
		return ip;
	 }
public :
	bool InnerInitNet();
	bool UnInitNet();
	bool InitNet(CKerenlToClientNet * pKernel);

private:	
	SOCKET m_sockClient ;
	CStringBuffer m_Buffer ;
	CKerenlToClientNet *m_pKernel ;
	SOCKET m_arrSocket[DEF_MAX_SOCKET];
	CLockQueue<int>m_indexQueue ;
public :
	  bool ConnectServer(SOCKET socket,const char * pIp,short port) ;
public :
	  SOCKET GetClientSocket()
	  {
		return m_sockClient;
	  }
};


#endif //____INCLUDE__NETWORK__H____