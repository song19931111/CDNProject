#include "Kernel.h"
#define DEF_PORT 12428
#define DEF_MAIN_IP "127.0.0.1"
CKernel::CKernel():m_pNet(NULL),m_pUI(NULL)
{

	//初始化处理函数
	for(int i=0;i<(DEF_PACK_START-DEF_PACK_END-1);i++)
	{
		m_DealFun[i] =&CKernel::DealDefaultTask; 
	}
	//注册函数

	m_DealFun[DEF_SEND_FILE_BLOCK_RS-DEF_PACK_START] = &CKernel::DealSendFileRs;
	m_DealFun[DEF_GET_BLOCK_IP_RS-DEF_PACK_START] = &CKernel::DealGetBlockIpRS;
	m_DealFun[DEF_NONE_NODE_SERVER_IP_RS-DEF_PACK_START] = &CKernel::DealNoneNodeServerIPRS;
}
bool CKernel::Init(IUIToKerel *pUI)
{
	if(false == UnInit())
	{
		return false ;
	}

	//创建网络对象;
	m_pNet  =new ClientNetImpl;
	m_pNet->Init(this);
	m_pUI = pUI;
	
	//初始化网络对象
	
	//获取系统核数
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int nProcess =  sys.dwNumberOfProcessors;
	//初始化处理线程池:
	m_poolDealTask.CreatePool(nProcess,nProcess*2,DEF_MAX_THREADPOOL_TASK_COUNT);

	//初始化连接节点服务器线程池
	m_poolNet.CreatePool(8*nProcess,8*nProcess*2,2*DEF_MAX_THREADPOOL_TASK_COUNT);
	//创建多个线程池由于读写I/O时间过长

	//TODO//连接主服务器
	strcpy_s(m_struMaiNServer.ip,20,DEF_MAIN_IP);
	m_struMaiNServer.port = DEF_PORT;
	
	HANDLE handle  =(HANDLE)_beginthreadex(NULL,0,RecvMainProc,this,0,NULL);
	if(NULL != handle)
	{
		CloseHandle(handle);
		handle = NULL;
	}
	return true ;
}
bool CKernel::UnInit()
{
	//网络反初始化
	if(m_pNet !=NULL)
	{
		m_pNet->UnInit();
	}
	//线程池反初始化::
		m_poolDealTask.DestroyPool();
		m_poolNet.DestroyPool();
	if(NULL != m_pNet)
	{
		delete m_pNet;
		m_pNet  = NULL;
	}
	Sleep(1000);//等待所有的线程退出
	MAP_BLOCK_INFO::iterator ite = m_mp_blockInfo.begin();
	while(ite !=m_mp_blockInfo.end())
	{
		delete (STRU_BLOCK_KEY*)ite->first;
		delete ite->second;
		ite++;
	}
return true ;
}

