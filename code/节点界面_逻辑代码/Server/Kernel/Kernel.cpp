#include "Kernel.h"
#define DEF_SERVER_IP  "220.250.14.105"
#define DEF_SERVER_TEST_PORT (12428)
#include <time.h>

CKernel::CKernel():m_pNet(NULL),m_pThreadPool(NULL)
{
	m_nUpLoadTotalByte = 0;
	m_nDownLoadTotalByte = 0;
	//��ʼ������ָ������
	for(int i=0;i<DEF_PACKAGE_COUNT;i++)
	{
		m_DealFun[i]  =&CKernel::DefaultDealTask;
	}
	//ע�ắ��
	m_DealFun[DEF_SEND_FILE_BLOCK_RQ-DEF_PACK_START] = &CKernel::DealSendFileRq;
	m_DealFun[DEF_SEND_NODE_FILE_RQ-DEF_PACK_START] = &CKernel::DealSendNodeFileRq;
	m_DealFun[DEF_REQUEST_MAIN_SERVER_RS-DEF_PACK_START] = &CKernel::DealRequestSendMainServerRs;
	m_DealFun[DEF_RECV_DATA_LENGTH_RS-DEF_PACK_START] = &CKernel::DealRecvDataLength;
	m_DealFun[DEF_CLIENT_CLOSE_RQ-DEF_PACK_START]  =&CKernel::DealClosePackage;
}
CKernel::~CKernel()
{
	if(m_pNet)
	{
		delete m_pNet;
		m_pNet =NULL;
	}
	if(m_pThreadPool)
	{
		delete m_pThreadPool;
		m_pThreadPool =NULL;
	}
}

bool CKernel::OpenKernel(IUIToKernel *pUI)
{
	// ����ʼ��
	if(!CloseKernel())
	{
		return false ;
	}
	m_pUI =pUI;
	//�����ͻ����������:
	m_pClientNet = new CClientNet;
	m_pClientNet->InitNet(this);
	//�����������
	m_pNet  =new CIOCP;
	//��ʼ���������	
	if(NULL == m_pNet)
	{
		return false; 
	}
	m_pNet->InitNet(this);
	//�����̳߳ض���
	m_pThreadPool = new CThreadPool;
	m_pNetPool = new CThreadPool;

	if( false == m_pClientNet->ConnectServer(m_pClientNet->GetClientSocket(),DEF_SERVER_IP,DEF_SERVER_TEST_PORT))
	{
		cout <<"���ӿͻ���ʧ��"<<endl;
		return false ;
	}
	//��ʼ�������ļ�:
	m_profile.InitProfile("./cache/serverprofile.pro");
	char szIp[DEF_IP_LEN] = {0};
	char szDistract[DEF_DISTRACT_LEN]={0};
	char szIDCType[DEF_IDC_TYPE_LEN] = {0};
	//m_pClientNet->InitNet(this);
	m_pClientNet->GetIpInfo(szIp,szDistract,szIDCType);
	STRU_NODE_SERVER_LOGIN_RQ rq; 	
	rq.m_lipServer = m_pClientNet->IPatoi(DEF_SERVER_IP);
	rq.m_lipNode = m_pClientNet->IPatoi(szIp);
	//strcpy_s(rq.m_szDistract,DEF_DISTRACT_LEN,"������ ������");
	memcpy(rq.m_szDistract,szDistract,strlen(szDistract)+1);
	///strcpy_s(rq.m_szIDCType,DEF_IDC_TYPE_LEN,"����");
	memcpy(rq.m_szIDCType,szIDCType,strlen(szIDCType)+1);
	

	rq.m_lDistractLength = strlen(rq.m_szDistract)+1;
	rq.m_lIDCTypeLength = strlen(rq.m_szIDCType)+ 1;
	rq.m_nProtocolSize = rq.m_lMinLength+ rq.m_lDistractLength+rq.m_lIDCTypeLength ;
	rq.m_nProtocolType = DEF_NODE_SERVER_LOGIN_RQ;
	
	
	char szSendBuf[DEF_MAX_BUF_SIZE]={0};
	long lsize = rq.Seriaze(szSendBuf,DEF_MAX_BUF_SIZE);
	if(lsize >0)
	{
		if(SOCKET_ERROR == m_pClientNet->SendData(m_pClientNet->GetClientSocket(),szSendBuf,lsize))
		{
			cout <<"���������������ʧ��"<<endl;
			return  false;
		}
	}

	//�����������߳�,�����ڵ����������:
	HANDLE handle = (HANDLE)_beginthreadex(NULL,0,RecvMainProc,this,0,NULL);
	if(handle != NULL)
	{
		CloseHandle(handle);
	}

	//��ʼ���������

	//��ʼ���̳߳�
			//�����շ������̺߳ʹ��������ٶȲ�һ�£����Դ������ݵ��̵߳����������У�Ӧ�����շ����ݵ�n������֤������
		  //�շ�����1s,n�벻������Ȼװ���¶���
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD dwNumberProcess = si.dwNumberOfProcessors;
	if( false ==m_pThreadPool->CreatePool(dwNumberProcess,4*dwNumberProcess,DEF_MAX_THREADPOOL_TASK_COUNT))
	{
		return  false ;
	}
	if (false ==m_pNetPool->CreatePool(dwNumberProcess*2,dwNumberProcess*4,DEF_MAX_THREADPOOL_TASK_COUNT))
	{
		return false ;
	}
	
	
	return true;
}

