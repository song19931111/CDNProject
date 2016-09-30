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
		//给完成端口发送0,NULL,NULL的请求告知其退出线程
		PostQueuedCompletionStatus(m_manage,0,NULL,NULL);
	}
	while (0	!=	m_dwThreadCount)
	{
		Sleep(10);
	}
	//等待线程结束
	if(NULL!=m_manage)
	{
		CloseHandle(m_manage);
		m_manage =NULL;
	}
	m_queue.UnInitQueue();
	if(NULL != m_pRecvInfo)
	{
		//删除内存池
		delete [] m_pRecvInfo;
		m_pRecvInfo=NULL;
	}
	m_bInit = false;
	return true;
}
bool CIOCP::InnerInitNet()				//内部调用初始化的方法,所有的I/O模型共有的方法：
{
	//初始化内存池:
	m_pRecvInfo = new RecvInfo[DEF_MAX_CLIENT_CONNECT*DEF_MAX_SOCKET_RECV];
		//初始化队列:
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
		//创建Socket 
		m_listenSock =WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED); 
		if(m_listenSock==INVALID_SOCKET)
		{
			return false;
		}
		sockaddr_in  socketaddr;
		socketaddr.sin_family = AF_INET;
		in_addr ipaddress;
		ipaddress.S_un.S_addr=GetHostIP();
		socketaddr.sin_port =htons(DEF_SERVER_PORT);//hton大端转小端
		socketaddr.sin_addr =ipaddress;
		if(SOCKET_ERROR==bind(m_listenSock,(sockaddr *)&socketaddr,sizeof(sockaddr_in)))
		{
			return false;
		}
		if(SOCKET_ERROR ==listen(m_listenSock,10))
		{
			return false;
		}
		//创建管理者
	m_manage=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(NULL==::CreateIoCompletionPort((HANDLE)m_listenSock,m_manage,m_listenSock,0))
	{
		return false; 
	}

	PostAccept();
	//投递接收请求
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
	//创建线程等待接收
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
		//成功accpet	,将新创建的 socket交由完成端口管理
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
	//注：第三个参数是wsbuf的数量
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
	if(false ==m_queue.Pop(&nIndex))//从索引池中弹出一个有用的索引
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
	//注：第三个参数是wsbuf的数量
	if(SOCKET_ERROR ==WSARecv(pRecv->m_sock,&wsbuf,1,&dwNumOfByteRecv,&dwflag,&pRecv->m_ole,NULL))
	{
		if( WSA_IO_PENDING!=WSAGetLastError())
		{
			bFlag = false ;
		
		}
	}
	if(bFlag)
	{
		//为每个新投递的创建一个StringBuffer 

		return true;
	}
	return false ;
}
bool CIOCP::PostSend(RecvInfo *pRev)
{
	return true ;
}
long CIOCP::GetHostIP()			//返回本机32位的IPV4地址 
{
	long lVaidIp = inet_addr("127.0.0.1");
	//////////////////////////测试，暂时注释////////////////////////////////////////////////
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
	//////////////////////////测试，暂时注释////////////////////////////////////////////////
	return lVaidIp;
}
 unsigned  _stdcall CIOCP::ThreadProc(  void * lpvoid )
 {
	 CIOCP *pThis = (CIOCP*)lpvoid;
	 DWORD dwNumberOfByte=0;		//接收到的字节数
	 LPOVERLAPPED pOverlapped=NULL;
	 RecvInfo * pInfo = NULL;//将pInfo的地址穿进去，给pInfo赋值成overLapped的地址
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
				//对端异常关闭
				RecvInfo *pInfo = (RecvInfo*)pOverlapped;
				pThis->m_queue.Push(pInfo->nIndex);
				Sleep(3000);//等待处理任务完成
				pThis->m_lock_stringbuffer.Lock();	
				map<SOCKET,CStringBuffer *>::iterator ite_find = pThis->m_mp_stringbuffer.find(pInfo->m_sock);
				if ( ite_find != pThis->m_mp_stringbuffer.end() )
				{
					delete ite_find->second;
					pThis->m_pNotify->NotifyKernelClientCount(pThis->m_mp_stringbuffer.size());
					pThis->m_mp_stringbuffer.erase(ite_find);				
				}
				pThis->m_lock_stringbuffer.UnLock();
				cout <<"服务端非0关闭"<<pInfo->m_sock<<endl;
				closesocket(pInfo->m_sock);
				continue;
			}
		}		
		
		
		//收到(NULL,NULL,NULL)完成通知，退出线程
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
				//投递一个Recv 请求
				//对新客户端提交接收数据请求
				
				//为每个客户端socket建立缓冲区:
				CStringBuffer *pBuffer = new CStringBuffer;
				pThis->m_lock_stringbuffer.Lock();
				pThis->m_mp_stringbuffer[pInfo->m_sock] = pBuffer;
				pThis->m_pNotify->NotifyKernelClientCount(pThis->m_mp_stringbuffer.size());
				pThis->m_lock_stringbuffer.UnLock();
				//for (int i=0;i<DEF_MAX_SOCKET_RECV;i++)
				//{
				//	//为了保证每个连接可以高效的收发数据，为每个新客户端多投递10个请求
				//	pThis->PostRecv(pInfo->m_sock);
				//}
				//再次投递一个accpet请求

				//！！！！！！！！！！！！最后再提交收数据请求，否则会有问题,导致map未添加就发生enum_recv了！！！！！！！！！！！！！！！！！！！！！
				pThis->PostRecv(pInfo->m_sock);
				pThis->PostAccept();
			}
			break;
		case enum_recv:
			{
				//给这个socket投递一个接收缓冲区
				pThis->PostRecv(pInfo->m_sock);
				if(dwNumberOfByte == 0)
				{
					cout <<"服务端关闭"<<pInfo->m_sock<<endl;
					closesocket(pInfo->m_sock);
					//正常退出
					pThis->m_queue.Push(pInfo->nIndex);
					//为每个客户端移除StringBuffer 
					Sleep(3000);//等待处理任务完成
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
//////////////////////////////////////////////定包处理////////////////////////////////////////////////////////////////////
			
				if(8 == dwNumberOfByte)
				{
						//8字节的定包处理:
					if(pThis->m_pNotify)
					{

						pThis->m_pNotify->DealDataChanelPackage(pInfo->m_sock,pInfo->m_szbuf,8);
					}
					pThis->m_queue.Push(pInfo->nIndex);
					continue ;
				}

////////////////////////////////////////// ///定包处理///////////////////////////////////////////////////////////////////////
				pThis->m_lock_stringbuffer.Lock();
				CStringBuffer *pBuffer =pThis->m_mp_stringbuffer[pInfo->m_sock];
				pThis->m_lock_stringbuffer.UnLock();
				pBuffer->addData(pInfo->m_szbuf,dwNumberOfByte);
				if(pThis->m_pNotify)
				{
						//调用继承类，接收传入的字符串，解耦合
					pThis->m_pNotify->RecvData(pInfo->m_sock,pBuffer);
				}
				pThis->m_queue.Push(pInfo->nIndex);
			}
			break;
		}
	}
	//线程数减1
	InterlockedDecrement(&pThis->m_dwThreadCount);
	return 1L;
 }

 bool CIOCP::NotifyIOCPCloseSocket(SOCKET socket)
 {
	 //去索引池取得一个索引
	 int nIndex = 0;
	 if(false == m_queue.Pop(&nIndex))
	 {
		return false; 
	 }
	 //得到一个投递结构
	 RecvInfo *pInfo  =&m_pRecvInfo[nIndex];

	 //填充投递结构
	 pInfo->m_sock = socket;
	 pInfo->nIndex = nIndex;
	 //向完成队列投递通知
	 pInfo->m_type = enum_recv;
	 if( false == PostQueuedCompletionStatus((HANDLE)m_manage,0,(ULONG_PTR)socket,&pInfo->m_ole))
	 {
		 int err =GetLastError();
		 return false ;
	 }
	 return true; 

 
 }