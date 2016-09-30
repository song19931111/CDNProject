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
#define DEF_MAX_THREADPOOL_TASK_COUNT (3000)  //�̳߳ش����������

using namespace std;
//
//���ڶ����������������ļ���������ֻ��FILE_KEY�ļ��ṹ�е�szSourceFileName�б���ԭ�ļ���
//map<string,CLibcurl *>��<string,int>�������ڷ����ļ����ص��ٶȵĽṹ�����õ���ԭ�ļ����������޸ĺ���ļ���

//////////////////////////////////////////////////////////////////////////////////////////////////////
//����������̳߳�
struct STRU_MAIN_SEVER{
char ip[20];
short port ;
};
class IKernelToDealThreadPool{
public:
	virtual   void DealTaskPool(SOCKET socket ,int nType,CStringBuffer *pBuffer) = 0;  //֪ͨkernel��������
};
class IKernelToNetPool{
public:
	//����������̳߳ع���
	virtual   void DealNetWorkPool(char *pIp, DWORD dwKey,HANDLE quitHandle) = 0;  
};
class CDealTask:public ITask{
public:
	CDealTask(IKernelToDealThreadPool *);	
	SOCKET m_sock;
	CStringBuffer *m_pBuffer; 
	int m_nTypeServer ;//������������
	IKernelToDealThreadPool *m_pKernel;
	bool RunTask();
};
//���ӷ����������ݵ��̳߳�:
class  CNetTask:public ITask{
public:
	char m_szIP[16];
	int m_nIndex ;//socket�ص�index 
    CNetTask(IKernelToNetPool *pKernel);
	DWORD  m_Key;//��һ���ļ�����һ��
	bool RunTask();
	IKernelToNetPool *m_pKernel; //֪ͨkernel �����ˣ����߳�ʱ
	HANDLE m_QuitHandle ;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////
struct STRU_FILE_KEY;
struct STRU_SOCK_NODE_INFO;
struct STRU_BLOCK_KEY{
	STRU_BLOCK_KEY()
	{

		
			m_bFromHttp = false;//����ԭ�����ص�
			m_bFromNode = false;//���Խڵ������������
			nBeginTIme = 0;
			nEndTime =  0;
	}
	//�ļ���ṹ��Ϣ   (��ÿ������ͨ��SOCKETһһ��Ӧ)
//DWORD m_dwKey ;
STRU_FILE_KEY *m_pFileKey;
int m_nPositon ;//������һ��
long m_lBufLen;  //�������
long m_lWriteLen ; //�Ѿ�д�˶���
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
typedef map <DWORD,char *> MAP_BLOCK_INFO;  //<�ļ���key�ṹ���ļ�������>
//STRU_BLOCK_KEY�ĵ�ַ��ΪDWORD��key
class IKernelToUI{
public:
	//UI�����¼�Ĳ���

	//UI�����ʼ���صĲ���
	virtual bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName)=0;
	//UI�����ͣ�Ĳ���
	virtual bool PauseDownLoad(const char *pFileName)=0;
	//UI��������Ĳ���
	virtual bool ContinueDownLoad(const char *pFileName)=0;
	//UI�˳��Ĳ���
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
	//IKernelToUI�ص�
	//UI�����ʼ���صĲ���
	virtual  bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName)=  0;
	//UI�����ͣPauseDownLoad�Ĳ���
	virtual bool PauseDownLoad(const char *pFileName)=0;
	//UI��������Ĳ���
	 virtual bool ContinueDownLoad(const char *pFileName)=0;
	//UI�˳��Ĳ���
	 virtual bool QuitDownLoad()=0;
};