///线程池的处理
CDealTask::CDealTask(IKernelToDealThreadPool *pKernel):m_pKernel(pKernel)
{
}
CNetTask::CNetTask(IKernelToNetPool *pKernel):m_pKernel(pKernel)
{
	memset(m_szIP,0,16);
}
bool CDealTask::RunTask()
{
	m_pKernel->DealTaskPool(m_sock,m_nTypeServer,m_pBuffer);
	return true;
}
bool CNetTask::RunTask()
{

	m_pKernel->DealNetWorkPool(m_szIP,m_Key,m_QuitHandle);
	return true ;
}
void CKernel::DealNetWorkPool(char *pIp, DWORD dwKey,HANDLE quitHandle) //通知kernel收完了或者错误:
{
	if(dwKey==0||pIp==NULL)
	{
		//设置参数错误
		errcode.GetReSult(enum_para_error);
		return  ;
	}
	//根据文件块key去获取socket 
	SOCKET sock_node ;
	int nIndex =  0;


	m_lock_sock_node.Lock();
	map<DWORD,STRU_SOCK_NODE_INFO *>::iterator ite_sock_node = m_mp_sock_node.find(dwKey);
	if(ite_sock_node != m_mp_sock_node.end())
	{
		sock_node  = ite_sock_node->second->socket;
		nIndex  = ite_sock_node->second->nIndex;		
	}
	m_lock_sock_node.UnLock();	

	if (sock_node ==INVALID_SOCKET)
	{
		return ;
	}
	
	STRU_BLOCK_KEY  * o_key  = (STRU_BLOCK_KEY  *)dwKey;  //转换key为文件块结构信息
	int rCode = m_pNet->OpenDataChanel(sock_node,nIndex,dwKey,quitHandle);
	/*cout <<"关闭socket "<<sock_node<<endl;
	closesocket(sock_node);*/
	if(rCode != enum_datachannel_recv_success)
	{
		cout <<"下载失败"<<endl;
		//下载失败
		m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(rCode);
		//closesocket(m_Commandsock);
		return ;
	}
//	closesocket(m_Commandsock);
	
	//从下载取得文件对象
	m_lock_downloadFile.Lock();
	map<char * ,CMyFile *>::iterator ite = m_mp_downloadingFile.find(o_key->m_pFileKey->szFileName);
	CMyFile *pFile  ;
	if(ite == m_mp_downloadingFile.end())
	{
		cout <<"取得文件对象失败"<<endl;
		errcode.GetReSult(enum_nothing_find_in_downloadling_list);
		m_lock_downloadFile.UnLock();
		return ;
	}
	 pFile=  m_mp_downloadingFile[o_key->m_pFileKey->szFileName];
	  m_lock_downloadFile.UnLock();
	//验证大小
	  if(o_key->m_lBufLen != o_key->m_lWriteLen)
	{
		char szTipbuf[100];
		sprintf_s(szTipbuf,100,"第%d块验证大小失败",o_key->m_nPositon);
		
		m_pUI->NofifyUITip(szTipbuf);
		//cout <<o_key->m_nPositon<<"块验证大小失败"<<endl;
		errcode.GetReSult(enum_data_count_error);
		//投入下载失败的队列
	    m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		return ;
	}	
	//取得数据:
	m_lock_blockInfo.Lock();
	MAP_BLOCK_INFO::iterator ite_block_info= m_mp_blockInfo.find((DWORD)o_key);
	if(ite_block_info == m_mp_blockInfo.end())
	{
		m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(enum_failed_find_block_info);
		m_lock_blockInfo.UnLock();
		return ;
	}
	//取得数据:
	char *pData = ite_block_info->second;
	if(pData ==NULL)
	{
		m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(enum_failed_find_block_info);
		return ;
	}
	//删除map信息:
	m_mp_blockInfo.erase(ite_block_info);
	m_lock_blockInfo.UnLock();
	//md5校验数据
	char *md5String = pFile->md5String((unsigned char* )pData,o_key->m_lBufLen,MD5_STYLE);
	cout <<md5String<<endl;
	cout <<o_key->md5<<endl;
	if(0 != strcmp(md5String,o_key->md5))
	{
			m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(enum_md5_failed);
		return ;
	}
	//写入文件块
	long len  = pFile->WriteFile(o_key->m_nPositon,pData);
	//删除数据
	delete pData;
	pData =NULL;
	if(0 ==len )
	{
		//文件写入失败
		//投入失败队列
		m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(enum_file_write_failed);
		return ;
	}
		char szTipbuf[100];
		sprintf_s(szTipbuf,100,"第%d块下载完毕",o_key->m_nPositon);
		m_pUI->NofifyUITip(szTipbuf);
	//cout <<"写入"<<o_key->m_nPositon<<endl;
	if(false == pFile->SetBlockInfo(o_key->m_nPositon,1))
	{
		//投入失败队列
			m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			o_key->m_bFromNode = true;
			pDownloadQueue->Push(o_key);
		}
		m_lock_dowload_block_queue.UnLock();
		errcode.GetReSult(enum_set_blockinfo_failed);
		return ;
	}
	//下载完成，删除文件块信息:
	//写入配置文
	m_lock_complete.Lock();
	//写入块数+1
	InterlockedIncrement((long *)&pFile->m_nHasWrittenBlock);
	//保存配置文件信息
	m_lock_save_config_file.Lock();
	pFile->SaveBlockInfo();
	m_lock_save_config_file.UnLock();
	if(pFile->m_nHasWrittenBlock == pFile->GetTotalBlock())
	{
		//下载文件完成的操作:
		//设置队列完成标志:
		m_lock_dowload_block_queue.Lock();
		if( m_mp_dowload_block_queue.find(o_key->m_pFileKey->szFileName) != m_mp_dowload_block_queue.end())
		{
			CLockQueue<STRU_BLOCK_KEY *> *pDownloadQueue  =   m_mp_dowload_block_queue[o_key->m_pFileKey->szFileName];
			pDownloadQueue->m_bIsComplete = true; 
		}
		m_lock_dowload_block_queue.UnLock();


		if(m_pUI != NULL)
		{
			//m_pUI->NotifyUIUpdateCompleteFile(ite->first,pFile->GetFileSize());
		}
		m_lock_downloadFile.Lock();
		m_mp_downloadingFile.erase(ite);
		m_lock_downloadFile.UnLock();

		//删除文件对象:
		//pFile->UnInitFIle();
		//delete pFile ;
		//pFile = NULL;
		//删除该文件的下载队列和下载失败队列:
		cout <<"下载完成"<<endl;
	}
	m_lock_complete.UnLock();
	delete o_key;
}
bool  CKernel::RecvDataChannel(DWORD dwKey,SOCKET socket,char szbuf[],long lBuflen)
{
	//return false 表示数据接收完毕，或者错误
//根据 key放入到对应的文件信息块
	//1.转换key
	

	if(0 == dwKey)
	{
		errcode.GetReSult(enum_pkey_null);
		return false ;
	}
	STRU_BLOCK_KEY *pBlock_info  = (STRU_BLOCK_KEY*)(dwKey);
	m_lock_download_from_node_server.Lock();
	//记录速度：
	m_mp_download_from_node_server[pBlock_info->m_pFileKey->szSourceFileName]+=lBuflen;
	m_lock_download_from_node_server.UnLock();

	if (pBlock_info->nBeginTIme = 0)
	{
		pBlock_info->nBeginTIme = time(NULL);
		pBlock_info->nEndTime = pBlock_info->nBeginTIme;
	}
	else
	{
		pBlock_info->nEndTime = time(NULL);
	}
	if(pBlock_info->nEndTime -pBlock_info->nBeginTIme>=60000)
	{
		pBlock_info->nBeginTIme = time(NULL);
		pBlock_info->nEndTime = pBlock_info->nBeginTIme;
		m_lock_blockInfo.Lock();
		STRU_RECV_DATA_LENGTH rs;
		rs.m_nProtocolSize  =rs.m_lMinLength;
		rs.m_nLengthByte = lBuflen;
		rs.m_nProtocolType = DEF_RECV_DATA_LENGTH_RS;
		char szSendBuf[12] = {0};
		int nSend = rs.Seriaze(szSendBuf,12);
		int nRet = send(socket,szSendBuf,nSend,0);
		if(nRet==-1)
		{
			int err  =WSAGetLastError();
		}
	
	}

	
	//2.查找是否在map中已经存在了map<key,char *>
	MAP_BLOCK_INFO::iterator  ite_block=  m_mp_blockInfo.find((DWORD)pBlock_info);
	if(ite_block == m_mp_blockInfo.end())
	{
		//如果不存在，new 创建一块buffer,添加到map结构里
		char*pTemp = new  char[pBlock_info->m_lBufLen];
		memset(pTemp,0,pBlock_info->m_lBufLen);
		m_mp_blockInfo[(DWORD)pBlock_info] = pTemp;
	}
		m_lock_blockInfo.UnLock();
	 // 3.看添加的数据是否超过buffer的限制
	if(pBlock_info->m_lWriteLen+lBuflen>pBlock_info->m_lBufLen)
	{
		errcode.GetReSult(enum_write_data_larger_than_buffer_len);
		return  false;
	}
	 //4往对应的buffer里添加数据, 
	m_lock_blockInfo.Lock();
	char *pTemp  = m_mp_blockInfo[(DWORD)pBlock_info];
	m_lock_blockInfo.UnLock();
	memcpy(pTemp+pBlock_info->m_lWriteLen,szbuf,lBuflen);
	pBlock_info->m_lWriteLen += lBuflen;
	if(pBlock_info->m_lWriteLen == pBlock_info->m_lBufLen)
	{
		cout <<"pBlock_info_m_lWritten"<<pBlock_info->m_lWriteLen<<endl;
		//向服务发送接收完毕！！！！！！！！！
		STRU_CLINET_CLOSE rq;
		rq.m_nProtocolSize =rq.m_lMinLength;
		rq.m_nStatus = 0;
		rq.m_nProtocolType = DEF_CLIENT_CLOSE_RQ;
		char szSendCloseBuf[20]= {0};
		long lSend = rq.Seriaze(szSendCloseBuf,20);
		m_pNet->SendData(socket,szSendCloseBuf,lSend);
		////关闭接收////////////////////////////////////////////////////
			return false ;
	}
	return true ;

}
void CKernel::DealTaskPool(SOCKET socket ,int nType,CStringBuffer *pBuffer) 
{
 while(1)
 {
		char *pTemp = NULL;
		Sleep(1000);
		m_lock_deal_task.Lock();
		//取出大小（只拿不取）
		pTemp = pBuffer->GetFront(sizeof(int),enum_only_read);
		if(pTemp ==NULL)
		{
			//取失败
			m_lock_deal_task.UnLock();
			break ;
		}
		int nSize=  *(int*)pTemp ;
		//根据大小取得对应大小的数据看是否能取出
		pTemp = pBuffer->GetFront(nSize,enum_get);
		//不能取出
		if(pTemp ==NULL)
		{
			m_lock_deal_task.UnLock();
			break ;
		}
		char * pTempData = new char[nSize];
		memcpy(pTempData,pTemp,nSize);
		m_lock_deal_task.UnLock();
		//移动4字节，去掉大小
		pTempData += sizeof(int);
		//得到类型
		int nProtocolType = *(int *)pTempData ;
		pTempData += sizeof(int);
	//根据类型判断是哪种包
		cout<<"-----------------------""线程处理开始处理"<<nProtocolType<<endl;
		//节点服务器的包处理函数
		(this->*m_DealFun[nProtocolType-DEF_PACK_START])(socket,pTempData,nSize-sizeof(int)-sizeof(int));

		//主服务器的包处理函数
		cout<<"-----------------------""线程处理处理完毕 "<<nProtocolType<<endl;
	 }

 cout<<"处理任务完成"<<endl;
}
void CKernel::RecvData(SOCKET socket ,int nType ,CStringBuffer *pBuffer)
{
	//收到节点服务器和主服务器的数据包的操作,投入处理线程当中去
	CDealTask *pTask  =new CDealTask(this);
	pTask->m_nTypeServer = nType;
	pTask->m_sock = socket;
	pTask->m_pBuffer  = pBuffer;
	int nIndex  =0 ; 
	for(int i=0;i<2;i++)  //多投递几个任务去取数据，避免由于收数据粘包造成任务未处理
	{		
		m_poolDealTask.PushTask(pTask);
		Sleep(100);
	}

}
void CKernel::DealDefaultTask(SOCKET ,char szbuf[],long lBuflen)
{
	return ;
}
void CKernel::DealSendFileRs(SOCKET sock,char szbuf[],long lBuflen)
{
	//解包
	STRU_SEND_FILE_BLOCK_RS stru_package;
	if(0==stru_package.UnSeriaze(szbuf,lBuflen))
	{
		return ;
	}
	//往线程池里投递任务:
	CNetTask  *pTask  = new CNetTask(this);
	pTask->m_Key  = stru_package.m_lKey;
	STRU_BLOCK_KEY *pstrublock_info = (STRU_BLOCK_KEY*)pTask->m_Key;
	//拷贝md5数据
	strcpy_s(pstrublock_info->md5,DEF_MD5_LENGTH,stru_package.md5);
	char *pIp = m_pNet->GetIPAddr(stru_package.m_ipServer);
	if(NULL == pIp)
	{
		errcode.GetReSult(enum_ip_null);
		return ;
	}
	strcpy_s(pTask->m_szIP,16,pIp);
	int i = 0 ;
	//多投递几次,直到投递成功
	while(i<5)
	{
		if(true ==  m_poolNet.PushTask(pTask))
		{
				break ;  //		
		}
		i++;
		Sleep(100);
	}
	szbuf-= (sizeof(int)  + sizeof(int));
	delete  [] szbuf ;
}
void CKernel::DealGetBlockIpRS(SOCKET ,char szbuf[],long lBuflen)
{
	static int _i = 0;
	cout <<_i++<<endl;
	if(_i ==8)
	{
		int a = 10;
	}
	STRU_GET_BLOCK_IP_RS o_rs;
	o_rs.UnSeriaze(szbuf,lBuflen);
	//向节点服务器发送请求块数据包
	STRU_BLOCK_KEY *pKey  =(STRU_BLOCK_KEY *)o_rs.m_lKey;
	STRU_SEND_FILE_BLOCK_RQ rq;
	cout <<o_rs.m_szFileName<<endl;
	rq.m_nProtocolType = DEF_SEND_FILE_BLOCK_RQ;
	rq.m_lKey = (DWORD)pKey;
	rq.m_ipUser = m_pNet->GetIP();
	//TODOrq.m_ipServer 
	strcpy_s(rq.m_szFileName,DEF_FILENAME_SIZE,pKey->m_pFileKey->szFileName);
	rq.m_lFileNameLength  =strlen(pKey->m_pFileKey->szFileName) +1;
	rq.m_nProtocolSize = STRU_SEND_FILE_BLOCK_RQ::lMinLength+rq.m_lFileNameLength;
	rq.m_lBlockSize  = o_rs.m_nBlockSize;
	rq.m_nBlockIndex  = pKey->m_nPositon;
	char szSendBuf[DEF_MAX_BUF_SIZE];
	long size = rq.Seriaze(szSendBuf,DEF_MAX_BUF_SIZE);
	
	if(size ==rq.m_nProtocolSize )
	{
		SOCKET socket ; 
		int nIndex  = 0;
		char *ip = m_pNet->GetIPAddr(o_rs.m_lConnectIp);
		//连接节点服务器
		m_pNet->ConnectNodeServer(ip,o_rs.m_shConnectPort,socket,nIndex);
		//开启收数据
		STRU_SOCK_RECV_ONCE *pstru_recv = new STRU_SOCK_RECV_ONCE;
		pstru_recv->socket = socket ;
		pstru_recv->pThis = this;
		_beginthreadex(NULL,0,RecvOnceData,pstru_recv,0,NULL);

		STRU_SOCK_NODE_INFO * pstru_node = new STRU_SOCK_NODE_INFO;
			pstru_node->dwKey = rq.m_lKey;
			pstru_node->socket= socket;
			pstru_node->nIndex = nIndex;
		m_lock_sock_node.Lock();
		m_mp_sock_node[rq.m_lKey] = pstru_node;
		m_lock_sock_node.UnLock();
		
		//发送SendFileRq包:
		m_pNet->SendData(socket,szSendBuf,size);
			
		
		Sleep(100);
				//开启一个收数据线程
		/*m_lock_swich.Lock();
		HANDLE quitHandle = m_mp_thread_switch[(char *)o_rs.m_szFileName];
		m_lock_swich.UnLock();	*/
		//开启一个线程，去Recv，而不是在任务线程中去Recv:
		/*STRU_RECV_NODE_PROC *pstru_recv_node  = new STRU_RECV_NODE_PROC;
		pstru_recv_node->m_quitHandle  =quitHandle;
		pstru_recv_node->m_nIndex = nIndex;
		pstru_recv_node->m_sock = socket ;
		pstru_recv_node->pThis = this ;
		HANDLE handle = (HANDLE )_beginthreadex(NULL,0,RecvNodeProc,pstru_recv_node,0,NULL);
		if( handle != NULL )
		{
			CloseHandle(handle);
		}*/
		//m_pNet->RecvNodeServer(socket,nIndex,quitHandle);
	}
	//回退指针，删除

	szbuf-= (sizeof(int)  + sizeof(int));
	delete  [] szbuf ;
}
 unsigned int CKernel::RecvOnceData(void * pVoid)
 {
	 
	 STRU_SOCK_RECV_ONCE *pstru =  (STRU_SOCK_RECV_ONCE*)pVoid;
	 CKernel *pThis=  pstru->pThis;
	 if( -1 == pThis ->m_pNet->RecvOnce(pstru->socket))
	 {
		cout <<"没连上"<<endl;
	 }

	 delete pVoid;
	 return 1;
 }
