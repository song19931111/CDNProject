#include "IOCP.h"


CIOCP::CIOCP(void)
{
	m_bInit = false;
	m_pNotify = NULL;
	m_pRecvInfo  =NULL;
}
CIOCP::~CIOCP(void)
{
}
long CIOCP::SendData(SOCKET socket ,char szbuf[],long lbuflen)
{
	return send(socket,szbuf,lbuflen,0);
}
 bool CIOCP:: InitNet(INotify *pNotify)
 {
	 
	 unInitNet();
	 m_pNotify  = pNotify;
	 if(InnerInitNet())
	 {	
			return false ; 
	 }
	  m_bInit = true ;
	 return true;
 }
bool CIOCP::unInitNet()
{
	if(false== m_bInit)
	{
		return false ;
	}
	long nThreadCount = m_dwThreadCount;
	for(int i=0;i<nThreadCount;i++)
	{
		//����ɶ˿ڷ���0,NULL,NULL�������֪���˳��߳�
		PostQueuedCompletionStatus(m_manage,0,NULL,NULL);
	}
	while (0	!=	m_dwThreadCount)
	{
		Sleep(10);
	}
	//�ȴ��߳̽���
	if(NULL!=m_manage)
	{
		CloseHandle(m_manage);
		m_manage =NULL;
	}
	m_queue.UnInitQueue();
	if(NULL != m_pRecvInfo)
	{
		//ɾ���ڴ��
		delete [] m_pRecvInfo;
		m_pRecvInfo=NULL;
	}
	m_bInit = false;
	return true;
}
bool CIOCP::InnerInitNet()				//�ڲ����ó�ʼ���ķ���,���е�I/Oģ�͹��еķ�����
{
	//��ʼ���ڴ��:
	m_pRecvInfo = new RecvInfo[DEF_MAX_CLIENT_CONNECT*DEF_MAX_SOCKET_RECV];
		//��ʼ������:
	m_queue.InitQueue(DEF_MAX_CLIENT_CONNECT*DEF_MAX_SOCKET_RECV);
	for(int i=0;i<DEF_MAX_CLIENT_CONNECT*DEF_MAX_SOCKET_RECV;i++)
	{
		m_queue.Push(i);
	}
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) 
		{
			return false;
		}
		//����Socket 
		m_listenSock =WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED); 
		if(m_listenSock==INVALID_SOCKET)
		{
			return false;
		}
		sockaddr_in  socketaddr;
		socketaddr.sin_family = AF_INET;
		in_addr ipaddress;
		ipaddress.S_un.S_addr=GetHostIP();
		socketaddr.sin_port =htons(DEF_SERVER_PORT);//hton���תС��
		socketaddr.sin_addr =ipaddress;
		if(SOCKET_ERROR==bind(m_listenSock,(sockaddr *)&socketaddr,sizeof(sockaddr_in)))
		{
			return false;
		}
		if(SOCKET_ERROR ==listen(m_listenSock,10))
		{
			return false;
		}
		//����������
	m_manage=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(NULL==::CreateIoCompletionPort((HANDLE)m_listenSock,m_manage,m_listenSock,0))
	{
		return false; 
	}

	PostAccept();
	//Ͷ�ݽ�������
	for(int i=0;i<DEF_MAX_SOCKET_ACC;i++)
	{
		if(!PostAccept())
		{
			return false;
		}
	}
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_dwThreadCount  =si.dwNumberOfProcessors;
	//�����̵߳ȴ�����
	for(int i=0;i<m_dwThreadCount*2;i++)
	{
		HANDLE handle = (HANDLE)_beginthreadex(NULL,0,ThreadProc,this,0,NULL);
		CloseHandle(handle);
	}
	return true;
}
bool  CIOCP::PostAccept()
{
	int nIndex =0;
		if(false ==m_queue.Pop(&nIndex))
		{
			unInitNet();
		}
		//RecvInfo *pInfo = new RecvInfo;
	RecvInfo *pInfo =&m_pRecvInfo[nIndex] ;
	if(pInfo==NULL)
	{
		unInitNet();
	}
	pInfo->nIndex = nIndex;
	pInfo->m_type=enum_accept;
	SOCKET sockClient  = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	pInfo->m_sock =sockClient;
	bool bFlag = true ;
	if(FALSE==AcceptEx(m_listenSock,sockClient,pInfo->m_szbuf,0,sizeof(sockaddr_in)+16,sizeof(sockaddr_in)+16,(LPDWORD)&pInfo->m_nBufLength,&pInfo->m_ole))
	{
		if(ERROR_IO_PENDING!=WSAGetLastError())
		{
			bFlag   =false;
		}
	}
	if(bFlag)
	{
		//�ɹ�accpet	,���´����� socket������ɶ˿ڹ���
		if (NULL==::CreateIoCompletionPort((HANDLE)sockClient,m_manage,NULL,0))
		{
			return false; 
		}
		return true;
	}
	return false;
}
	
