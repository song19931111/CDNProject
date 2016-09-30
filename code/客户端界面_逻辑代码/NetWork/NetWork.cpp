#include "NetWork.h"
CClientNet::CClientNet()
 {
 }
CClientNet::~CClientNet()
{
	 
} 
bool CClientNet::InitNet()
{
	//反初始化
	UnInitNet();
	//初始化pNotify
	//内部初始化
	if(false==InnerInitNet())
	{
		return false;
	}
	//初始化SOCKET的索引池
	return true ;
}
bool CClientNet::InnerInitNet()
{
	WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
	{
		return  false;    
    }
	return true ;
}
bool CClientNet::UnInitNet()
{ 
	return true ;
}
bool CClientNet::ConnectServer(SOCKET socket ,const char * pIp,short port)
{
	 //转换IP为long
	long ip = inet_addr(pIp);
	//创建网络结构体
	sockaddr_in addr;
	addr.sin_addr.S_un.S_addr = ip;
	addr.sin_family =AF_INET;
	addr.sin_port  =htons(port);
	//connect 
		if(0 ==connect(socket,(sockaddr*)&addr,sizeof(addr)))
		{
			return true; 
		}
		int err = WSAGetLastError();
			return false; 
}
long CClientNet::SendData(SOCKET socket,char szbuf[],long lBufLen)
{
	return send(socket,szbuf,lBufLen,0);
}
long CClientNet::GetValidIp()//得到本机真实的IP地址
{

	remove("./IP.ini");
	char szReadBuf[DEF_MAX_READ_INI_SIZE]={0};
	char szIP[16]={0};
	int nRet = URLDownloadToFile(0, "http://1212.ip138.com/ic.asp", "./IP.ini", 0, NULL);
	if(S_OK!=nRet)
	{
		return 0;
	}
	FILE *fp;
	fopen_s(&fp,"./IP.ini", "r");
	if(NULL==fp)
	{
		fclose(fp);
	}
	//文件指针偏移到头 
	fseek(fp,0,SEEK_SET);
	//读取:
	fread(szReadBuf,DEF_MAX_READ_INI_SIZE,1,fp);
	char *pIndex  = strstr(szReadBuf,"[");
	int i=0;
	while(*pIndex != NULL)
	{
		if(*(pIndex+1)==']')
		{
			break;
		}
		szIP[i++]  =*++pIndex;

	}
	return inet_addr(szIP);
	 
}

char *CClientNet::GetIPAddr(long l_ipaddr)
{
	in_addr addr;
	addr.S_un.S_addr  = l_ipaddr;
	return inet_ntoa(addr);
	
}