void	CKernel::NofityKernelSeriazeKey(long ip, DWORD dwKey,char szbuf[],long lBuflen)
{
	STRU_CONNET_SOCKET_INFO_RQ m_oKey ;
	m_oKey.m_ip = ip;
	m_oKey.m_lKey = dwKey;
	m_oKey.Seriaze(szbuf,lBuflen);
}
//收数据线程
  unsigned int  CKernel::RecvMainProc(void * pThis)
 {
	CKernel* This  =  (CKernel* ) pThis;
	if(enum_connect_failed == This->m_pNet->RecvMainServer(This->m_struMaiNServer.ip,This->m_struMaiNServer.port))
	{
		//TODO通知UI连接失败
	}
	return 1L;
 }
  //////////////////////////////////////////////////////////////////////////////////////////////////
bool CKernel::StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName)
{
	m_pNet->GetIpInfo(m_szIp,m_szDistract,m_szIDCType);
	STRU_FILE_KEY  *pStruFileKey = new STRU_FILE_KEY;
	pStruFileKey->dwFileKey = (DWORD)pStruFileKey;
	strcpy_s(pStruFileKey->szFileName,DEF_FILENAME_SIZE,pFileName);
	strcpy_s(pStruFileKey->szSourceFileName,DEF_FILENAME_SIZE,pFileName);
	strcpy_s(pStruFileKey->szPath,DEF_FILEPATH_SIZE,pPath);
	strcpy_s(pStruFileKey->szUrl,DEF_URL_LEN,pUrl);
	pStruFileKey->pThis  =this ;
	//strcpy_s(pStruFileKey->szFileName,DEF_FILENAME_SIZE,pFileName);
	//STRU_UI_FILE_INFO * stru_ui_info = new STRU_UI_FILE_INFO;
	//stru_ui_info->pThis = this ;
	//stru_ui_info->pUrl = (char *)pUrl;
	//stru_ui_info->pPath =  (char *)pPath;
	//stru_ui_info->pFileName = (char *)pFileName;
	HANDLE handle = (HANDLE)_beginthreadex(NULL,0,StartProc,pStruFileKey,0,NULL);
	if(handle != NULL)
	{
		CloseHandle( handle );
		handle  = NULL;
	}
	return true ;
}
unsigned int CKernel::StartProc(LPVOID lpvoid)
{
	
	STRU_FILE_KEY * stru_file_info =(STRU_FILE_KEY*) lpvoid;
	CKernel *pThis = stru_file_info->pThis;
	pThis->StartDown(stru_file_info);
	//delete stru_ui_info;
	return 1; 
}

	//UI点击暂停的操作
