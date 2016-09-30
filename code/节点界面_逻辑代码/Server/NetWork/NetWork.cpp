#include "NetWork.h"
#define DEF_IP_LEN (20)
#define DEF_DISTRACT_LEN (60)
#define DEF_IDC_TYPE_LEN (15)
#define DEF_SERVER_DATACHANEL_IP "220.250.14.105"
#define DEF_SERVER_DATACHANEL_PORT (12345)
CClientNet::CClientNet()
 {
 }
CClientNet::~CClientNet()
{
	 
} 
bool CClientNet::InitNet(CKerenlToClientNet * pKernel)
{
	UnInitNet();
	m_pKernel =  pKernel;
	//����ʼ��
	
	//��ʼ��pNotify
	//�ڲ���ʼ��
	if(false==InnerInitNet())
	{
		return false;
	}
	m_sockClient = socket(AF_INET,SOCK_STREAM,0);
	m_indexQueue.InitQueue(DEF_MAX_SOCKET);
	for(int i=0;i<DEF_MAX_SOCKET;i++)
	{
		m_arrSocket[i] = socket(AF_INET,SOCK_STREAM,0);
	}
	//��ʼ��������
	for(int i=0;i<DEF_MAX_SOCKET;i++)
	{
		m_indexQueue.Push(i);
	}
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
	m_pKernel = NULL;
	m_indexQueue.UnInitQueue();
	return true ;
}
bool CClientNet::ConnectServer(SOCKET socket ,const char * pIp,short port)
{
	 //ת��IPΪlong
	long ip = inet_addr(pIp);
	//��������ṹ��
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
long CClientNet::GetValidIp()//�õ�������ʵ��IP��ַ
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
	//�ļ�ָ��ƫ�Ƶ�ͷ 
	fseek(fp,0,SEEK_SET);
	//��ȡ:
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
int  CClientNet::RecvProcFun(SOCKET socket)
{
	char buf[DEF_MAX_BUF_SIZE]  = {0};
	while (1)
	{
		int nRet = recv(socket,buf,DEF_MAX_BUF_SIZE,0);
		if( nRet == 0)
		{
			//���Ͷ������Ĺر�
			return enum_main_server_normal_quit; 
		}
		if (SOCKET_ERROR == nRet)
		{
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK)
			{
				return enum_main_server_error;
			}
		}
		else
		{
			m_Buffer.addData(buf,nRet);
			m_pKernel->RecvData(socket,&m_Buffer);
		}	
	}
}

bool CClientNet::GetIpInfo(char *pIp,char *pDistract,char *pIDCType)
{
	int nRet = URLDownloadToFile(0, "http://1212.ip138.com/ic.asp", "./IP.ini", 0, NULL);
	if (nRet !=0)
	{
		return false; 
	}
	FILE *fp;
	fopen_s(&fp,"./IP.ini", "r");
	if (NULL == fp)
	{
		return false;
	}
	char szReadBuf[300];
	fseek(fp,0,SEEK_SET);
	//��ȡ:
	fread(szReadBuf,300,1,fp);
	//char *pIndex  = strstr(szReadBuf,"[");
	char *p = NULL;
	p = strtok(szReadBuf,"[");
	p = strtok(NULL,"]");
	//����IP
	strcpy_s(pIp,DEF_IP_LEN,p);
	p = strtok(NULL,"</center></body></html>");
	char * pString = &p[7];
	// ���Ƶ���:
	pString = strtok(pString," ");
	strcpy_s(pDistract,DEF_DISTRACT_LEN,pString);
	p  = strtok(NULL,"");
	//������Ӫ��:
	strcpy_s(pIDCType,DEF_IDC_TYPE_LEN,p);
	return true;
} 
int  CClientNet::OpenDataChannel(long lFileKey,HANDLE quitHandle)
 {
	//�������ػ��socket
	 int nIndex = 0;
	 if(false == m_indexQueue.Pop(&nIndex))
	 {
		return 0;
	 }
	 SOCKET socket  =m_arrSocket[nIndex];
	 Sleep(1000);
	 if(false ==ConnectServer(socket,DEF_SERVER_DATACHANEL_IP,DEF_SERVER_DATACHANEL_PORT))
	 {
		 return enum_connect_failed;
	 }
	 char szFixPackBuf[4];
	 *(long *)szFixPackBuf = lFileKey;
	 //���Ͷ���
	 SendData(socket,szFixPackBuf,4);
	 //����Ϊ�첽
	 int flag = 1;
	 ioctlsocket(socket,FIONBIO,(u_long*)flag);
	 char buf[8*DEF_MAX_BUF_SIZE] ={0};
	 int errcode = 0;
	int nTotalRecv = 0;
	 while(1)
	 {
		//recv������
		int nRecv =recv(socket,buf,8*DEF_MAX_BUF_SIZE,0);
		if(nRecv>0)
		{
			nTotalRecv +=nRecv;
			//cout <<"�յ�"<<nTotalRecv<<endl;
		}
		
		//֪ͨkernelȥ������
		if (nRecv == 0)
		{
			break; 
		}
		if(nRecv ==SOCKET_ERROR)
		{
			int err =WSAGetLastError();
			if(err !=WSAEWOULDBLOCK)
			{
				errcode =err; 
				break; 
			}
			continue ;
		}
		if( false ==m_pKernel->RecvDataChannel(lFileKey,socket,buf,nRecv))
		{
			break; 
		}
	 }

	 flag = 0;
	 ioctlsocket(socket,FIONBIO,(u_long*)flag);
	 m_indexQueue.Push(nIndex);
	 //��������
	 if(errcode !=0)
	 {
		 return errcode;
	 }
	 return enum_datachannel_recv_success;
 }