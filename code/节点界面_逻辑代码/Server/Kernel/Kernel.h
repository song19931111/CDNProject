#pragma once 

#ifndef ____INCLUDE__KERNEL__H____
#define ____INCLUDE__KERNEL__H____
#define DEF_MAX_THREADPOOL_TASK_COUNT (30000)
#include "NetWork.h"
#include  "IOCP.h"
#include "ThreadPool.h"
#include "PackDef.h"
#include "MyFile.h"
#include "ErrCode.h"
#include <string>
#include <set>
#include "ProFile.h"
class CKernel ;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IKerenlToUI{
public:
	virtual void NofityKernelSendFileRequest()= 0;//֪ͨkernel���������������ļ�����
	virtual int NotifyKernelReturnDownloadByte()  = 0;
	virtual int NotifyKernelReturnUploadByte() = 0;
};
class IUIToKernel{
public :
	virtual void NofifyUILoginSuccess()= 0;
	 virtual void NotifyUIUpdataClientCount(int n) = 0;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct STRU_EACH_FILE_INFO{
	//ÿһ�����ļ�����
	CMyFile *m_pFile ;
	SOCKET m_sock ;//Ϊ�˷���������ͨ�����ҵ�����ͨ����socket,֪ͨ��ɶ˿ڹر���
	int m_nBlockIndex ;//�ļ����С


};
struct STRU_SEND_DATA{
	//���ݸ��̺߳����Ľṹ
CKernel *pThis;
SOCKET socket ;
DWORD dwKey ;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//����Kernel������Ľӿڣ�ʹ�����������ص�kernel�еĺ�������������
class IKernelToTask{
public:
	virtual bool DealTask(SOCKET socket, CStringBuffer * pBuffer )=0;
};
class IKernelToNetTask{
public :
virtual bool DealNetTask(long m_lKey,HANDLE quitHandle)=0;
};
class CRecvTask:public ITask{
public:
	bool RunTask();
	CRecvTask(IKernelToTask *pKernel):m_pKernel(pKernel){}
public:
	SOCKET m_sock;
	CStringBuffer *m_pBuffer; 
private:
	IKernelToTask *m_pKernel;
};


class CNetTask:public ITask{
public:
	bool RunTask();
	CNetTask(IKernelToNetTask *pKernel):m_pKernel(pKernel){}
public:
	long m_lFileKey;
	HANDLE m_quitHandle ;
private:
	IKernelToNetTask *m_pKernel;
};
class CKernel :public INotify,public IKernelToTask,public IKernelToNetTask,public CKerenlToClientNet,IKerenlToUI{
public:
CKernel();
~CKernel();
bool RecvData(SOCKET socket, CStringBuffer *pBuffer);
 void NotifyKernelClientCount(int n) ;
 bool RecvDataChannel(long lFileKey ,SOCKET socket,char szbuf[],long lBuflen);
bool OpenKernel(IUIToKernel *pUI);
bool CloseKernel();
public :
	CErrCode errcode; 
private :

	bool DealTask(SOCKET socket, CStringBuffer * pBuffer );
	bool DealNetTask(long m_lKey,HANDLE quitHandle);
private:
	IUIToKernel *m_pUI;
	IIOCP *m_pNet;
	CClientNet * m_pClientNet ;
	CThreadPool  *m_pThreadPool;
	CThreadPool *m_pNetPool ;
	CProFile m_profile; //�����ļ�
	//����һ�麯��ָ������,ʹ��DealTask�ڵ��õ�ʱ��ֱ�Ӹ������ݰ����͵���ָ�����飬������չ��
	typedef void (CKernel::*TASK_FUN)(SOCKET,char [],long);
	//����ָ������
	TASK_FUN m_DealFun[DEF_PACKAGE_COUNT];
	//����һ��Ĭ�ϵĴ�������ʹ���յ���֪���ĳ������ʱ����򲻻ᱼ�� 
	void  DefaultDealTask(SOCKET,char [],long);
	void DealSendFileRq(SOCKET ,char szbuf[],long lBuflen);
	void DealSendNodeFileRq(SOCKET ,char szbuf[],long lBuflen);
	void DealRecvDataLength(SOCKET,char szbuf[],long lBuflen);
private :
	MyLock m_lock_deal_task;

//���͵Ĵ���
private:
	static unsigned int _stdcall SendProc(void * pVoid);
	bool  SendClientFileBlock(SOCKET socket ,STRU_EACH_FILE_INFO *);
	void DealDataChanelPackage(SOCKET socket,char szbuf[],long lBuflen);
	void DealClosePackage(SOCKET socket,char szbuf[],long lBuflen);
	void DealRequestSendMainServerRs(SOCKET socket,char szbuf[],long lBuflen);
private :
	MyLock m_lock_each_send_info;
	map<DWORD,STRU_EACH_FILE_INFO*>m_mp_each_send_info;
	MyLock m_lock_file_obj;
	map<string,CMyFile *>m_mp_file_obj;
//�����ڵ�������Ľ���:
private : 
static unsigned int __stdcall RecvMainProc(void *lpvoid );
map<long,CMyFile *>m_mp_from_main_file ;//[�ļ�key���ļ�����]
MyLock m_lock_from_main_file;
public:
friend unsigned int __stdcall RecvMainProc(void *lpvoid);
///////////////////////////////////////////////////////////
//UI֪ͨKernel�Ĳ���:
void NofityKernelSendFileRequest();
 int NotifyKernelReturnDownloadByte();
  int NotifyKernelReturnUploadByte();
 int m_nDownLoadTotalByte; 
 int m_nUpLoadTotalByte; 
 MyLock m_lock_download_byte ;
  MyLock m_lock_upload_byte ;

};


#endif 