bool CKernel::CloseKernel()
{
	if(m_pNet)
	{
		m_pNet->unInitNet();
		delete m_pNet;
		m_pNet=NULL;
	}
	if(m_pThreadPool)
	{
		m_pThreadPool->DestroyPool();
		delete m_pThreadPool;
		m_pThreadPool = NULL;
	}
	if(m_pUI)
	{
		m_pUI=NULL;
	}
	return true; 
}
unsigned int __stdcall CKernel::RecvMainProc(void *lpvoid )
{
	CKernel *pThis  =(CKernel *)lpvoid ;
	pThis->m_pClientNet->RecvProcFun(pThis->m_pClientNet->GetClientSocket());
	cout <<"�����������ж�"<<endl;
	return 1;
}
 void CKernel::NotifyKernelClientCount(int n)
 {
	m_pUI->NotifyUIUpdataClientCount(n); 
 }
bool CKernel::RecvData(SOCKET socket,CStringBuffer *pBuffer)
{
	CRecvTask *pTask  =new CRecvTask(this);
	pTask->m_sock = socket;
	pTask->m_pBuffer  = pBuffer;
	int nIndex  =0 ; 
	for(int i=0;i<1;i++)  //��Ͷ�ݼ�������ȥȡ���ݣ���������������ճ���������δ����
	{
		if(m_pThreadPool->PushTask(pTask))
		{
				nIndex++;
		}
		if(10 == nIndex)
		{
			break ; 
		}	
	}
	return true; 
}
bool CRecvTask::RunTask()
{
	//�����̳߳�����������֪��Ҫ�������������kernel�ṩIKernelToTask�ӿڣ��ص���Kernel
	Sleep(300);
	return m_pKernel->DealTask(m_sock,m_pBuffer);
}
bool CNetTask::RunTask()
{
	//�����̳߳�����������֪��Ҫ�������������kernel�ṩIKernelToTask�ӿڣ��ص���Kernel
	return m_pKernel->DealNetTask(m_lFileKey,m_quitHandle);
}
void CKernel::DefaultDealTask(SOCKET,char [],long)
{
	//Ĭ�ϵĴ��������ڷ�ֹ�յ����ݰ��޷��������³�����
	return ; 
}
bool CKernel::DealTask(SOCKET socket,CStringBuffer *pBuffer)
{
	while(1)
	{
		if ( socket == NULL||pBuffer == NULL )
		{
			return false; 
		}
		char *pTemp = NULL;
		m_lock_deal_task.Lock();
		//ȡ����С��ֻ�ò�ȡ��
		pTemp = pBuffer->GetFront(sizeof(int),enum_only_read);
		if(pTemp ==NULL)
		{
			//ȡʧ��
			m_lock_deal_task.UnLock();
			return  false;
		}
		int nSize=  *(int*)pTemp ;
	
		//���ݴ�Сȡ�ö�Ӧ��С�����ݿ��Ƿ���ȡ��
		pTemp = pBuffer->GetFront(nSize,enum_get);
		//����ȡ��
		if(pTemp ==NULL)
		{
			m_lock_deal_task.UnLock();
			return false;
		}
		char * pTempData = new char[nSize];
		memcpy(pTempData,pTemp,nSize);
		m_lock_deal_task.UnLock();
		//�ƶ�4�ֽڣ�ȥ����С
		pTempData += sizeof(int);
		//�õ�����
		int nProtocolType = *(int *)pTempData ;
		pTempData += sizeof(int);
	//���������ж������ְ�
	
		//�ڵ�������İ�������
		(this->*m_DealFun[nProtocolType-DEF_PACK_START])(socket,pTempData,nSize-sizeof(int)-sizeof(int));
	}
		return true;
}
bool CKernel::DealNetTask(long m_lKey,HANDLE quitHandle)
{
	if(enum_datachannel_recv_success!= m_pClientNet->OpenDataChannel(m_lKey,quitHandle))
	{
		cout <<"�ļ�����ʧ��"<<endl;
		return false ;
	}
	//����ļ���ÿ��md5:TODO:
	return true; 
}
void CKernel::DealSendFileRq(SOCKET socket,char szbuf[],long lBuflen)
{
	STRU_SEND_FILE_BLOCK_RQ rq;
	if(0 == rq.UnSeriaze(szbuf,lBuflen))
	{
		return ;
	}
	string strFileAllPath ;
	strFileAllPath +="./cache/";
	strFileAllPath+=rq.m_szFileName;
	STRU_EACH_FILE_INFO *pEachFileInfo = new STRU_EACH_FILE_INFO;
	//m_lock_set_send_file.Lock();
	///*if(m_set_send_file.find(*/
	//m_lock_set_send_file.UnLock();
	CMyFile * pFile = NULL;
	m_lock_file_obj.Lock();
	map<string,CMyFile *>::iterator ite_file = m_mp_file_obj.find(rq.m_szFileName);
	if(ite_file != m_mp_file_obj.end())
	{
		pFile = ite_file->second;
	}
	else
	{
		pFile = new CMyFile (strFileAllPath.c_str());
		pFile->InitFile();
		pFile->ReadFileInfo();
		m_mp_file_obj[rq.m_szFileName]  = pFile ;
		pFile->pMd5=new char[pFile->m_nHeadSize];
		//��ȡmd5:
		pFile->ReadFileEx(0,pFile->pMd5,pFile->m_nHeadSize);
	}
	m_lock_file_obj.UnLock();
	
	
	pEachFileInfo->m_sock = socket ;
	pEachFileInfo->m_pFile = pFile ;
	pEachFileInfo->m_nBlockIndex =rq.m_nBlockIndex;
	m_lock_each_send_info.Lock();
	m_mp_each_send_info[rq.m_lKey]  = pEachFileInfo ;
	m_lock_each_send_info.UnLock();
	
	
	/*if (NULL == pFile->ReadMd5(rq.m_nBlockIndex,arrMd5,sizeof(arrMd5)))
	{
		cout<<"δ��ͷ�ļ��ҵ�md5"<<endl;
	}*/
	//pFile->ReadFileEx(rq.m_nBlockIndex*33,arrMd5,33);
	//char *md5 = new char[33];
	//////////////////////////////������ʱȥ��md5��֤//////////////////////////////
	//char *md5= pFile->ReadOneBlockMd5(rq.m_nBlockIndex);
	 //��ȡ���ļ����md5:
	//char *md5 = new char[33];
	//strcpy_s(md5,33,"abcdefghijklmnopqrstuvwxyzabcde");
	//////////////////////////////������ʱȥ��md5��֤//////////////////////////////
	/*if(md5 == NULL)
	{
		
		errcode.GetReSult(enum_read_md5_error);
		return ;
		
	}*/
	//���ͻظ����ݰ�:
	STRU_SEND_FILE_BLOCK_RS rs;
	memcpy(rs.md5,pFile->pMd5+rq.m_nBlockIndex*DEF_MD5_LENGTH,DEF_MD5_LENGTH);
	rs.m_ipServer = m_pNet->GetHostIP();
	rs.m_ipUser = rq.m_ipUser;
	rs.m_lBlockSize = rq.m_lBlockSize;
	rs.m_lFileNameLength = rq.m_lFileNameLength;
	rs.m_lKey = rq.m_lKey;
	rs.m_nBlockIndex = rq.m_nBlockIndex;
	rs.m_nProtocolSize = STRU_SEND_FILE_BLOCK_RS::lMinLength+rs.m_lFileNameLength+DEF_MD5_LENGTH;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	rs.m_shPort = DEF_SERVER_PORT;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	strcpy_s(rs.m_szFileName,DEF_FILENAME_SIZE,rq.m_szFileName);
	char szSendBuffer[DEF_MAX_BUF_LEN] = {0};
	int size = rs.Seriaze(szSendBuffer,DEF_MAX_BUF_LEN);
	m_pNet->SendData(socket,szSendBuffer,size);

}

