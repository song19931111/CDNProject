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

/////////////////////////////////// 

class CClientNet{
public:
	 CClientNet();
	 ~CClientNet();
	 
	 long SendData(SOCKET,char szbuf[],long lBufLen);
	 long GetValidIp();//得到本机真实的IP地址
	 bool InitNet();
	 bool UnInitNet();
	 char *GetIPAddr(long l_ipaddr);
private :
	bool InnerInitNet();
protected :
	  bool ConnectServer(SOCKET socket,const char * pIp,short port) ;
	  
};



#endif //____INCLUDE__NETWORK__H____