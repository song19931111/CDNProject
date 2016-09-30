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
	//��ñ�����IP
	//l_myIp = GetValidIp();
	//Sleep(5000);
	l_myIp  = inet_addr("127.0.0.1");
	m_IndexQueue.InitQueue(DEF_MAX_SOCKET);
	//��������Ͷ�ݿ��õ�����
	for(int i=0;i<100;i++)
	{
		m_IndexQueue.Push(i);
	}
	
	//�����ڵ������sock,������ͨ��Socket 
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
	//��socket��Ϊ�첽socket 
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
	m_bQuit = true ;  //�����е�������ͨ���߳��˳�
	Sleep(1000); //�ȴ����е��߳�����
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
	
	
	//����ʼ��������
	m_IndexQueue.UnInitQueue();
	//ж�ؿ�
	
	return true ;
}
int ClientNetImpl::OpenDataChanel(SOCKET socket,int nIndex,DWORD dwKey,HANDLE quitHandle)
{
	//����������ȡ��һ�����õ�����
	
	
	cout<<"socket"<<socket<<"��������ͨ��"<<endl;
	//����Server:
	//����Ϊ�첽
	int flag = 1;
	//ioctlsocket(socket,FIONBIO ,(u_long*)&flag);
	//����һ������;
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
		//����ͨ���첽����
		int nRecv  = recv(socket,buf,DEF_MAX_BUF_SIZE,0);
		if (nRecv >0)
		{
			nRecvLen +=nRecv;
		}
		//����յ�������Ϊ0,����ѭ��
		if(0==nRecv)
		{
			break;
		}	
		//���ʧ��,
		if(SOCKET_ERROR == nRecv)
		{
			//��ȡ������
			int err = WSAGetLastError();
			if(err == WSAETIMEDOUT)
			{
				return enum_timeout;
			}
			else if(err == WSAENOTCONN)
			{
				return enum_connect_failed;
			}
			//�첽IO��ʱ�������ȴ���
			else if(WSAEWOULDBLOCK == err)
			{
				Sleep(10);
				continue;
				//��ʱ����:
			}
			else
			{
				//ʧ��
				cout<<"OpenDataChanel"<<"ʧ��"<<socket<<endl;
				cout<<nRecvLen<<endl;
				return enum_unknown_failed ;
			}
		}
		//�ɹ���֪ͨkernel�ص�	
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
				//cout<<"socket"<<socket<<"�ر�����ͨ��"<<endl;
			//	cout <<"�յ�"<<nRecvLen<<endl;
			//	shutdown(socket,SD_BOTH);
				//closesocket(socket);
				//cout <<"�ر�socket"<<socket<<endl;
				
				break ;
			}
			memset(buf,0,DEF_MAX_BUF_SIZE);
		}
	}
	if(m_bQuit)
	{
		return enum_client_interupt; //�ͻ�����ֹ
	
	}
	//�������ݣ�����socket����������
	//closesocket(m_arrSock[nIndex]);
	//����Ϊ�첽
	flag = 0;
	//�ָ�Ϊͬ��
	ioctlsocket(socket,FIONBIO ,(u_long*)&flag);
	m_IndexQueue.Push(nIndex);
	return enum_datachannel_recv_success;
}
 int ClientNetImpl::ConnectNodeServer(const char *ip,short port,SOCKET & sock,int & index)
 {
	 //����NodeServer:
	//�ӻ������ȡ��һ��socket 
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
	//cout<<sockNode<<"����"<<endl;
	return true ;
 }
 int ClientNetImpl::RecvOnce(SOCKET socket)
 {
	 //���� ����
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
	//nIndex����������
	 //�½�����:
	 //m_lock_stringbuffer.Lock();
	 int flag =1 ; 
	 ioctlsocket(socket,FIONBIO,(u_long *)&flag );
	 CStringBuffer *pBuffer = new CStringBuffer(2048);
	 //m_mp_stringbuffer[m_sockNode]   = pBuffer;
	//m_lock_stringbuffer.UnLock();
	//��������
	char buf[DEF_MAX_BUF_SIZE]  ={0};
	while(1)
	{
		if(WAIT_OBJECT_0 ==WaitForSingleObject(quitHandle,10))
		{
			break ;
		}
		//�첽����
		int nRecv  = recv(socket,buf,DEF_MAX_BUF_SIZE/2,0);
		//����յ�������Ϊ0,����ѭ��
		if(0==nRecv)
		{
			return enum_node_server_normal_quit;
		}	
		//���ʧ��,
		if(SOCKET_ERROR == nRecv)
		{
			//��ȡ������
			int err = WSAGetLastError();
			//�첽IO��ʱ�������ȴ���
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
		//�ɹ���֪ͨkernel�ص�	
		if(m_pNotify)
		{
			m_pNotify->RecvData(socket,enum_node,pBuffer);
		}
	}//while
	//�ȴ��̳߳ص���������
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
		//����յ�������Ϊ0,����ѭ��
		if(0==nRecv)
		{
			cout<<"�����˳�"<<endl;
			break;
		}	
		//���ʧ��,
		if(SOCKET_ERROR == nRecv)
		{
			//��ȡ������
			int err = WSAGetLastError();
			//�첽IO��ʱ�������ȴ���
			if(WSAEWOULDBLOCK == err)
			{
				Sleep(10);
				continue;
			}
			else
			{
				//�쳣�ر�
				//m_errcode.GetReSult(enum_main_server_error);
				cout<<"�쳣�˳�"<<endl;
				return enum_main_server_error ;
			}
		}
		//�ɹ���֪ͨkernel�ص�	
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
