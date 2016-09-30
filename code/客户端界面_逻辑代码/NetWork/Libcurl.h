#ifndef ____INCLUDE__LIBCURL__H____
#define ____INCLUDE__LIBCURL__H____
#include <curl/curl.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include "MyLock.h"
using namespace std;
#pragma comment(lib,"libcurl.lib")
struct STRU_HEADER_DATA{
	 char m_szLastModified[50];
	 __int64 m_i64FileSize ;
};
struct STRU_DOWNLOAD_INFO{
	STRU_DOWNLOAD_INFO()
	{
		m_nIndexData = 0;
		m_pTempData = NULL;
		m_nDownloadByte = 0;
	}
	int m_nIndexData; //下载了多少字节
	char *m_pTempData;//下载缓冲
	int m_nDownloadByte; //下载了多少字节
	MyLock m_lock_downloadByte; 
};
class CLibCurl{
private:
	
    char * m_strUrl ;
	
	int m_nDownLoadSize; 
	char *m_pTempData; 
	CURL *m_curlhandle;
	STRU_HEADER_DATA m_stru_HeaderData;
	STRU_DOWNLOAD_INFO m_stru_DownloadInfo;
public :
	friend size_t  wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream);
	STRU_HEADER_DATA * GetInfoFromHttpHeader();
	__int64  DownLoadFile(__int64 i64StartIndex ,__int64 i64EndIndex,char **pDownData, int length);
	CLibCurl(char *pUrl);
	~CLibCurl();
public :
	static int m_nDownloadTotalByte;  //下载速度
	int GetDownloadByte()
	{
		m_stru_DownloadInfo.m_lock_downloadByte.Lock();
		int temp =m_stru_DownloadInfo.m_nDownloadByte;  
		m_stru_DownloadInfo.m_nDownloadByte = 0;
		m_stru_DownloadInfo.m_lock_downloadByte.UnLock();
	return temp ;
	}
};

#endif 

