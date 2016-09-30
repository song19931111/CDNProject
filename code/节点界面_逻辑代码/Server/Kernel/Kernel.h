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
	virtual void NofityKernelSendFileRequest()= 0;//通知kernel向主服务器发送文件请求
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
	//每一个的文件请求
	CMyFile *m_pFile ;
	SOCKET m_sock ;//为了方便在数据通道中找到命令通道的socket,通知完成端口关闭它
	int m_nBlockIndex ;//文件块大小


};
struct STRU_SEND_DATA{
	//传递给线程函数的结构
CKernel *pThis;
SOCKET socket ;
DWORD dwKey ;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//定义Kernel给任务的接口，使得任务人数回调kernel中的函数，处理任务
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
	CProFile m_profile; //配置文件
	//声明一组函数指针数组,使得DealTask在调用的时候，直接根据数据包类型调用指针数组，增加扩展性
	typedef void (CKernel::*TASK_FUN)(SOCKET,char [],long);
	//创建指针数组
	TASK_FUN m_DealFun[DEF_PACKAGE_COUNT];
	//创建一个默认的处理函数，使得收到不知名的程序包的时候程序不会奔溃 
	void  DefaultDealTask(SOCKET,char [],long);
	void DealSendFileRq(SOCKET ,char szbuf[],long lBuflen);
	void DealSendNodeFileRq(SOCKET ,char szbuf[],long lBuflen);
	void DealRecvDataLength(SOCKET,char szbuf[],long lBuflen);
private :
	MyLock m_lock_deal_task;

//发送的处理
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
//和主节点服务器的交互:
private : 
static unsigned int __stdcall RecvMainProc(void *lpvoid );
map<long,CMyFile *>m_mp_from_main_file ;//[文件key，文件对象]
MyLock m_lock_from_main_file;
public:
friend unsigned int __stdcall RecvMainProc(void *lpvoid);
///////////////////////////////////////////////////////////
//UI通知Kernel的部分:
void NofityKernelSendFileRequest();
 int NotifyKernelReturnDownloadByte();
  int NotifyKernelReturnUploadByte();
 int m_nDownLoadTotalByte; 
 int m_nUpLoadTotalByte; 
 MyLock m_lock_download_byte ;
  MyLock m_lock_upload_byte ;

};


#endif 