void  CKernel::StartDown(STRU_FILE_KEY *pstruFileKey)
{
	//以上封装为线程函数
	
//创建文件对象:
	CMyFile *pFile = new CMyFile(pstruFileKey->szFileName,pstruFileKey->szPath);
	//pFile->InitFile();
	strcpy_s(pFile->m_url,DEF_URL_LEN,pstruFileKey->szUrl);
	CLibCurl *pLibCurl = new CLibCurl((char *)pstruFileKey->szUrl);
	//将其Libcurl加入到map表;
	m_lock_libcurl.Lock();

	m_mp_libcurl[pstruFileKey->szSourceFileName] = pLibCurl;
	m_lock_libcurl.UnLock();
	__int64 i64Size ;
	//获取大小
	char szi64Size[100] = {0};
	STRU_HEADER_DATA *pData =    pLibCurl->GetInfoFromHttpHeader();
	i64Size = pData->m_i64FileSize;
	_i64toa(i64Size,szi64Size,10);
	//获取最新更新时间
	string strLastModifyd = pData->m_szLastModified;
	//获取文件块:
	
//看磁盘是否存在配置文件，//存在读取配置文件
	if(false  == pFile->ReadBlockInfo())
	{//否则

		//通过url获得文件大小TODO
		//__int64 i64Size = pLibCurl->GetSizeFromHttpHeader();
		//__int64 i64Size = 19550208;
		if(i64Size<=0 )
		{
				//TODO：通知UI获取文件大小失败
			errcode.GetReSult(enum_get_file_size_from_http_failed);
			return ;
		}

		pFile->SetFileSize(i64Size);
		pFile->InitFile();
		pFile->CreateInitFile();
		
		char szTotalBlock[20]=  {0};
	    itoa(pFile->GetTotalBlock(),szTotalBlock,10);
		//拼接并修改文件名
		//拼接文件名:
		string strFileName ="";
		strFileName+=pstruFileKey->szFileName;
		strFileName+='_';
		strFileName+=szi64Size;
		strFileName+='_';
		strFileName +=strLastModifyd;
		strFileName +='_';
		strFileName+=szTotalBlock;
		pFile->SetFileName((char *)strFileName.c_str());
		//创建配置文件
		pFile->SaveBlockInfo();
		
		
		 //通知UI文件的信息:
		m_pUI->NofityUIFileInfo(pstruFileKey->szFileName,i64Size,0);
	}

	
	else 
	{
		//计算已经下载的大小：
		__int64 i64HasWrittenSize= 0;
		if(pFile->GetBlockInfo(pFile->GetTotalBlock()-1)>0)
		{
			 //得到最后一块的大小:
			__int64 i64LastSize = pFile->GetFileSize()-(pFile->GetTotalBlock()-1*pFile->GetBlockSize());
			i64HasWrittenSize  = (pFile->m_nHasWrittenBlock-1)*pFile->GetBlockSize()+i64LastSize;
		}
		else
		{
				i64HasWrittenSize = (pFile->m_nHasWrittenBlock)*pFile->GetBlockSize();
		}
		m_pUI->NofityUIFileInfo(pstruFileKey->szFileName,i64Size,i64HasWrittenSize);
	}
	char *pdata  =NULL;
	//加入到map集合中去 ：//加锁啊！！！！！！！！！！！！！！！！！！！！！！！！！！！
	m_lock_downloadFile.Lock();
	map<char *,CMyFile *>::iterator ite = m_mp_downloadingFile.find(pstruFileKey->szFileName);
	if(ite == m_mp_downloadingFile.end())
	{
		m_mp_downloadingFile[pstruFileKey->szFileName]  = pFile;
	}
	m_lock_downloadFile.UnLock();
	//设置map线程开关为开启
	HANDLE quitHandle = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	//设置为自动（不会恢复），无信号
	//map<char *,HANDLE>::iterator ite_switch  =m_mp_thread_switch.find((char *)pFileName)
	m_lock_swich.Lock();
	m_mp_thread_switch[pstruFileKey->szFileName]  = quitHandle ;
	m_lock_swich.UnLock();
	//建立每个文件的下载队列，和下载失败队列
	CLockQueue<STRU_BLOCK_KEY *> * pDownQueue  = new CLockQueue<STRU_BLOCK_KEY *>;
	//CLockQueue<STRU_BLOCK_KEY *> * pFailedQueue  = new CLockQueue<STRU_BLOCK_KEY *>;
	pDownQueue->InitQueue(pFile->GetTotalBlock());
	//pFailedQueue->InitQueue(pFile->GetTotalBlock());
	m_lock_download_queue.Lock();
	map<string,CLockQueue<STRU_BLOCK_KEY *> *>::iterator ite_download = m_mp_dowload_block_queue.find(pFile->GetFileName());
	if (ite_download != m_mp_dowload_block_queue.end())
	{
		//删除队列
		delete ite_download->second;
		m_mp_dowload_block_queue.erase(ite_download);
	}
	m_mp_dowload_block_queue[pFile->GetFileName()]  = pDownQueue;
	m_lock_download_queue.UnLock();
	//根据文件块数，分配,将没有下载的文件块结构投入到下载中的文件块队列
	for(int i=0;i<pFile->GetTotalBlock();i++)
	{
		if(0 == pFile->GetBlockInfo(i))
		{
			//如果文件块没有被读写过
			STRU_BLOCK_KEY *pKey  = new STRU_BLOCK_KEY;
			strcpy_s(pstruFileKey->szFileName,DEF_FILENAME_SIZE,pFile->GetFileName());
			//strcpy_s(pKey->m_szfileName,DEF_FILENAME_SIZE,pFileName);
			pKey->m_pFileKey = pstruFileKey;
			pKey->m_nPositon  = i ;
			pKey->m_lBufLen =  i!=pFile->GetTotalBlock()-1? pFile->GetBlockSize():pFile->GetFileSize()-pFile->GetBlockSize()*(pFile->GetTotalBlock()-1);
			pKey->m_lWriteLen = 0;
			pDownQueue->Push(pKey);
		}
	}
	//1.开启原链线程 
	STRU_DWONLOAD_FROM_HTTP *pStruFromHttp = new STRU_DWONLOAD_FROM_HTTP;
	pStruFromHttp->pLibcurl = pLibCurl;
	pStruFromHttp->pQueue = pDownQueue;
	pStruFromHttp->pFile = pFile;
	pStruFromHttp->pThis  =this;
	HANDLE handle_download =(HANDLE)_beginthreadex(NULL,0,DownloadFromHttpProc,pStruFromHttp,0,NULL);
	//开启 测试线程
	//_beginthreadex(NULL,0,DownloadFromHttpTestProc,pStruFromHttp,0,NULL);
	if (handle_download != NULL)
	{
		CloseHandle(handle_download);
		handle_download = NULL;
	}
	//2.向主服务器发送请求IP端口数据包:
	int nCount  =0 ;
	while(1)
	{
		if(pDownQueue->m_bIsComplete ==true)
		{
			break; 
		}
		STRU_BLOCK_KEY *pBlock  =NULL;
		if(false == pDownQueue->Pop(&pBlock))
		{
			pBlock = NULL;
			continue ; 
		}
		if(pBlock->m_bFromNode ==true&&pBlock->m_bFromHttp==true)
		{
			pDownQueue->m_bIsComplete   = true ;
			cout <<"由于第"<<pBlock->m_nPositon<<"下载失败导致下载终止"<<endl;
		}
		else if(pBlock->m_bFromNode ==false)
		{
			SendMainServerGetIPPackage(pBlock,pFile->GetFileSize(),pFile->GetBlockSize(),pstruFileKey->szUrl);
		}
		else
		{
			pDownQueue->Push(pBlock);
		}
		Sleep(1000);
	}
	//不管是否下载成功；
	Sleep(2000);
	//删除libcurl下载对象
	m_lock_libcurl.Lock();
	map<string,CLibCurl *>::iterator ite_libcurl = m_mp_libcurl.find(pstruFileKey->szSourceFileName);
	if( ite_libcurl!=m_mp_libcurl.end() )
	{
		delete 	ite_libcurl->second;
		m_mp_libcurl.erase(ite_libcurl);
	}
	m_lock_libcurl.UnLock();
	 //删除下载速度:
	m_lock_download_from_node_server.Lock();
	map<string, int>::iterator ite_download_from_node_server= m_mp_download_from_node_server.find(pFile->GetFileName());
	if( ite_download_from_node_server!=m_mp_download_from_node_server.end() )
	{
		m_mp_download_from_node_server.erase(ite_download_from_node_server);
	}
	m_lock_download_from_node_server.UnLock();

	

	//文件反初始化:
	pFile->UnInitFIle();
	delete pFile;
	pFile = NULL;
	//删除队列
	m_lock_downloadFile.Lock();
	map<string,CLockQueue<STRU_BLOCK_KEY *> *>::iterator ite_down  =m_mp_dowload_block_queue.find(pstruFileKey->szFileName);
	m_mp_dowload_block_queue.erase(ite_down);
	m_lock_downloadFile.UnLock();
	delete pDownQueue;
	pDownQueue = NULL;
	/*delete pDownQueue;
	pDownQueue =NULL; */
}
void  CKernel::SendMainServerGetIPPackage(STRU_BLOCK_KEY *pKey,__int64 i64FileSize,int nBlockSize,char szUrl[])
{
	STRU_GET_BLOCK_IP_RQ rq;
	
	memcpy(rq.m_szUrl,szUrl,strlen(szUrl)+1);
	rq.m_lUrlLen = strlen(rq.m_szUrl)+1;
	rq.ipClient = m_pNet->GetIP();
	rq.ipServer = m_pNet->IPatoi(DEF_MAIN_IP);
	memcpy(rq.m_szDistract,m_szDistract,strlen(m_szDistract)+1);
	memcpy(rq.m_szIDCType,m_szIDCType,strlen(m_szIDCType)+1); 
	rq.m_lDistractLen = strlen(rq.m_szDistract)+1;
	rq.m_lIDCTypeLen = strlen(rq.m_szIDCType)+ 1;
	rq.ipClient =m_pNet->GetIP();
	rq.m_i64FileSize = i64FileSize;
	strcpy_s(rq.m_szFileName,DEF_FILENAME_SIZE,pKey->m_pFileKey->szFileName);
	rq.m_lFileNameLength = strlen(pKey->m_pFileKey->szFileName)+1;
	rq.m_lKey  = (DWORD)pKey;
	rq.m_nBlockIndex =  pKey->m_nPositon;
	rq.m_nBlockSize = pKey->m_lBufLen;
	rq.m_nProtocolSize = STRU_GET_BLOCK_IP_RQ::lMinLength+rq.m_lFileNameLength+rq.m_lDistractLen+rq.m_lIDCTypeLen+rq.m_lUrlLen;
	rq.m_nProtocolType = DEF_GET_BLOCK_IP_RQ;
	char szSend[DEF_MAX_BUF_SIZE] = {0};
	long lSeriazeSize = rq.Seriaze(szSend,DEF_MAX_BUF_SIZE);
	if(0 ==m_pNet->SendMainServer(szSend,lSeriazeSize))
	{
		errcode.GetReSult(enum_send_failed);
	}
}
bool CKernel::PauseDownLoad(const char *pFileName)
{
	return true  ;
}
	//UI点击继续的操作