void CKernel::DealSendNodeFileRq(SOCKET socket,char szbuf[],long lBuflen)
{
//���ڴ������ڵ�������ķ����ļ�����:

//���:
	STRU_SEND_NODE_FILE_RQ rq;
	rq.UnSeriaze(szbuf,lBuflen);
	//�����ļ������ļ�����:
	/*string allPath ="./cache";
	allPath+=rq.m_szFileName;*/
	//python-2.7.11.amd64.msi_19550208_Sat, 05 Dec 2015 210047 GMT
	//python-2.7.11.amd64.msi_19550208_Sat, 05 Dec 2015 21:00:47 GMT"
	CMyFile *pFile = new CMyFile(rq.m_szFileName,"./cache/",rq.m_i64FileSize);
	//�鿴�ļ��Ƿ��Ѿ����ڣ���� ���ڶ�ȡ�����ļ�:
	if(false ==pFile->ReadFileInfo())
	{
		//���򴴽�һ����С��ȵ��ļ�
		pFile->InitFile();
		pFile->CreateInitFile();
		pFile->m_nProfileSize = rq.m_nMd5BlockSize;//�����ļ��Ĵ�С
		if( rq.m_nMd5BlockSize<64*1024)
		{
			pFile->m_nHeadSize = 64*1024;
		}
		else
		{
			if(0 ==rq.m_nMd5BlockSize%(64*1024))
			{
				pFile->m_nHeadSize = rq.m_nMd5BlockSize;
			}
			else
			{
				pFile->m_nHeadSize =( rq.m_nMd5BlockSize/(64*1024) + 1)*64*1024;
			}
		}
		pFile->SaveFileInfo();
	}
	//��ʼ��һЩ�ļ����Ȳ���
	pFile->InitFileMappingInfo();
	pFile->m_pTempCache = new char[DEF_ONE_MB];
	//���ļ��������map
	//m_lock_from_main_file.Lock();
	//map<long,CMyFile  *>::iterator ite_main_file = m_mp_from_main_file.find((long)pFile);
	//if(ite_main_file ==m_mp_from_main_file.end() )
	//{
	//	m_mp_from_main_file[(long)pFile] = pFile;
	//}
	//m_lock_from_main_file.UnLock();
	//����������ͻظ�����
	STRU_SEND_NODE_FILE_RS rs;
	strcpy_s(rs.m_szFileName,DEF_FILENAME_SIZE,rq.m_szFileName);
	rs.m_lFileNameLength = rq.m_lFileNameLength;
	rs.m_nStartByte = pFile->GetWriteFilePos();
	rs.m_nProtocolType =DEF_SEND_NODE_FILE_RS;
	rs.m_nProtocolSize = rs.m_lMinLength+rs.m_lFileNameLength;
	rs.m_lFileKey = (long)pFile;
	char szSendBuf[DEF_MAX_BUF_SIZE];
	long lSend = rs.Seriaze(szSendBuf,DEF_MAX_BUF_SIZE);
	m_pClientNet->SendData(socket,szSendBuf,lSend);
	//���̳߳�Ͷ�뿪������ͨ��������
	CNetTask *pTask = new CNetTask(this);
	pTask->m_lFileKey = (long)pFile;
	//strcpy_s(pTask->m_szFileName,DEF_FILENAME_SIZE,rs.m_szFileName);
	//TODO "�����˳�HANDLE
	pTask->m_quitHandle = NULL;
	for(int i=0;i<5;i++)
	{
		if(m_pNetPool->PushTask(pTask))
		{
			break;
		}
		Sleep(500);
	}
}
void CKernel::DealRequestSendMainServerRs(SOCKET socket,char szbuf[],long lBuflen)
 {
	 //��½�ɹ��� 
	 STRU_REQUEST_MAIN_SERVER_RS rs;
	 rs.UnSeriaze(szbuf,lBuflen);
	// int nSize = m_profile.m_ls_flie_node.size();
	 if (rs.m_nCommand ==DEF_COMMAND_LOGIN_SUCCESS)
	 {
		 //��ȡ����:
		 if(false == m_profile.ReadProfile())
		 {
			 m_pUI->NofifyUILoginSuccess();
			 //û�������ļ�
			return ; 
		 }
		 //int nSize =  m_profile.m_ls_flie_node.size();
		 //���ͱ����Ѿ����ڵĻ���:
		 char szSendBuf[DEF_MAX_BUF_SIZE] = {0};
		 
		 list<STRU_FILE_NODE*>::iterator ite_file =m_profile.m_ls_flie_node.begin();
		 while(ite_file != m_profile.m_ls_flie_node.end())
		 {

				STRU_SEND_FILE_COMPLETE_RQ rq ;
				rq.m_lFileKey = 0;
				rq.m_lFileNameLength = (*ite_file)->m_lFileNameLength;
				rq.m_nProtocolSize = rq.m_lFileNameLength +rq.m_lMinLength;
				rq.m_nProtocolType = DEF_SEND_FILE_COMPLETE_RQ;
				strcpy_s(rq.m_szFileName,DEF_FILENAME_SIZE,(*ite_file)->m_szFileName);
				long lLen = rq.Seriaze(szSendBuf,DEF_MAX_BUF_SIZE);
				m_pClientNet->SendData(socket,szSendBuf,lLen);
				ite_file++;
		 }
		 m_pUI->NofifyUILoginSuccess();
	 }
 }
 bool CKernel::RecvDataChannel(long lKey ,SOCKET socket,char szbuf[],long lBuflen) 
 {
	 m_lock_download_byte.Lock();
	 m_nDownLoadTotalByte +=lBuflen;
	 m_lock_download_byte.UnLock();
	 //��¼���ֽ���:
	 //���ļ���д���ļ�:
	 CMyFile *pFile = (CMyFile *)lKey;
	 if(pFile->GetWriteFilePos()+lBuflen>pFile->GetFileSize())
	 {
		return false ;
	 }
	 
	 //memset(pFile->m_pTempCache,0,DEF_ONE_MB);
	 if(pFile->m_lTempCacheLength +lBuflen>DEF_ONE_MB)
	 {
		 //�ȿ���1Mʣ��Ĳ���
		memcpy(pFile->m_pTempCache+pFile->m_lTempCacheLength,szbuf,DEF_ONE_MB-pFile->m_lTempCacheLength);
		//д��
		pFile->WriteFileLittleData(pFile->m_pTempCache,DEF_ONE_MB);
		int jiLength   = pFile->m_lTempCacheLength;
		pFile->m_lTempCacheLength = 0;

		memset(pFile->m_pTempCache,0,DEF_ONE_MB);
		//��ʣ�µ�д�뻺��
		memcpy(pFile->m_pTempCache,szbuf+(DEF_ONE_MB-jiLength),lBuflen-((DEF_ONE_MB-jiLength)));
		 pFile->m_lTempCacheLength+=(lBuflen-(DEF_ONE_MB-jiLength));
	 }
	 else if(pFile->m_lTempCacheLength +lBuflen ==DEF_ONE_MB)
	 {
		 memcpy(pFile->m_pTempCache+pFile->m_lTempCacheLength,szbuf,lBuflen);
		 //strncpy_s(pFile->m_pTempCache+pFile->m_lTempCacheLength,DEF_ONE_MB,szbuf,lBuflen);
		 pFile->WriteFileLittleData(pFile->m_pTempCache,DEF_ONE_MB);
		 memset(pFile->m_pTempCache,0,DEF_ONE_MB);
		 pFile->m_lTempCacheLength  =0;
	 }
	 else
	 {
		 memcpy(pFile->m_pTempCache+pFile->m_lTempCacheLength,szbuf,lBuflen);
		 //strncpy_s(pFile->m_pTempCache+pFile->m_lTempCacheLength,DEF_ONE_MB,szbuf,lBuflen);
		 pFile->m_lTempCacheLength+=lBuflen;
	 }
	 cout<<"�Ѿ�д��"<<pFile->GetWriteFilePos()<<endl;
	 if( pFile->GetWriteFilePos()+pFile->m_lTempCacheLength == pFile->GetFileSize())
	 {
		 //д�����һ�Σ�
		 pFile->WriteFileLittleData(pFile->m_pTempCache,pFile->m_lTempCacheLength);
		//������ɱ���
		 STRU_SEND_FILE_COMPLETE_RQ rq; 
		 strcpy_s(rq.m_szFileName,DEF_FILENAME_SIZE,pFile->GetFileName());
		 rq.m_lFileKey =lKey;
		 rq.m_nProtocolType  = DEF_SEND_FILE_COMPLETE_RQ;
		 rq.m_lFileNameLength = strlen(rq.m_szFileName)+1;
		 rq.m_nProtocolSize = rq.m_lMinLength+ rq.m_lFileNameLength;
		 char szSendBuf[DEF_MAX_BUF_LEN] = {0};
		 long lLen = rq.Seriaze(szSendBuf,DEF_MAX_BUF_LEN);
		 //ͨ������ͨ�������ļ���ɰ�:
		 m_pClientNet->SendData(m_pClientNet->GetClientSocket(),szSendBuf,lLen);
		/* STRU_SEND_CLOSE_RESULT rq_result;
		 rq_result.m_nResult  = 0;
		 char szbuf[]*/
		 char szClose[1] = {0};
		 int n=  m_pClientNet->SendData(socket,szClose,1);
		 //�ļ�д��
		 m_profile.UpdateProfile(pFile->GetFileName());
		 pFile->UnInitFIle();
		 //���������ļ�:
		
		 delete pFile ;
		 //�ٴ�������������µ��ļ�:
		 STRU_REQUEST_MAIN_SERVER_RQ rq_send_main;
		 rq_send_main.m_nCommand =DEF_COMMAND_REQUEST_FILE;
		 rq_send_main.m_nProtocolSize = rq_send_main.m_lMinLength;
		 rq_send_main.m_nProtocolType =DEF_REQUEST_MAIN_SERVER_RQ;
		 char szSendMainBuf[DEF_MAX_BUF_SIZE];
		 long lSendMain=rq_send_main.Seriaze(szSendMainBuf,DEF_MAX_BUF_SIZE);
		 m_pClientNet->SendData(m_pClientNet->GetClientSocket(),szSendMainBuf,lSendMain);
	 }
	return true ;
 }
