#include "Libcurl.h"
//HTTP头部的回调函数

int CLibCurl::m_nDownloadTotalByte = 0;
typedef size_t (*PFUN)(void *ptr, size_t size, size_t nmemb, void *stream);
size_t  wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	
	STRU_DOWNLOAD_INFO *pInfo = (STRU_DOWNLOAD_INFO *)stream;
	
	//printf("写文件操作\n");
	 //ptr 接收到的数据 stream curl_easy_opt设置的问及那指针
		
		if(pInfo->m_pTempData!=NULL)
		{
			memcpy(pInfo->m_pTempData+pInfo->m_nIndexData,ptr,size*nmemb);
			//缓冲区索引
			pInfo->m_nIndexData+=size*nmemb;
			//下载速度
			pInfo->m_lock_downloadByte.Lock();
			pInfo->m_nDownloadByte+=size*nmemb;
			pInfo->m_lock_downloadByte.UnLock();
		}
       // return fwrite(ptr, size, nmemb, (FILE *)stream);
		

		return size*nmemb;
}
size_t  getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	STRU_HEADER_DATA *pHeadData = (STRU_HEADER_DATA * )stream;
       int r,r1;
       __int64 len = 0;
	   __int64 t_len;
		r = sscanf((const char *)ptr, "Content-Length: %lld/n", &t_len);
		r1 = sscanf( (const char *)ptr ,"Last-Modified: %s/n",pHeadData->m_szLastModified);
       if(1 ==r1)
	   {
		   strcpy_s(pHeadData->m_szLastModified,50,(const char *)ptr+strlen("Last-Modified: "));
		   char *p  =pHeadData->m_szLastModified;
		   while(*p!='\r'&&*(p+1)!='\n')
		   {
				p++;
				if(*p==':')
				{
				   *p='_';
				}
		   }
		   *p = 0;
	   }
		if (r)
	   {
		   pHeadData->m_i64FileSize= t_len;
	   }
       return size * nmemb;
}
CLibCurl::CLibCurl(char *pUrl)
{
	m_nDownloadTotalByte = 0;
	m_strUrl = pUrl;
	curl_global_init(CURL_GLOBAL_ALL);
     m_curlhandle = curl_easy_init();
	 m_pTempData = NULL;
	 curl_easy_setopt(m_curlhandle, CURLOPT_SSL_VERIFYPEER, 0L);
}
CLibCurl::~CLibCurl()
{
	if(m_stru_DownloadInfo.m_pTempData!=NULL)
	{
		delete m_stru_DownloadInfo.m_pTempData;
	}
	  curl_easy_cleanup(m_curlhandle);
      curl_global_cleanup();
	  if(NULL != m_pTempData)
	  {
		delete m_pTempData;
	  }
}
STRU_HEADER_DATA * CLibCurl::GetInfoFromHttpHeader()
{
	
	
	CURLcode r = CURLE_GOT_NOTHING;
	 curl_easy_setopt(m_curlhandle, CURLOPT_URL, m_strUrl);
	 curl_easy_setopt(m_curlhandle, CURLOPT_HEADER, 1);
	 curl_easy_setopt(m_curlhandle, CURLOPT_CONNECTTIMEOUT, 3000);
	  curl_easy_setopt(m_curlhandle, CURLOPT_HEADERDATA, &m_stru_HeaderData); //设置函数getcontentlengthfunc的stream指针的来源
	 curl_easy_setopt(m_curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);//设置得到头部信息的回调函数
  
	 curl_easy_setopt(m_curlhandle, CURLOPT_NOBODY, 1);
	 curl_easy_perform(m_curlhandle);
	curl_easy_cleanup(m_curlhandle);
	 return  &m_stru_HeaderData;
}
__int64 CLibCurl::DownLoadFile(__int64 i64StartIndex ,__int64 i64EndIndex,char ** pDownData, int nDataSize)
{
	m_stru_DownloadInfo.m_nDownloadByte = 0;
	m_stru_DownloadInfo.m_nIndexData= 0;
	if (m_stru_DownloadInfo.m_pTempData!=NULL)
	{
		delete m_pTempData;
		m_pTempData = NULL;
	}
	
	//curl_global_init(CURL_GLOBAL_ALL);
     m_curlhandle = curl_easy_init();
	 curl_easy_setopt(m_curlhandle, CURLOPT_SSL_VERIFYPEER, 0L);
	CURLcode r = CURLE_GOT_NOTHING;
	if(m_pTempData != NULL)
	{
		delete m_pTempData;
		m_pTempData =NULL;
	}
	
	m_stru_DownloadInfo.m_pTempData= new char [nDataSize];
	//GetSizeFromHttpHeader(pUrl);
	curl_easy_setopt(m_curlhandle, CURLOPT_URL, m_strUrl);
	 curl_easy_setopt(m_curlhandle, CURLOPT_CONNECTTIMEOUT, 3000);
	 curl_easy_setopt(m_curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);//设置得到头部信息的回调函数
	 curl_easy_setopt(m_curlhandle, CURLOPT_HEADERDATA, &m_stru_HeaderData); //设置函数getcontentlengthfunc的stream指针的来源
	char tempData[100];
	sprintf_s(tempData,100,"%lld-%lld",i64StartIndex,i64EndIndex);
	curl_easy_setopt(m_curlhandle,CURLOPT_RANGE,tempData);
	curl_easy_setopt(m_curlhandle, CURLOPT_WRITEDATA, &m_stru_DownloadInfo);
	curl_easy_setopt(m_curlhandle, CURLOPT_WRITEFUNCTION, wirtefunc);
	curl_easy_setopt(m_curlhandle, CURLOPT_NOPROGRESS, 1L);
     curl_easy_setopt(m_curlhandle, CURLOPT_VERBOSE, 1L);
    r = curl_easy_perform(m_curlhandle);
	  if (r == CURLE_OK)
	  {
        //  curl_easy_cleanup(m_curlhandle);
		 *pDownData = m_stru_DownloadInfo.m_pTempData;
		 return m_stru_DownloadInfo.m_nIndexData;
	  }
       else
	   {
              fprintf(stderr, "%s/n", curl_easy_strerror(r));
              return 0;
       }
	 
}

//bool CLibCurl::GetFileName(char szFileName[])
//{
//  //得到真实文件名:
//  char * p  = NULL;
//  p = m_strUrl;
//  //遍历到最尾部
//  while(*p!=0)
//  {
//	p++;
//  }
//  while(p!=m_strUrl)
//  {
//	  if(p =='')
//	p--;
//  }
//  
//}