#pragma once 
#ifndef ____INCLUDE__KERNEL__H____
#define ____INCLUDE__KERNEL__H____
#include "ClientNetImpl.h"
#include "ThreadPool.h"
#include "MyFile.h"
#include "PackDef.h"
#include "MyLock.h"
#include "LockQueue.h"
#include "Libcurl.h"
#include <list>
#include <map>
#include <process.h>
#include "ErrCode.h"
#define MD5_STYLE 32
#define DEF_MAX_THREADPOOL_TASK_COUNT (3000)  //线程池处理的任务数

using namespace std;
//
//由于二次重新了命名了文件名，所以只在FILE_KEY文件结构中的szSourceFileName中保留原文件名
//map<string,CLibcurl *>和<string,int>两个用于返回文件下载的速度的结构均采用的是原文件名，而非修改后的文件名

//////////////////////////////////////////////////////////////////////////////////////////////////////
//处理任务的线程池
struct STRU_MAIN_SEVER{
char ip[20];
short port ;
};
class IKernelToDealThreadPool{
public:
	virtual   void DealTaskPool(SOCKET socket ,int nType,CStringBuffer *pBuffer) = 0;  //通知kernel处理数据
};
class IKernelToNetPool{
public:
	//处理网络的线程池工作
	virtual   void DealNetWorkPool(char *pIp, DWORD dwKey,HANDLE quitHandle) = 0;  
};
class CDealTask:public ITask{
public:
	CDealTask(IKernelToDealThreadPool *);	
	SOCKET m_sock;
	CStringBuffer *m_pBuffer; 
	int m_nTypeServer ;//服务器的类型
	IKernelToDealThreadPool *m_pKernel;
	bool RunTask();
};
//连接服务器收数据的线程池:
class  CNetTask:public ITask{
public:
	char m_szIP[16];
	int m_nIndex ;//socket池的index 
    CNetTask(IKernelToNetPool *pKernel);
	DWORD  m_Key;//哪一个文件的哪一块
	bool RunTask();
	IKernelToNetPool *m_pKernel; //通知kernel 收完了，或者超时
	HANDLE m_QuitHandle ;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////
struct STRU_FILE_KEY;
struct STRU_SOCK_NODE_INFO;
struct STRU_BLOCK_KEY{
	STRU_BLOCK_KEY()
	{

		
			m_bFromHttp = false;//来自原链下载的
			m_bFromNode = false;//来自节点服务器的数据
			nBeginTIme = 0;
			nEndTime =  0;
	}
	//文件块结构信息   (跟每个数据通道SOCKET一一对应)
//DWORD m_dwKey ;
STRU_FILE_KEY *m_pFileKey;
int m_nPositon ;//存在哪一块
long m_lBufLen;  //多大数据
long m_lWriteLen ; //已经写了多少
char md5[DEF_MD5_LENGTH];
bool m_bFromHttp;
bool m_bFromNode;
int nBeginTIme;
int nEndTime; 
};
class CKernel;
struct STRU_UI_FILE_INFO{
CKernel *pThis ; 
char *pUrl ; 
char *pPath;
char *pFileName;
};
struct STRU_SPEED{
    string strFileName;
	int m_nDownloadByte ;

};
typedef map <DWORD,char *> MAP_BLOCK_INFO;  //<文件名key结构，文件块数据>
//STRU_BLOCK_KEY的地址作为DWORD的key
class IKernelToUI{
public:
	//UI点击登录的操作

	//UI点击开始下载的操作
	virtual bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName)=0;
	//UI点击暂停的操作
	virtual bool PauseDownLoad(const char *pFileName)=0;
	//UI点击继续的操作
	virtual bool ContinueDownLoad(const char *pFileName)=0;
	//UI退出的操作
	virtual bool QuitDownLoad()=0;
public :
	virtual map<string,int >  NotifyKernelReturnHttpDownloadByte() =  0;
	virtual map<string,int >  NofiyKernelReturnNodeServerByte() = 0;
    //  virtual int NotifyKernelReturnDownloadRange();
};
class IUIToKerel{
public :
	//virtual void NotifyUIUpdateCompleteFile(char *pFile,__int64 i64Size) =  0;
    virtual void NofifyUITip(string strTip)=  0;
	virtual void NofityUIFileInfo(string strFileName,__int64 fileSize,int HasWrittenByte)= 0;
};
class IKerNelToUI{
public :
	//IKernelToUI回调
	//UI点击开始下载的操作
	virtual  bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName)=  0;
	//UI点击暂停PauseDownLoad的操作
	virtual bool PauseDownLoad(const char *pFileName)=0;
	//UI点击继续的操作
	 virtual bool ContinueDownLoad(const char *pFileName)=0;
	//UI退出的操作
	 virtual bool QuitDownLoad()=0;
};