bool CKernel::ContinueDownLoad(const char *pFileName)
{
	return true  ;
}
	//UI退出的操作
 bool CKernel::QuitDownLoad()
{
	return true  ;
}
  unsigned int  CKernel::DownloadFromHttpProc(void *pParameter)
  {
	  STRU_DWONLOAD_FROM_HTTP * pstru = ( STRU_DWONLOAD_FROM_HTTP * )pParameter;
	  CMyFile *pFile = pstru->pFile;
	  CKernel *pThis = pstru->pThis;
	  CLockQueue<STRU_BLOCK_KEY *> *pQueue =pstru->pQueue;
	  CLibCurl *pLibcurl  = pstru->pLibcurl;
	  STRU_BLOCK_KEY *  pBlockKey = NULL;
	  //char *temp = new char[DEF_ONE_MB];
	  char *pTemp = NULL;
	//  memset(temp,0,DEF_ONE_MB);
	  while(1)
	  {
		  if(pQueue->m_bIsComplete==true)
		  {
			cout <<"原链队列中没有任务了"<<endl;
			break ;
		  }
		 if( false ==pQueue->Pop(&pBlockKey))
		 {
			continue ;
		 }
		
		  else
		  {
			   string str ="取1";
			pThis->m_pUI->NofifyUITip(str);	
			 int nLen =  pLibcurl->DownLoadFile(pBlockKey->m_nPositon*DEF_ONE_MB,pBlockKey->m_nPositon*DEF_ONE_MB+pBlockKey->m_lBufLen-1,&pTemp,DEF_ONE_MB);
			 //设置已经被原链下载过
			 pBlockKey->m_bFromHttp = true;
			 if(nLen==0)
			 {
				 cout<<"第"<<pBlockKey->m_nPositon<<"块"<<"下载失败"<<endl;
				 pQueue->Push(pBlockKey);
			 }
			 else
			 {
			     //验证大小:
				 if(nLen == pBlockKey->m_lBufLen)
				 {
					pFile->WriteFile(pBlockKey->m_nPositon,pTemp);
					pFile->SetBlockInfo(pBlockKey->m_nPositon,2);
					 InterlockedIncrement((long *)&pFile->m_nHasWrittenBlock);
					 pThis->m_lock_save_config_file.Lock();
					 pFile->SaveBlockInfo();
					 pThis->m_lock_save_config_file.UnLock();
				     cout<<"第"<<pBlockKey->m_nPositon<<"块"<<"写入成功"<<endl;
				 }
				 if(pFile->m_nHasWrittenBlock==pFile->GetTotalBlock())
				 {
					 pQueue->m_bIsComplete = true; 
				 }
			 }
		  }
	  
	  }
	return 1L;
  }
 //  unsigned int  CKernel::DownloadFromHttpTestProc(void *pParameter)
 // {
	//STRU_DWONLOAD_FROM_HTTP * pstru = ( STRU_DWONLOAD_FROM_HTTP * )pParameter;
	//CLibCurl *pLibcurl  = pstru->pLibcurl;
	//while(1)
	//{
	//	Sleep(1000);
	//	int n =  pLibcurl->GetDownloadByte();
	//	cout <<"下载速度"<<n<<"字节"<<endl;
	//}
 // }
 //  int t_i = 0;
   void CKernel::DealNoneNodeServerIPRS(SOCKET ,char szbuf[],long lBuflen)
   {
	   STRU_NONE_NODE_SERVER_IP_RS rs;
	   rs.UnSeriaze(szbuf,lBuflen);
	   m_lock_download_queue.Lock();
	   CLockQueue<STRU_BLOCK_KEY *> * pDownloadQueue = NULL;
	   map<string,CLockQueue<STRU_BLOCK_KEY *> *>::iterator ite_find  =m_mp_dowload_block_queue.find(rs.m_szFileName);
	   if(ite_find!=m_mp_dowload_block_queue.end())
	   {
		  pDownloadQueue =  ite_find->second;
	   }
	   m_lock_download_queue.UnLock();
	   if(NULL == pDownloadQueue)
	   {
		return ;
	   }
	   if(rs.m_lFileBlockKey != NULL)
	   {
		   //重新放入队列
		   STRU_BLOCK_KEY*pBlockKey  = (STRU_BLOCK_KEY*) rs.m_lFileBlockKey;
		   pBlockKey->m_bFromNode = true; //已经从节点服务器获取过了，今后就不会用向节点发送下载
			pDownloadQueue->Push(pBlockKey);
			string str; 
			int i  =pBlockKey->m_nPositon;
			char sztemp [10]= {0};
			itoa(i,sztemp,10);
			str = "节点服务器没有第";
			str+=sztemp;
			str+="文件块，尝试从原链下载";
			m_pUI->NofifyUITip(str);	
	   }
  }

map<string,int > CKernel::NotifyKernelReturnHttpDownloadByte()
{
	//清空
	m_mp_download_from_http.clear();
	m_lock_libcurl.Lock();
	map<string,CLibCurl *>::iterator ite_libcurl = m_mp_libcurl.begin();
	while( ite_libcurl != m_mp_libcurl.end() )
	{
		CLibCurl  *pLibcurl = ite_libcurl->second;
		m_mp_download_from_http[ite_libcurl->first]  =pLibcurl->GetDownloadByte();
		ite_libcurl++;
	}
	m_lock_libcurl.UnLock();
	return m_mp_download_from_http;
	
}
map<string,int >   CKernel::NofiyKernelReturnNodeServerByte()
{
	
	m_lock_download_from_node_server.Lock();
	//值拷贝
	map<string,int> mp = m_mp_download_from_node_server;

	map<string,int>::iterator ite_node = m_mp_download_from_node_server.begin();
	while( ite_node != m_mp_download_from_node_server.end() )
	{
		ite_node->second  =0;
		ite_node++;
	}
	m_lock_download_from_node_server.UnLock();

	return mp;

}