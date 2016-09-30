#include "ClientNetImpl.h"

ClientNetImpl::ClientNetImpl()
{
	memset(m_arrSock,0,sizeof(m_arrSock));
	m_bQuit = false ;
}
bool ClientNetImpl::Init(INotify *pNotify)
{
	if(false == UnInit())
	{
		return false ;
	}
	CClientNet::InitNet();
	m_pNotify = pNotify;
	
	m_bQuit = false ;
	//获得本机的IP
	//l_myIp = GetValidIp();
	//Sleep(5000);
	l_myIp  = inet_addr("127.0.0.1");
	m_IndexQueue.InitQueue(DEF_MAX_SOCKET);
	//向索引池投递可用的索引
	for(int i=0;i<100;i++)
	{
		m_IndexQueue.Push(i);
	}
	
	//创建节点服务器sock,和数据通道Socket 
	m_sockNode = socket(AF_INET,SOCK_STREAM,0);
	m_sockServer = socket(AF_INET,SOCK_STREAM,0);
	for(int i=0;i<DEF_MAX_SOCKET;i++)
	{
		sockaddr_in sock_addr;
		sock_addr.sin_family =AF_INET;
		sock_addr.sin_port =htons(DEF_START_PORT+i);
		sock_addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
		
		m_arrSock[i] =socket(AF_INET,SOCK_STREAM,0);
		bind(m_arrSock[i],(sockaddr *)&sock_addr,sizeof(sock_addr));
	}
	//将socket变为异步socket 
	int  flag = 1;
	//ioctlsocket(m_sockNode,FIONBIO ,(u_long*)&flag);
	//ioctlsocket(m_sockServer,FIONBIO ,(u_long*)&flag);
	/*for(int i=0;i<DEF_MAX_SOCKET;i++)
	{
		ioctlsocket(m_arrSock[i],FIONBIO ,(u_long*)&flag);
	}*/
	return true ;
}
bool ClientNetImpl::UnInit()
{
	m_bQuit = true ;  //让所有的收数据通道线程退出
	Sleep(1000); //等待所有的线程走完
	memset(m_arrSock,0,sizeof(m_arrSock));
	if(NULL != m_sockNode)
	{
		//closesocket(m_sockNode);
		m_sockNode = NULL;
	}
	if(NULL != m_sockServer)
	{
		//closesocket(m_sockServer);
		m_sockServer = NULL;
	}
	
	
	//反初始化索引池
	m_IndexQueue.UnInitQueue();
	//卸载库
	
	return true ;
}
int ClientNetImpl::OpenDataChanel(SOCKET socket,int nIndex,DWORD dwKey,HANDLE quitHandle)
{
	//从索引池中取得一个可用的索引
	
	
	cout<<"socket"<<socket<<"进入数据通道"<<endl;
	//连接Server:
	//设置为异步
	int flag = 1;
	//ioctlsocket(socket,FIONBIO ,(u_long*)&flag);
	//发送一个定包;
	char szTemp[8];
	m_pNotify->NofityKernelSeriazeKey(l_myIp,dwKey,szTemp,8);
	int nSendBuffer = 0 ;
	bool m_bQuit = false; 
	int ret=  send(socket,szTemp,8,0);
	if(ret==SOCKET_ERROR)
	{
		return enum_send_failed;
	}

	char buf[DEF_MAX_BUF_SIZE]  ={0};
	int nRecvLen  = 0;
	ioctlsocket(socket,FIONBIO ,(u_long*)&flag);
	while(!m_bQuit)
	{
		//数据通道异步接收
		int nRecv  = recv(socket,buf,DEF_MAX_BUF_SIZE,0);
		if (nRecv >0)
		{
			nRecvLen +=nRecv;
		}
		//如果收到的数据为0,结束循环
		if(0==nRecv)
		{
			break;
		}	
		//如果失败,
		if(SOCKET_ERROR == nRecv)
		{
			//获取错误吗
			int err = WSAGetLastError();
			if(err == WSAETIMEDOUT)
			{
				return enum_timeout;
			}
			else if(err == WSAENOTCONN)
			{
				return enum_connect_failed;
			}
			//异步IO超时，继续等待：
			else if(WSAEWOULDBLOCK == err)
			{
				Sleep(10);
				continue;
				//超时处理:
			}
			else
			{
				//失败
				cout<<"OpenDataChanel"<<"失败"<<socket<<endl;
				cout<<nRecvLen<<endl;
				return enum_unknown_failed ;
			}
		}
		//成功，通知kernel回调	
		//nRecvLen += nRecv;
		if(m_pNotify)
		{
			if(false ==m_pNotify->RecvDataChannel(dwKey,socket,buf,nRecv))
			{
				m_lock_stringbuffer.Lock();
				map<SOCKET,CStringBuffer *>::iterator ite_stringbuffer = m_mp_stringbuffer.find(socket);
				if (ite_stringbuffer != m_mp_stringbuffer.end())
				{
					m_mp_stringbuffer.erase(ite_stringbuffer);
				}		
				m_lock_stringbuffer.UnLock();
				//closesocket(socket);
				//cout<<"socket"<<socket<<"关闭数据通道"<<endl;
			//	cout <<"收到"<<nRecvLen<<endl;
			//	shutdown(socket,SD_BOTH);
				//closesocket(socket);
				//cout <<"关闭socket"<<socket<<endl;
				
				break ;
			}
			memset(buf,0,DEF_MAX_BUF_SIZE);
		}
	}
	if(m_bQuit)
	{
		return enum_client_interupt; //客户端终止
	
	}
	//收完数据，回收socket放入索引池
	//closesocket(m_arrSock[nIndex]);
	//设置为异步
	flag = 0;
	//恢复为同步
	ioctlsocket(socket,FIONBIO ,(u_long*)&flag);
	m_IndexQueue.Push(nIndex);
	return enum_datachannel_recv_success;
}
 int ClientNetImpl::ConnectNodeServer(const char *ip,short port,SOCKET & sock,int & index)
 {
	 //连接NodeServer:
	//从缓冲池里取得一个socket 
	 int nIndex =  0;
	 if(false == m_IndexQueue.Pop(&nIndex))
	 {
		return enum_socket_pool_false ;
	 }

	 index = nIndex;
	 SOCKET sockNode  = m_arrSock[nIndex];
	 //SOCKET sockNode = socket(AF_INET,SOCK_STREAM,0);
	 sock = sockNode;
	 if(false== ConnectServer(sockNode,ip,port))
	 {
		return enum_connect_failed;
	 }
	//cout<<sockNode<<"连上"<<endl;
	return true ;
 }
 int ClientNetImpl::RecvOnce(SOCKET socket)
 {
	 //阻塞 的是
	 char buf[DEF_MAX_BUF_SIZE];
	 int nRecv = recv(socket,buf,DEF_MAX_BUF_SIZE,0);
	 CStringBuffer *pBuffer = NULL;
	 if( nRecv >0)
	 {

		 m_lock_stringbuffer.Lock();
		 map<SOCKET,CStringBuffer *>::iterator ite = m_mp_stringbuffer.find(socket);
		 if ( ite !=m_mp_stringbuffer.end() )
		 {
			 pBuffer = ite->second;
		 
		 }
		 else
		 {
			 pBuffer =new CStringBuffer;
			m_mp_stringbuffer[socket] = pBuffer;
		 }
		 m_lock_stringbuffer.UnLock();
		pBuffer->addData(buf,nRecv);
		m_pNotify->RecvData(socket,enum_node,pBuffer);

	 }
	 else
	 {
		return -1 ;
	 
	 }
	 return 1; 
 }