class CKernel :public INotify,public IKernelToDealThreadPool,public IKernelToNetPool,public IKerNelToUI{
public:
	bool Init(IUIToKerel *pUI);
		bool UnInit();
		CKernel();
public:
	//INotify�Ļص�
	 void RecvData(SOCKET,int ,CStringBuffer *pBuffer); //��������Node����Server ,StringBuffer����
	 bool RecvDataChannel(DWORD dwKey,SOCKET socket,char szbuf[],long lBuflen);//�ص���kernel�յ�����ͨ��������
	 void NofityKernelSeriazeKey(long ip, DWORD dwKey,char szbuf[],long lBuflen);//���л�8�ֽڰ�
private:
	char m_szIp[DEF_IP_LEN];
	char m_szDistract[DEF_DISTRACT_LEN];
	char m_szIDCType[DEF_IDC_TYPE_LEN] ;
	IClientNet * m_pNet; //������� 
	CThreadPool m_poolNet; //�����̳߳�
	map<char * ,CMyFile *>m_mp_downloadingFile; //�ļ����ص��б�<�ļ���,�ļ�����>
	//map<char *,CLibCurl *>
	map<char * ,CMyFile *>m_mp_completedFile; //�ļ����ص��б�<�ļ���,�ļ�����>
	map<char * ,CMyFile *>m_mp_FailedFile; //�ļ����ص��б�<�ļ���,+�ļ�����>
	MAP_BLOCK_INFO m_mp_blockInfo;  //<�ļ��� key���ļ�������>
	CThreadPool m_poolDealTask; //�����̳߳�
	CErrCode errcode; // ������
	MyLock m_lock_downloadFile;
	MyLock m_lock_blockInfo;
	MyLock m_lock_deal_task; //ȡ�������
	MyLock m_lock_swich ; //
	MyLock m_lock_complete ;
	MyLock m_lock_download_queue;
	MyLock m_lock_dowload_block_queue;
	MyLock m_lock_sock_node;
	MyLock m_lock_save_config_file; //�����ļ�������
    MyLock m_lock_download_from_node_server ;
	MyLock m_lock_libcurl;
	map<string,int >m_mp_download_from_node_server;  //�����ļ��������ٶ�
	map<string,int >m_mp_download_from_http;  //�����ļ��������ٶ�
	map<string,CLibCurl*>m_mp_libcurl;
	//m_nDownloadFromNodeServerSpeed ;  //��¼�ӷ��������ص��ٶ�
	//map<char *,CLockQueue<STRU_BLOCK_KEY *> *>m_mp_failed_queue ; //����ʧ�ܵĶ���
	map<string,CLockQueue<STRU_BLOCK_KEY *> *>m_mp_dowload_block_queue ; //�������صĶ���
	map<char *,HANDLE> m_mp_thread_switch ; //�߳� ����
	map<DWORD,STRU_SOCK_NODE_INFO *>m_mp_sock_node ; //�洢�ڵ������socket 
private :
	IUIToKerel *m_pUI;
public:
	//IKernelToUI�ص�
	//UI�����ʼ���صĲ���
	 bool StartDownLoad(const char *pUrl,const char *pPath ,const char * pFileName);
	//UI�����ͣPauseDownLoad�Ĳ���
	 bool PauseDownLoad(const char *pFileName);
	//UI��������Ĳ���
	 bool ContinueDownLoad(const char *pFileName);
	//UI�˳��Ĳ���
	 bool QuitDownLoad();
	 //�������ص��ٶ�:
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
	void DealNetWorkPool(char *pIp, DWORD dwKey,HANDLE quitHandle) ;  //�������ӽڵ���������̳߳ش�����
    void DealTaskPool(SOCKET socket ,int nType,CStringBuffer *pBuffer)  ;  //������������̳߳غ���
typedef void (CKernel::*DEAL_FUN)(SOCKET socket ,char szbuf[],long lBuflen);
private :
	DEAL_FUN m_DealFun[DEF_PACK_END-DEF_PACK_START-1];
private :
	//��������:
	void DealDefaultTask(SOCKET ,char szbuf[],long lBuflen);
	void DealSendFileRs(SOCKET ,char szbuf[],long lBuflen);
	//�����������İ�������:
	void DealGetBlockIpRS(SOCKET ,char szbuf[],long lBuflen);
	void  SendMainServerGetIPPackage(STRU_BLOCK_KEY *pKey,__int64 i64FileSize,int nBlockSize,char szUrl[]);
	void DealNoneNodeServerIPRS(SOCKET ,char szbuf[],long lBuflen);
	//����������ʼ�߳�
	static unsigned int _stdcall StartProc(LPVOID pVoid);

private :
	//recv�߳�:
	static unsigned int  _stdcall  RecvMainProc(void * pThis);
	static unsigned int  _stdcall  RecvOnceData(void * pThis);
	//ԭ���̣߳�
	static unsigned int _stdcall DownloadFromHttpProc(void *pParameter);
	//static unsigned int _stdcall DownloadFromHttpTestProc(void *pParameter);
private :
	void StartDown(STRU_FILE_KEY * pstru_fileinfo);
};
////�ļ���key�ṹ��Ϊ�˷�ֹmap��ӵ�char *�Ĵ���
struct STRU_FILE_KEY{
public :
	CKernel *pThis; 
	DWORD dwFileKey;
	char szPath[DEF_MAX_PATH_SIZE];
	char szFileName[DEF_FILENAME_SIZE];
	char szSourceFileName[DEF_FILENAME_SIZE];
	char szUrl[DEF_URL_LEN]; 
};
struct STRU_SOCK_NODE_INFO{//�û����ݸ��ڵ��������߳�
DWORD dwKey ;
SOCKET socket;
int nIndex; //socket�� index

};
struct STRU_SOCK_RECV_ONCE{
	CKernel *pThis; 
	SOCKET socket ;
};
//Ͷ�뵽ԭ���̵߳�:
struct STRU_DWONLOAD_FROM_HTTP{
	CKernel *pThis;
	CLibCurl *pLibcurl;
	CLockQueue<STRU_BLOCK_KEY *>*pQueue ;
	CMyFile *pFile;

};
#endif //____INCLUDE__KERNEL__H____