bool CIOCP::PostRecv(RecvInfo * pRev)
{
	pRev->m_type = enum_recv;
	WSABUF wsbuf;
	wsbuf.buf =pRev->m_szbuf;
	wsbuf.len  =sizeof(pRev->m_szbuf);
	DWORD dwNumOfByteRecv;
	DWORD dwflag = 0;
	bool bFlag=true; 
	//ע��������������wsbuf������
	if(SOCKET_ERROR ==WSARecv(pRev->m_sock,&wsbuf,1,&dwNumOfByteRecv,&dwflag,&pRev->m_ole,NULL))
	{
		if( WSA_IO_PENDING!=WSAGetLastError())
		{
			bFlag = false ;
		
		}
	}
	if(bFlag)
	{
		return true;
	}
	return false ;
}
bool CIOCP::PostRecv(SOCKET socket)
{
	int nIndex=  0;
	if(false ==m_queue.Pop(&nIndex))//���������е���һ�����õ�����
	{
		return false;
	}
	RecvInfo *pRecv =&m_pRecvInfo[nIndex];
	pRecv->m_type = enum_recv;
	pRecv->m_sock = socket;
	pRecv->nIndex = nIndex;
	WSABUF wsbuf;
	wsbuf.buf =pRecv->m_szbuf;
	wsbuf.len  =sizeof(pRecv->m_szbuf);
	DWORD dwNumOfByteRecv;
	DWORD dwflag = 0;
	bool bFlag=true; 
	//ע��������������wsbuf������
	if(SOCKET_ERROR ==WSARecv(pRecv->m_sock,&wsbuf,1,&dwNumOfByteRecv,&dwflag,&pRecv->m_ole,NULL))
	{
		if( WSA_IO_PENDING!=WSAGetLastError())
		{
			bFlag = false ;
		
		}
	}
	if(bFlag)
	{
		//Ϊÿ����Ͷ�ݵĴ���һ��StringBuffer 

		return true;
	}
	return false ;
}
bool CIOCP::PostSend(RecvInfo *pRev)
{
	return true ;
}
long CIOCP::GetHostIP()			//���ر���32λ��IPV4��ַ 
{
	long lVaidIp = inet_addr("127.0.0.1");
	//////////////////////////���ԣ���ʱע��////////////////////////////////////////////////
	//char szHostName[1024];
	//if(SOCKET_ERROR ==gethostname(szHostName,1024))
	//{
	//	return lVaidIp;
	//}
	//hostent  *pHost = gethostbyname(szHostName);
	//if(NULL==pHost)
	//{
	//	return lVaidIp;
	//}
	//if(4==pHost->h_length)
	//{
	//	long lIp =  *(long*)pHost->h_addr_list[0];
	//	in_addr addr;
	//	addr.S_un.S_addr =lIp;
	//	//cout<<inet_ntoa(addr)<<endl;
	//	return *(long*)pHost->h_addr_list[0];
	//}
	//////////////////////////���ԣ���ʱע��////////////////////////////////////////////////
	return lVaidIp;
}
 unsigned  _stdcall CIOCP::ThreadProc(  void * lpvoid )
 {
	 CIOCP *pThis = (CIOCP*)lpvoid;
	 DWORD dwNumberOfByte=0;		//���յ����ֽ���
	 LPOVERLAPPED pOverlapped=NULL;
	 RecvInfo * pInfo = NULL;//��pInfo�ĵ�ַ����ȥ����pInfo��ֵ��overLapped�ĵ�ַ
	 ULONG_PTR  key =NULL;
	while(1)
	{
		if (FALSE ==GetQueuedCompletionStatus(pThis->m_manage,&dwNumberOfByte,&key,&pOverlapped,10))
		{
			int nError = ::GetLastError();
			if (WAIT_TIMEOUT==nError)
			{
				//cout <<nError<<endl;
				Sleep(100);
				continue;
			}
			else
			{
				if (NULL==pOverlapped)
				{
					continue;
				}
				cout <<nError<<endl;
				//�Զ��쳣�ر�
				RecvInfo *pInfo = (RecvInfo*)pOverlapped;
				pThis->m_queue.Push(pInfo->nIndex);
				Sleep(3000);//�ȴ������������
				pThis->m_lock_stringbuffer.Lock();	
				map<SOCKET,CStringBuffer *>::iterator ite_find = pThis->m_mp_stringbuffer.find(pInfo->m_sock);
				if ( ite_find != pThis->m_mp_stringbuffer.end() )
				{
					delete ite_find->second;
					pThis->m_pNotify->NotifyKernelClientCount(pThis->m_mp_stringbuffer.size());
					pThis->m_mp_stringbuffer.erase(ite_find);				
				}
				pThis->m_lock_stringbuffer.UnLock();
				cout <<"����˷�0�ر�"<<pInfo->m_sock<<endl;
				closesocket(pInfo->m_sock);
				continue;
			}
		}		
		
		
		//�յ�(NULL,NULL,NULL)���֪ͨ���˳��߳�
		if(key==NULL && dwNumberOfByte==0&&pInfo==NULL)
		{
			break;
		}
		RecvInfo *pInfo = (RecvInfo*)pOverlapped;
		pInfo->m_nBufLength = (int)dwNumberOfByte;
		switch(pInfo->m_type)
		{
		case enum_accept:
			{
				//Ͷ��һ��Recv ����
				//���¿ͻ����ύ������������
				
				//Ϊÿ���ͻ���socket����������:
				CStringBuffer *pBuffer = new CStringBuffer;
				pThis->m_lock_stringbuffer.Lock();
				pThis->m_mp_stringbuffer[pInfo->m_sock] = pBuffer;
				pThis->m_pNotify->NotifyKernelClientCount(pThis->m_mp_stringbuffer.size());
				pThis->m_lock_stringbuffer.UnLock();
				//for (int i=0;i<DEF_MAX_SOCKET_RECV;i++)
				//{
				//	//Ϊ�˱�֤ÿ�����ӿ��Ը�Ч���շ����ݣ�Ϊÿ���¿ͻ��˶�Ͷ��10������
				//	pThis->PostRecv(pInfo->m_sock);
				//}
				//�ٴ�Ͷ��һ��accpet����

				//������������������������������ύ���������󣬷����������,����mapδ��Ӿͷ���enum_recv�ˣ�����������������������������������������
				pThis->PostRecv(pInfo->m_sock);
				pThis->PostAccept();
			}
			break;
		case enum_recv:
			{
				//�����socketͶ��һ�����ջ�����
				pThis->PostRecv(pInfo->m_sock);
				if(dwNumberOfByte == 0)
				{
					cout <<"����˹ر�"<<pInfo->m_sock<<endl;
					closesocket(pInfo->m_sock);
					//�����˳�
					pThis->m_queue.Push(pInfo->nIndex);
					//Ϊÿ���ͻ����Ƴ�StringBuffer 
					Sleep(3000);//�ȴ������������
					pThis->m_lock_stringbuffer.Lock();
					map<SOCKET,CStringBuffer *>::iterator ite_find = pThis->m_mp_stringbuffer.find(pInfo->m_sock);
						
					if ( ite_find != pThis->m_mp_stringbuffer.end() )
					{
						delete ite_find->second;
						pThis->m_mp_stringbuffer.erase(ite_find);				
					}
					pThis->m_lock_stringbuffer.UnLock();

					continue;
				}
//////////////////////////////////////////////��������////////////////////////////////////////////////////////////////////
			
				if(8 == dwNumberOfByte)
				{
						//8�ֽڵĶ�������:
					if(pThis->m_pNotify)
					{

						pThis->m_pNotify->DealDataChanelPackage(pInfo->m_sock,pInfo->m_szbuf,8);
					}
					pThis->m_queue.Push(pInfo->nIndex);
					continue ;
				}

////////////////////////////////////////// ///��������///////////////////////////////////////////////////////////////////////
				pThis->m_lock_stringbuffer.Lock();
				CStringBuffer *pBuffer =pThis->m_mp_stringbuffer[pInfo->m_sock];
				pThis->m_lock_stringbuffer.UnLock();
				pBuffer->addData(pInfo->m_szbuf,dwNumberOfByte);
				if(pThis->m_pNotify)
				{
						//���ü̳��࣬���մ�����ַ����������
					pThis->m_pNotify->RecvData(pInfo->m_sock,pBuffer);
				}
				pThis->m_queue.Push(pInfo->nIndex);
			}
			break;
		}
	}
	//�߳�����1
	InterlockedDecrement(&pThis->m_dwThreadCount);
	return 1L;
 }

 bool CIOCP::NotifyIOCPCloseSocket(SOCKET socket)
 {
	 //ȥ������ȡ��һ������
	 int nIndex = 0;
	 if(false == m_queue.Pop(&nIndex))
	 {
		return false; 
	 }
	 //�õ�һ��Ͷ�ݽṹ
	 RecvInfo *pInfo  =&m_pRecvInfo[nIndex];

	 //���Ͷ�ݽṹ
	 pInfo->m_sock = socket;
	 pInfo->nIndex = nIndex;
	 //����ɶ���Ͷ��֪ͨ
	 pInfo->m_type = enum_recv;
	 if( false == PostQueuedCompletionStatus((HANDLE)m_manage,0,(ULONG_PTR)socket,&pInfo->m_ole))
	 {
		 int err =GetLastError();
		 return false ;
	 }
	 return true; 

 
 }