long ClientNetImpl::RecvNodeServer(SOCKET socket,int nIndex,HANDLE quitHandle)
{
	//nIndex索引池索引
	 //新建缓存:
	 //m_lock_stringbuffer.Lock();
	 int flag =1 ; 
	 ioctlsocket(socket,FIONBIO,(u_long *)&flag );
	 CStringBuffer *pBuffer = new CStringBuffer(2048);
	 //m_mp_stringbuffer[m_sockNode]   = pBuffer;
	//m_lock_stringbuffer.UnLock();
	//接收数据
	char buf[DEF_MAX_BUF_SIZE]  ={0};
	while(1)
	{
		if(WAIT_OBJECT_0 ==WaitForSingleObject(quitHandle,10))
		{
			break ;
		}
		//异步接收
		int nRecv  = recv(socket,buf,DEF_MAX_BUF_SIZE/2,0);
		//如果收到的数据为0,结束循环
		if(0==nRecv)
		{
			return enum_node_server_normal_quit;
		}	
		//如果失败,
		if(SOCKET_ERROR == nRecv)
		{
			//获取错误吗
			int err = WSAGetLastError();
			//异步IO超时，继续等待：
			if(WSAEWOULDBLOCK == err)
			{
				Sleep(10);
				continue;
			}
			else
			{	
				cout<<err<<endl;
				break ;
			}
		}
		pBuffer->addData(buf,nRecv);
		//成功，通知kernel回调	
		if(m_pNotify)
		{
			m_pNotify->RecvData(socket,enum_node,pBuffer);
		}
	}//while
	//等待线程池的任务处理完
	Sleep(2000);
	delete pBuffer;
	pBuffer =NULL;
	  flag =0;
	ioctlsocket(socket,FIONBIO,(u_long *)&flag );
	//closesocket(socket);
	m_IndexQueue.Push(nIndex);

	if(m_bQuit==true)
	{
		return enum_client_quit;
	}

	return enum_node_server_normal_quit;
}
long ClientNetImpl::RecvMainServer(const char *pIp , short shport )
{
	if(false ==ConnectServer(m_sockServer,pIp,shport))
	{
		return enum_connect_failed;
	}
	int  flag = 1;
	ioctlsocket(m_sockServer,FIONBIO ,(u_long*)&flag);
	char buf[DEF_MAX_BUF_SIZE]  ={0};
	while(!m_bQuit)
	{
		
		int nRecv  = recv(m_sockServer,buf,128,0);
		//cout<<nRecv<<endl;
		//如果收到的数据为0,结束循环
		if(0==nRecv)
		{
			cout<<"正常退出"<<endl;
			break;
		}	
		//如果失败,
		if(SOCKET_ERROR == nRecv)
		{
			//获取错误吗
			int err = WSAGetLastError();
			//异步IO超时，继续等待：
			if(WSAEWOULDBLOCK == err)
			{
				Sleep(10);
				continue;
			}
			else
			{
				//异常关闭
				//m_errcode.GetReSult(enum_main_server_error);
				cout<<"异常退出"<<endl;
				return enum_main_server_error ;
			}
		}
		//成功，通知kernel回调	
		m_buffer_main.addData(buf,nRecv);
		if(m_pNotify)
		{
			m_pNotify->RecvData(m_sockServer,enum_main,&m_buffer_main);
		}
	}
	if(m_bQuit==true)
	{
		return enum_client_quit;
	}
	return enum_main_server_normal_quit;
}