void CKernel::DealDataChanelPackage(SOCKET socket,char szbuf[],long lBuflen)
{
	 //8�ֽڶ�������:
	//jiexi����
	
		STRU_CONNET_SOCKET_INFO_RQ rq;
		rq.UnSeriaze(szbuf,lBuflen);
		STRU_SEND_DATA  * pData  = new STRU_SEND_DATA;
		pData->pThis = this ;
		pData->dwKey  =rq.m_lKey;
		pData->socket =  socket;
		HANDLE handle  = ( HANDLE )_beginthreadex(NULL,0,SendProc,pData,0,NULL);
		if(NULL != handle )
		{
			CloseHandle(handle);
			handle = NULL ; 
		}
}
 unsigned int _stdcall CKernel::SendProc(void * pVoid)
 {
	//����һ��Send�߳�:
	 STRU_SEND_DATA  * pData  = (STRU_SEND_DATA*)pVoid;
	 CKernel *pThis = pData->pThis;
	 map<DWORD,STRU_EACH_FILE_INFO*>::iterator ite ;
	 pThis->m_lock_each_send_info.Lock();
	 ite = pThis->m_mp_each_send_info.find(pData->dwKey);
	if(ite == pThis->m_mp_each_send_info.end())
	{
		pThis->m_lock_each_send_info.UnLock();
		return  false ;
	}
	STRU_EACH_FILE_INFO * pstru_each_file_info  = ite->second;
	 pThis->m_lock_each_send_info.UnLock();
	 ///��ʼ�����߳�
	 pThis->SendClientFileBlock(pData->socket,pstru_each_file_info);
	 ///
	 //ɾ�� ����ļ���key�ṹ:
	 pThis->m_lock_each_send_info.Lock();
	 delete pstru_each_file_info;

	 pThis->m_mp_each_send_info.erase(ite);
	 pThis->m_lock_each_send_info.UnLock();
	 return true ;
 }
 static int i=0;
 bool CKernel::SendClientFileBlock(SOCKET socket ,STRU_EACH_FILE_INFO *pEachInfo)
 {
	 cout<<"socket"<<socket<<"��������"<<endl;
	 //����ļ�����:
	 CMyFile *pFile = pEachInfo->m_pFile;
	 //�����ļ������ݿռ�
	 int nBlockSize  = pFile->GetBlockSize();
	 int nReadIndex =pEachInfo->m_nBlockIndex;
	
	 //�ļ�ǰ����ֽ��������ļ�
	 int nReadSize =  0;
	 int nTotalBlock =  pFile->GetTotalBlock();
	 if (nReadIndex == nTotalBlock-1)
	 {
		 nReadSize = pFile->GetFileSize()-pFile->m_nHeadSize-(pFile->GetTotalBlock()-1)*nBlockSize;	 
	 }
	 else
	 {
		 nReadSize=  pFile->GetBlockSize();
	 }
	 char * pReadTemp  = new char[nReadSize];
	 memset(pReadTemp,0,nReadSize);
	 if (++i==19)
	 {
		int a = 10;
	 }
	 int nReadLen  =   pFile->ReadFileEx(pFile->m_nHeadSize+nReadIndex*nBlockSize,pReadTemp,nReadSize);
	 if(0 ==nReadLen)
	 {
		 errcode.GetReSult(enum_read_file_failed);
		 return false; 
	 }
	 //��ȡһ���ļ�������ݣ�����ѭ������
	 int nSendLen = 0;
	 while (true)
	 {
		int ret = send(socket ,pReadTemp,nReadLen,0);
		if( ret > 0)
		{
			/*m_lock_upload_byte.Lock();
			m_nUpLoadTotalByte +=ret ;
			m_lock_upload_byte.UnLock();*/
			nSendLen += ret ;
		}
		if( nSendLen == nReadLen )
		{
			//֪ͨ�����Ƴ�����socket��������socket
			/*m_pNet->NotifyIOCPCloseSocket(socket);
			m_pNet->NotifyIOCPCloseSocket(pEachInfo->m_sock);
			break; 
			closesocket(pEachInfo->m_sock);
			closesocket(socket);*/
			cout <<socket<<"�������"<<endl;
			break;
		}
		if( 0 == ret )
		{
			//�ļ�������:
			break ;
		}
		if( ret == SOCKET_ERROR )
		{
			int err = WSAGetLastError();
			if( err == WSAEWOULDBLOCK )
			{
				continue ;
			}
			
			else
			{	
				errcode.GetReSult(enum_client_interupt);
				break;
			}
		}
	 }
	 //m_pNet->NotifyIOCPCloseSocket(socket);
	//  m_pNet->NotifyIOCPCloseSocket(pEachInfo->m_sock);
	  cout<<"socket"<<socket<<"����"<<nSendLen<<"��������ͨ��"<<endl;
	 delete pReadTemp;
	 pReadTemp  = NULL ;
	return true ;
 }
 void CKernel::DealClosePackage(SOCKET socket,char szbuf[],long lBuflen)
 {
	 CancelIo((HANDLE)socket);
	 //shutdown(socket,SD_BOTH);
	 //Sleep(3000);
	 //closesocket(socket);

 }

 void CKernel::NofityKernelSendFileRequest()
 {
	//�����ļ�����:
		 STRU_REQUEST_MAIN_SERVER_RQ rq;
		
		 rq.m_nCommand =DEF_COMMAND_REQUEST_FILE;
		 rq.m_nProtocolSize = rq.m_lMinLength;
		 rq.m_nProtocolType =DEF_REQUEST_MAIN_SERVER_RQ;
		 char szSendBuf[DEF_MAX_BUF_SIZE];
		 long lSend =rq.Seriaze(szSendBuf,DEF_MAX_BUF_SIZE);
		 m_pClientNet->SendData(m_pClientNet->GetClientSocket(),szSendBuf,lSend);
 }
  int CKernel::NotifyKernelReturnDownloadByte()
  {
	  int jIByte = m_nDownLoadTotalByte;
	  m_lock_download_byte.Lock();
	 m_nDownLoadTotalByte = 0;
	  m_lock_download_byte.UnLock();
	return jIByte;
  }
 

  int CKernel::NotifyKernelReturnUploadByte()
  {
	  int jIByte = m_nUpLoadTotalByte;
		  m_lock_upload_byte.Lock();
		 m_nUpLoadTotalByte = 0;
		  m_lock_upload_byte.UnLock();
		return jIByte;
  }
  void CKernel::DealRecvDataLength(SOCKET,char szbuf[],long lBuflen)
  {
	  int nUpload = *(int *)szbuf;
	  m_lock_upload_byte.Lock();
	  m_nUpLoadTotalByte +=nUpload;
	  m_lock_upload_byte.UnLock();
  }