class CKernel :public INotify,public IKernelToDealThreadPool,public IKernelToNetPool,public IKerNelToUI{
public:
	bool Init(IUIToKerel *pUI);
		bool UnInit();
		CKernel();
public:
	//INotify的回调
	 void RecvData(SOCKET,int ,CStringBuffer *pBuffer); //何种类型Node还是Server ,StringBuffer数据
	 bool RecvDataChannel(DWORD dwKey,SOCKET socket,char szbuf[],long lBuflen);//回调给kernel收到数据通道的数据
	 void NofityKernelSeriazeKey(long ip, DWORD dwKey,char szbuf[],long lBuflen);//序列化8字节包
private:
	char m_szIp[DEF_IP_LEN];
	char m_szDistract[DEF_DISTRACT_LEN];
	char m_szIDCType[DEF_IDC_TYPE_LEN] ;
	IClientNet * m_pNet; //网络对象 
	CThreadPool m_poolNet; //网络线程池
	map<char * ,CMyFile *>m_mp_downloadingFile; //文件下载的列表<文件名,文件对象>
	//map<char *,CLibCurl *>
	map<char * ,CMyFile *>m_mp_completedFile; //文件下载的列表<文件名,文件对象>
	map<char * ,CMyFile *>m_mp_FailedFile; //文件下载的列表<文件名,+文件对象>
	MAP_BLOCK_INFO m_mp_blockInfo;  //<文件的 key，文件块数据>
	CThreadPool m_poolDealTask; //处理线程池
	CErrCode errcode; // 错误类
	MyLock m_lock_downloadFile;
	MyLock m_lock_blockInfo;
	MyLock m_lock_deal_task; //取任务的锁
	MyLock m_lock_swich ; //
	MyLock m_lock_complete ;
	MyLock m_lock_download_queue;
	MyLock m_lock_dowload_block_queue;
	MyLock m_lock_sock_node;
	MyLock m_lock_save_config_file; //保存文件配置锁
    MyLock m_lock_download_from_node_server ;
	MyLock m_lock_libcurl;
	map<string,int >m_mp_download_from_node_server;  //所有文件的下载速度
	map<string,int >m_mp_download_from_http;  //所有文件的下载速度
	map<string,CLibCurl*>m_mp_libcurl;
	//m_nDownloadFromNodeServerSpeed ;  //记录从服务器下载的速度
	//map<char *,CLockQueue<STRU_BLOCK_KEY *> *>m_mp_failed_queue ; //下载失败的队列
	map<string,CLockQueue<STRU_BLOCK_KEY *> *>m_mp_dowload_block_queue ; //即将下载的队列
	map<char *,HANDLE> m_mp_thread_switch ; //线程 开关
	map<DWORD,STRU_SOCK_NODE_INFO *>m_mp_sock_node ; //存储节点的所有socket 
private :
	IUIToKerel *m_pUI;
public:
	//IKernelToUI回调
	//UI点击开始下载的操作
	 bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName);
	//UI点击暂停PauseDownLoad的操作
	 bool PauseDownLoad(const char *pFileName);
	//UI点击继续的操作
	 bool ContinueDownLoad(const char *pFileName);
	//UI退出的操作
	 bool QuitDownLoad();
	 //返回下载的速度:
public :
	  map<string,int >  NotifyKernelReturnHttpDownloadByte() ;
	  map<string,int > NofiyKernelReturnNodeServerByte();
	// list<STRU_SPEED *>ls_node_server_speed;
	 //list<STRU_SPEED *>ls_http_speed;

private :
	STRU_MAIN_SEVER m_struMaiNServer; 
public : 
	 friend unsigned int    RecvMainProc(void * pThis);
	 friend int main();
private :
	void DealNetWorkPool(char *pIp, DWORD dwKey,HANDLE quitHandle) ;  //处理连接节点服务器的线程池处理函数
    void DealTaskPool(SOCKET socket ,int nType,CStringBuffer *pBuffer)  ;  //处理解包请求的线程池函数
typedef void (CKernel::*DEAL_FUN)(SOCKET socket ,char szbuf[],long lBuflen);
private :
	DEAL_FUN m_DealFun[DEF_PACK_END-DEF_PACK_START-1];
private :
	//包处理函数:
	void DealDefaultTask(SOCKET ,char szbuf[],long lBuflen);
	void DealSendFileRs(SOCKET ,char szbuf[],long lBuflen);
	//和主服务器的包处理函数:
	void DealGetBlockIpRS(SOCKET ,char szbuf[],long lBuflen);
	void  SendMainServerGetIPPackage(STRU_BLOCK_KEY *pKey,__int64 i64FileSize,int nBlockSize,char szUrl[]);
	void DealNoneNodeServerIPRS(SOCKET ,char szbuf[],long lBuflen);
	//主服务器开始线程
	static unsigned int _stdcall StartProc(LPVOID pVoid);

private :
	//recv线程:
	static unsigned int  _stdcall  RecvMainProc(void * pThis);
	static unsigned int  _stdcall  RecvOnceData(void * pThis);
	//原链线程：
	static unsigned int _stdcall DownloadFromHttpProc(void *pParameter);
	//static unsigned int _stdcall DownloadFromHttpTestProc(void *pParameter);
private :
	void StartDown(STRU_FILE_KEY * pstru_fileinfo);
};
////文件的key结构，为了防止map添加的char *的错误
struct STRU_FILE_KEY{
public :
	CKernel *pThis; 
	DWORD dwFileKey;
	char szPath[DEF_MAX_PATH_SIZE];
	char szFileName[DEF_FILENAME_SIZE];
	char szSourceFileName[DEF_FILENAME_SIZE];
	char szUrl[DEF_URL_LEN]; 
};
struct STRU_SOCK_NODE_INFO{//用户传递给节点收数据线程
DWORD dwKey ;
SOCKET socket;
int nIndex; //socket的 index

};
struct STRU_SOCK_RECV_ONCE{
	CKernel *pThis; 
	SOCKET socket ;
};
//投入到原链线程的:
struct STRU_DWONLOAD_FROM_HTTP{
	CKernel *pThis;
	CLibCurl *pLibcurl;
	CLockQueue<STRU_BLOCK_KEY *>*pQueue ;
	CMyFile *pFile;

};
#endif //____INCLUDE__KERNEL__H____