long ClientNetImpl::GetValidIp()
{
	return CClientNet::GetValidIp();
}
int ClientNetImpl::SendData(SOCKET socket,char szbuf[],long lBufLen)
{

	return CClientNet::SendData(socket,szbuf,lBufLen);
	
}
char *ClientNetImpl::GetIPAddr(long l_ipaddr)
{
	return CClientNet::GetIPAddr(l_ipaddr);
}
 long  ClientNetImpl::SendMainServer(char szbuf[],long lBufLen)
 {
 
	 return send(m_sockServer,szbuf,lBufLen,0);
 }
 bool ClientNetImpl::GetIpInfo(char *pIp,char *pDistract,char *pIDCType)
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
	//读取:
	fread(szReadBuf,300,1,fp);
	//char *pIndex  = strstr(szReadBuf,"[");
	char *p = NULL;
	p = strtok(szReadBuf,"[");
	p = strtok(NULL,"]");
	//复制IP
	strcpy_s(pIp,DEF_IP_LEN,p);
	p = strtok(NULL,"</center></body></html>");
	char * pString = &p[7];
	// 复制地区:
	pString = strtok(pString," ");
	strcpy_s(pDistract,DEF_DISTRACT_LEN,pString);
	p  = strtok(NULL,"");
	//复制运营商:
	strcpy_s(pIDCType,DEF_IDC_TYPE_LEN,p);
	return true;
 }
