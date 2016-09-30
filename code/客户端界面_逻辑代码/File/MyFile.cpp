#include "MyFile.h"
long CMyFile::lMinLength = 4*sizeof(int)+4*sizeof(long)+sizeof(char *)+sizeof(__int64)+1*sizeof(DWORD);
CMyFile::CMyFile(const char *pAllPath):m_pFileBlockInfo(NULL),m_bOpenFileMap(false),m_handleMap(NULL),m_bLittleReadFlag(false),m_bLittleWriteFlag(false),m_Raddr(NULL),m_Waddr(NULL),m_i64WriteFilePos(0),m_i64ReadFilePos(0),m_i64WriteLen(0),m_i64ReadLen(0),m_nProfileSize(0)
{
	m_pTempCache = NULL;
	m_i64WriteFilePos = 0;
	m_i64_size = 0;
	m_nHasWrittenBlock = 0;
	m_nProfileSize = 0;
	m_lTempCacheLength  = 0;
	//读取已存在的文件构造 
	memset(m_chAllPath,0,sizeof(m_chAllPath));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(m_szPath,0,sizeof(m_szPath));
	memset(m_url,0,sizeof(m_url));
	//读文件方式构造
	m_fileHandle =::CreateFile(pAllPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	int err = GetLastError();
	if(INVALID_HANDLE_VALUE == m_fileHandle)
	{

	    return ;
	}
	//获取文件大小:
	DWORD dwSizeHigh;
	DWORD dwSizeLow;
	dwSizeLow = ::GetFileSize(m_fileHandle,&dwSizeHigh);
	m_i64_size = ((__int64)dwSizeHigh<<32)+dwSizeLow;
	strcpy_s(m_chAllPath,200,pAllPath);
	m_lAllPathLength = strlen(pAllPath) +1;
	CloseHandle(m_fileHandle);
	m_fileHandle = NULL;
}
CMyFile::CMyFile(const char *pFileName,const char *pPath, __int64 i64_size):m_pFileBlockInfo(NULL),m_bOpenFileMap(false),m_handleMap(NULL),m_bLittleReadFlag(false),m_bLittleWriteFlag(false),m_Raddr(NULL),m_Waddr(NULL),m_i64WriteFilePos(0),m_i64ReadFilePos(0),m_i64WriteLen(0),m_i64ReadLen(0),m_nProfileSize(0)//不存在的文件构造
{
	m_lTempCacheLength = 0;
	m_pTempCache =  NULL;
	m_i64WriteFilePos = 0; 
	m_i64_size = 0;
	m_nHasWrittenBlock = 0;
	m_nProfileSize = 0;
	
	//不存在的文件构造
	//参数检测
	if(NULL==pFileName|| NULL==pPath)
	{
		return ;
	}
	memset(m_chAllPath,0,sizeof(m_chAllPath));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(m_szPath,0,sizeof(m_szPath));
	memset(m_url,0,sizeof(m_url));
	//文件长度
	m_lFilenameLength  =strlen(pFileName)+1;
	//路径长度：
	m_lPathLength  =strlen(pPath)+1;
	//文件大小
	m_i64_size = i64_size;
	//文件名
	memcpy(m_szFileName,pFileName,m_lFilenameLength);
	//文件路径
	memcpy(m_szPath,pPath,m_lPathLength);
	//初始化文件的全路径
	//拼接文件
	strcat_s(m_chAllPath,200,m_szPath);
	strcat_s(m_chAllPath,200,m_szFileName);
	m_lAllPathLength  =strlen(m_szPath)+strlen(m_szFileName)+1;
}
void  CMyFile::SetFileName(char szFileName[])
{
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(m_chAllPath,0,sizeof(m_chAllPath));
	strcpy_s(m_szFileName,DEF_FILENAME_SIZE,szFileName);
	strcat_s(m_chAllPath,200,m_szPath);
	strcat_s(m_chAllPath,200,m_szFileName);
	m_lAllPathLength  =strlen(m_szPath)+strlen(m_szFileName)+1;
}
bool CMyFile::InitFile()
{
	UnInitFIle();
	m_i =  0;
	//设置块大小:
	SetBlockSize();
	//得到文件总块数
	if(0 == m_i64_size)
	{
		return false ; 
	}
	m_nTotalBlock  = m_i64_size/m_nBlockSize;
	if(0!=m_i64_size%m_nBlockSize|| 0==m_nTotalBlock)
	{
		m_nTotalBlock+=1;
	}
	//初始化文件块信息
	m_pFileBlockInfo = new char[m_nTotalBlock];
	memset(m_pFileBlockInfo,0,m_nTotalBlock);
	if(NULL == m_pFileBlockInfo)
	{
		return false;
	}
	//初始化文件映射参数：
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwGran = sys.dwAllocationGranularity;
	m_dwMapSize =  160*dwGran;   //文件的粒度
	m_i64T =(__int64) 0.6 * m_dwMapSize;

	
	
	return true;

}
bool CMyFile::InitFileMappingInfo()
{
	//在不需要块读写的时候，调用
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwGran = sys.dwAllocationGranularity;
	m_dwMapSize =320*dwGran;   //文件的粒度
	m_i64T =(double) 0.6 * m_dwMapSize;
	return true; 
	
}
//块写完成后进行写文件操作
bool CMyFile::SaveBlockInfo()
{
	if(m_szPath==NULL)
	{
		return false ;
	}
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//以二进制打开文件
	FILE *fp  ;
	fopen_s(&fp,szSavePath,"wb");
	if(NULL==fp)
	{
		//fclose(fp);
		return false ;
	}
	//序列化文件信息:
	int nSize = lMinLength+100+100+m_nTotalBlock*sizeof(char);
	char  *pTemp = new char[nSize];
	char * temp = pTemp ; 
	memset(pTemp, 0,nSize);
	long len  =Seriaze(pTemp,nSize);
	//序列化失败
	if(0==len)
	{
		delete pTemp;
		pTemp = NULL;
		return false; 
	}
//写入文件信息:
	fseek(fp,0,SEEK_SET);
		//写入文件大小
	fwrite(&len,sizeof(len),1,fp);
		//写入序列化数据
	fwrite(pTemp,len,1,fp);
	//关闭文件
	fclose(fp);
	//删除指针
	//delete [] temp;
	temp  =NULL;
	pTemp = NULL;
	return true ;
}
bool CMyFile::ReadBlockInfo()
{
	//读取文件
	//取得长度
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//以二进制打开文件

	FILE *fp  = fopen(szSavePath,"rb");
	if(NULL==fp)
	{
		
		//fclose(fp);
		return false ;
	}
	//读取文件信息:
	int nReadlen = 0; 
	//要读多大
	fread(&nReadlen,sizeof(nReadlen),1,fp);
	if(0==nReadlen)
	{
		return false ;
	}
	char szBuf[1024];
	int nRead = fread(szBuf,1,nReadlen*sizeof(char),fp);
	if(nRead!=nReadlen)
	{
		//读失败
		return false ;
	}
	long len  = 0;
	len  =UnSeriaze(szBuf,sizeof(szBuf));
	//序列化失败
	if(0==len)
	{
		return false; 
	}

	//关闭文件
	fclose(fp);
	this->SetBlockSize();
	return true ;
}

bool CMyFile::UnInitFIle()
{
	//删除文件块信息:
	if(NULL!=m_pFileBlockInfo)
	{
		delete m_pFileBlockInfo;
	}
	//删除文件映射:
	if(m_bOpenFileMap)
	{
		m_bOpenFileMap = false; 
		CloseHandle(m_handleMap);
		m_handleMap =NULL;
	}
	return true;
}
CMyFile::~CMyFile()
{
}
void CMyFile::SetBlockSize()
{
	//TODO：采用一定的分块的算法
	m_nBlockSize = DEF_ONE_MB;
}
long CMyFile::Seriaze(char szbuf[],long lBufSize)
{
	//校验大小
	if(lMinLength+100+100+m_nTotalBlock>lBufSize)
	{
		return 0;
	}
	//路径长度
	*(long*)szbuf = m_lPathLength;
	szbuf+=sizeof(long);
	//路径
	memcpy(szbuf,m_szPath,m_lPathLength);
	szbuf +=m_lPathLength;
	//文件名长度
	*(long*)szbuf =  m_lFilenameLength;
	szbuf+=sizeof(long);
	//文件名
	memcpy(szbuf,m_szFileName,m_lFilenameLength);
	szbuf +=m_lFilenameLength;
	//文件总块数
	*(int*)szbuf =  m_nTotalBlock;
	szbuf+=sizeof(int);
	//文件信息块
	memcpy(szbuf,m_pFileBlockInfo,m_nTotalBlock);
	szbuf+=m_nTotalBlock;	
	//文件大小
	*(__int64*)szbuf =  m_i64_size;
	szbuf+=sizeof(__int64);
	//文件的粒度:
	*(DWORD*)szbuf = m_dwMapSize;
	szbuf +=sizeof(DWORD);
	//文件的全路径长度:
	*(long*)szbuf = m_lAllPathLength;
	szbuf+=sizeof(long);
	//文件全路径
	memcpy(szbuf,m_chAllPath,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	//ＵＲL:
	m_lUrlLen  =strlen(m_url)+1;

	*(long*)szbuf = m_lUrlLen;
	szbuf+=sizeof(long);

	memcpy(szbuf,m_url,m_lUrlLen);
	szbuf+=m_lUrlLen;

	//配置文件的大小
	*(int *)szbuf  =m_nProfileSize;
	//已经读写的块数：
	*(int *) szbuf  =m_nHasWrittenBlock;
	szbuf +=sizeof(int );


	
	return lMinLength+m_lPathLength+m_lFilenameLength+m_lAllPathLength+m_lUrlLen+m_nTotalBlock;
}
long CMyFile::UnSeriaze(char szbuf[],long lBufSize)
{
		//校验大小
	if(lMinLength+100+100+m_nTotalBlock>lBufSize)
	{
		return 0;
	}
	//路径长度
	 m_lPathLength= *(long*)szbuf ;
	szbuf+=sizeof(long);
	//路径
	memcpy(m_szPath,szbuf,m_lPathLength);
	szbuf +=m_lPathLength;
	//文件名长度
	m_lFilenameLength=*(long*)szbuf  ;
	szbuf+=sizeof(long);
	//文件名
	memcpy(m_szFileName,szbuf,m_lFilenameLength);
	szbuf +=m_lFilenameLength;
	//文件总块数
	m_nTotalBlock=*(int*)szbuf  ;
	szbuf+=sizeof(int);
	//申请空间
	m_pFileBlockInfo  =new char[m_nTotalBlock];
	//文件信息块
	memcpy(m_pFileBlockInfo,szbuf,m_nTotalBlock);
	szbuf+=m_nTotalBlock;
	
	//文件大小
	m_i64_size =*(__int64*)szbuf ;
	szbuf+=sizeof(__int64);
	//文件的粒度:
	m_dwMapSize = *(DWORD*)szbuf ;
	szbuf +=sizeof(DWORD);
	//文件的全路径长度:
	m_lAllPathLength = *(long*)szbuf  ;
	szbuf+=sizeof(long);
	//文件全路径
	memcpy(m_chAllPath,szbuf,m_lAllPathLength);
	szbuf+=m_lAllPathLength;

	m_lUrlLen=*(long*)szbuf  ;
	szbuf+=sizeof(long);

	memcpy(m_url,szbuf,m_lUrlLen);
	szbuf+=m_lUrlLen;

		//配置文件的大小
	m_nProfileSize =*(int *)szbuf  ;
	//已经读写的块数：
	m_nHasWrittenBlock=*(int *) szbuf  ;
	szbuf +=sizeof(int );

	return lMinLength+m_lPathLength+m_lFilenameLength+m_lAllPathLength+m_lUrlLen;
}
long CMyFile::ReadFile(int m_nPosition,void *pData)
{
	//返回读到的长度，读取的数据存入pData
	if(NULL==pData)
	{
		return 0; 
	}
	if(m_nPosition>m_nTotalBlock-1)
	{
		return  0;
	}
	if(false ==m_bOpenFileMap)
	{
		if(false == OpenFileMap(OPEN_EXISTING))
		{
			int err =  GetLastError();
			cout<<"OpenFileMap"<<err<<endl;
		}
		return 0;	
	}
	//获得该文件块所在的位置：
	__int64 filePos  = m_nPosition*m_nBlockSize;
	DWORD dwLowPos= (int)filePos;
	DWORD dwHighPos = (filePos-dwLowPos)>>32;
	//获得虚拟内存的起始地址
	void *addr  = NULL;
	DWORD dwMapSize = 0;//映射的大小
	if(m_nPosition==m_nTotalBlock-1)
	{
			//最后一块
		 dwMapSize = m_i64_size-(m_nTotalBlock-1)*m_nBlockSize;
	}
	else
	{
		dwMapSize = m_nBlockSize;
	}
	addr = MapViewOfFile(m_handleMap,FILE_MAP_WRITE,dwHighPos,dwLowPos,dwMapSize);
	if(NULL==addr)
	{
		return 0 ;
	}
	//写入数据
	memcpy(pData,addr,dwMapSize);
	//撤销映射
	UnmapViewOfFile(addr);
	return dwMapSize;
 }


long CMyFile::WriteFile(int m_nPosition,char *pData)
{
	if(NULL==pData)
	{
		return 0; 
	}
	if(m_nPosition>m_nTotalBlock-1)
	{
		return  0;
	}
	if(false == m_bOpenFileMap)
	{
		if(false  == OpenFileMap(OPEN_EXISTING))
		{
			int err = GetLastError();
			cout<<"OpenFileMap"<<err<<endl;
			return 0;
		}
		
	}
	//获得该文件块所在的位置：
	__int64 filePos  = m_nPosition*m_nBlockSize;
	DWORD dwLowPos= (int)filePos;
	DWORD dwHighPos = (filePos-dwLowPos)>>32;
	//获得虚拟内存的起始地址
	void *addr  = NULL;
	DWORD dwMapSize = 0;//映射的大小
	if(m_nPosition==m_nTotalBlock-1)
	{
		 dwMapSize = m_i64_size-(m_nTotalBlock-1)*m_nBlockSize;
	}
	else
	{
		dwMapSize = m_nBlockSize;
	}
	addr = MapViewOfFile(m_handleMap,FILE_MAP_WRITE,dwHighPos,dwLowPos,dwMapSize);
	if(NULL==addr)
	{
		return 0 ;
	}
	//写入数据
	memcpy(addr,pData,dwMapSize);
	//撤销映射
	UnmapViewOfFile(addr);
	return dwMapSize;
 }

bool CMyFile::OpenFileMap(int nFileMode)
{

	//以何种方式打开文件映射
	HANDLE hFile  =::CreateFile(m_chAllPath,GENERIC_READ|GENERIC_WRITE,NULL,NULL,nFileMode ,FILE_FLAG_RANDOM_ACCESS,NULL);
 	if(hFile==INVALID_HANDLE_VALUE)
	 {
		 int err= GetLastError();
		 cout<<"OpenMaping ErrCode"<<err<<endl;
		 CloseHandle(hFile);
		 hFile = NULL;
		return false; 
	 }
	m_handleMap=::CreateFileMapping(hFile,NULL,PAGE_READWRITE,(DWORD)(m_i64_size>>32),(DWORD)m_i64_size,NULL);
	if(NULL==m_handleMap)
	{
		 CloseHandle(hFile);
		 hFile = NULL;
		return false ;
	}
    m_bOpenFileMap = true;
    CloseHandle(hFile);
	hFile = NULL;
	return true ;
}
long CMyFile::CreateInitFile()
{
	//创建一个大小一样文件:
	 if(true ==OpenFileMap(CREATE_ALWAYS))
	 {
		 return m_i64_size;
	 }
	 return 0;
}
long CMyFile::WriteFilleEx(__int64 i64FileIndex ,char *pData , long m_lBufLen)//从文件的哪个位置开始写，写多长
{
	//一次写入大数据
	//打开文件映射
	if(false == m_bOpenFileMap)
	{
		if(false ==OpenFileMap(OPEN_ALWAYS))
		{
			cout<<"OpenMaping ErrCode"<<endl;
			return 0;
		}
	}
	//int n_lastDataLength  = 0; //最后一块数据的长度:
	//拼接文件
	DWORD dwLow  =(DWORD)m_i64_size;
	DWORD dwHigh =(DWORD)((m_i64_size-dwLow)>>32);
	//获取文件映射的次数
	int nMapCount  = m_lBufLen/m_dwMapSize;
	if(0!=m_lBufLen%m_dwMapSize||0 == nMapCount)
	{
		nMapCount += 1;
	}
	DWORD dwMapSize = 	m_dwMapSize; //每次映射的文件大小
	for(int nMapIndex=0;nMapIndex<nMapCount;nMapIndex++)
	{

		if(1 == nMapCount ||nMapIndex==nMapCount-1)
		{
			//最后一次要写入多少字节:
			DWORD nLastByte  = m_lBufLen- (nMapCount-1)*m_dwMapSize;
			//获得最后一次映射的字节数
			dwMapSize  =nLastByte;
		}
		//获得映射地址:
		__int64 filePos_index= i64FileIndex+m_dwMapSize*nMapIndex;
		DWORD filePosLow = (DWORD)filePos_index;
		DWORD filePosHigh = (DWORD)((filePos_index-filePosLow)>>32);
		void *addr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,filePosHigh,filePosLow,dwMapSize);
		if(addr==NULL)
		{
			 int err = ::GetLastError();
			 cout<<"错误"<<err<<endl;
			return 0;
		}
	//	lock.Lock();
		memcpy(addr,pData,dwMapSize);
	//	lock.UnLock();
		pData+=dwMapSize;
	}//for
		return m_lBufLen;
}
long CMyFile::ReadFileEx(__int64 i64FileIndex ,char *pData , long m_lReadLen)//从文件的哪个位置开始读，读多长
{
	int nReadLen  = 0;
	//打开文件映射
	m_lock_open_file_mappong.Lock();
	if(false == m_bOpenFileMap)
	{
		if(false ==OpenFileMap(OPEN_EXISTING))
		{
			cout<<m_bOpenFileMap<<endl;
			cout<<"OpenMaping ErrCode"<<endl;
			m_lock_open_file_mappong.UnLock();
			return 0;
		}
	}
	m_lock_open_file_mappong.UnLock();
	int n_lastDataLength  = 0; //最后一块数据的长度:
	//拼接文件
	DWORD dwLow  =(DWORD)m_i64_size;
	DWORD dwHigh =(DWORD)((m_i64_size-dwLow)>>32);
	//获取文件映射的次数
	int nMapCount  = m_lReadLen/m_dwMapSize;
	if(0!=m_lReadLen%m_dwMapSize||0 == nMapCount)
	{
		nMapCount += 1;
	}
	DWORD dwMapSize = 	m_dwMapSize; //每次映射的文件大小
	for(int nMapIndex=0;nMapIndex<nMapCount;nMapIndex++)
	{
		//关闭文件句柄
	//1M1M的写入文件映射：
		//获取写入次数	
		int nWriteMapCount = m_dwMapSize/m_nBlockSize;
		if(0!=m_dwMapSize%m_nBlockSize|| 0==nWriteMapCount)
		{
					nWriteMapCount++;
		}
		if(1 == nMapCount ||nMapIndex==nMapCount-1)
		{//小于等于文件粒度的情况,或者最后一次写入
			//最后一次要写入多少字节:
			DWORD nLastByte  = m_lReadLen- (nMapCount-1)*m_dwMapSize;
			//最后一次映射的地址:
			dwMapSize  =nLastByte;
		}
		__int64 filePos_index= i64FileIndex+m_dwMapSize*nMapIndex;
		DWORD filePosLow = (DWORD)filePos_index;
		DWORD filePosHigh = (DWORD)((filePos_index-filePosLow)>>32);
		void *addr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,filePosHigh,filePosLow,dwMapSize);
		if(addr==NULL)
		{
			 int err = ::GetLastError();
			 cout<<"错误"<<err<<endl;
			return 0;
		}
		///lock.Lock();
		memcpy(pData,addr,dwMapSize);
		nReadLen+=dwMapSize;
		//lock.UnLock();
		pData+= dwMapSize;
		UnmapViewOfFile(addr);
	}
	return nReadLen;
}
long CMyFile::WriteFileLittleData(char *pData , long m_lBufLen)
{
		if(m_lBufLen>m_dwMapSize)
		{
			//如果超过了文件的粒度的倍数，写小数据将失败;
			return false ;
		}
		
		//1.打开文件映射
		if(false == m_bOpenFileMap)
		{
			if(false  == OpenFileMap(OPEN_EXISTING))
			{
				int err = GetLastError();
				cout<<"OpenFileMap"<<err<<endl;
				return 0;
			}
		}
		//2.建立文件映射，写数据
		if(false == m_bLittleWriteFlag)
		{
			m_bLittleWriteFlag = true ;
			
			m_i64WriteT  =m_i64T;
			__int64 dwMapSize =m_i64_size-m_i64WriteFilePos;
			if(dwMapSize<m_dwMapSize)
			{
				m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,0,0,dwMapSize);
			}
			else
			{
				m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,0,0,m_dwMapSize);
			}
			if(NULL == m_Waddr)
			{
				int err = GetLastError();
				cout<<"MapError"<<err<<endl;
				m_bLittleWriteFlag = false ;
				m_i64WriteLen = 0;
				m_i64WriteFilePos  = 0;	
				m_Waddr = NULL;
				return 0;
			}
		}
		//if(m_i==24)
		//{
		// int a=  10;
		//}
		//写数据
		char *pTempData = (char *)m_Waddr;
		memcpy(pTempData+m_i64WriteLen,pData,m_lBufLen); 
		//文件位置增加
		m_i64WriteFilePos+= m_lBufLen;
		//文件写入的数据长度增加
		m_i64WriteLen +=m_lBufLen;
	//检测是否超过了槛值
		if(m_i64WriteFilePos>m_i64WriteT)
		{//如果超过
				//取消文件映射
				if(0 ==UnmapViewOfFile(m_Waddr))
				{
					int err =GetLastError();
					cout<<"MapError"<<err<<endl;
					m_bLittleWriteFlag = false ;
					m_i64WriteLen = 0;
					m_i64WriteFilePos  = 0;	
					//UnmapViewOfFile(m_addr);
					m_Waddr = NULL;
					return 0;
				}
				m_Waddr  = NULL;
				//在此地址上,建立新的文件 映射更新地址
				__int64 dwMapSize =m_i64_size-m_i64WriteFilePos;
				if(dwMapSize<m_dwMapSize)
				{
					m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,(DWORD)(m_i64WriteFilePos>>32),(DWORD)m_i64WriteFilePos,dwMapSize);
					m_i64WriteT = m_i64_size+1 ;
				}
				else
				{
					m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,(DWORD)(m_i64WriteFilePos>>32),(DWORD)m_i64WriteFilePos,m_dwMapSize);
				//槛值增加
				m_i64WriteT =m_i64WriteFilePos+m_i64T;
				}
				m_i64WriteLen =  0  ;  //清空已经写的数据
				if(NULL == m_Waddr)
				{
					int err = GetLastError();
					cout<<"MapError"<<err<<endl;
					m_bLittleWriteFlag = false ;
					m_i64WriteLen = 0;
					m_i64WriteFilePos  = 0;	
					//UnmapViewOfFile(m_addr);
					m_Waddr = NULL;
					return 0;
				}

		 }
	//判断是否写完
		if(m_i64WriteFilePos ==m_i64_size)
	{
		//如果写完将各个变量清零处理:
		m_bLittleWriteFlag = false ;
		m_i64WriteLen = 0;
		//m_i64WriteFilePos  = 0;	
		UnmapViewOfFile(m_Waddr);
		m_Waddr = NULL;
    	m_i64WriteT = 0;
	}
		m_i++;
	return m_lBufLen;
}
long CMyFile::ReadFileLittleData(char *pData , long m_lBufLen)
{
			if(m_lBufLen>m_dwMapSize)
		{
			//如果超过了文件的粒度的倍数，写小数据将失败;
			return false ;
		}
		//1.打开文件映射
		if(false == m_bOpenFileMap)
		{

			if(false  == OpenFileMap(OPEN_EXISTING))
			{
				int err = GetLastError();
				cout<<"OpenFileMap"<<err<<endl;
				return 0;
			}
		}
		//2.建立文件映射，读数据
		if(false ==m_bLittleReadFlag)
		{
				m_i64ReadT = m_i64T;
			m_bLittleReadFlag = true ;
			__int64 dwMapSize =m_i64_size-m_i64ReadFilePos;
			if(dwMapSize<(__int64)m_dwMapSize)
			{
				m_Raddr =MapViewOfFile(m_handleMap,FILE_MAP_READ,0,0,(DWORD)dwMapSize);
			}
			else
			{
				m_Raddr =MapViewOfFile(m_handleMap,FILE_MAP_READ,0,0,m_dwMapSize);
			}
			if(NULL == m_Raddr)
			{
				int err = GetLastError();
				cout<<"MapError"<<err<<endl;
				m_bLittleReadFlag = false ;
				m_i64ReadLen = 0;
				m_i64ReadFilePos  = 0;	
				m_Raddr = NULL;
				return 0;
			}
		}
		if(m_i==24)
		{
		 int a=  10;
		}
		//读数据
		char *pTempData = (char *)m_Raddr;
		memcpy(pData,pTempData+m_i64ReadLen,m_lBufLen); 
		//文件位置增加
		m_i64ReadFilePos+= m_lBufLen;
		//文件写入的数据增加
		m_i64ReadLen +=m_lBufLen;
	//检测是否超过了槛值
		if(m_i64ReadFilePos>=m_i64ReadT)
		{//如果超过
				//取消文件映射
				UnmapViewOfFile(m_Raddr);
				m_i64ReadLen = 0;
				m_Raddr  = NULL;
				//在此地址上,建立新的文件 映射更新地址
				__int64 dwMapSize =m_i64_size-m_i64ReadFilePos;
				if(dwMapSize<m_dwMapSize)
				{
					m_Raddr =MapViewOfFile(m_handleMap,FILE_MAP_READ,(DWORD)(m_i64ReadFilePos>>32),(DWORD)m_i64ReadFilePos&0xFFFFFFFF,dwMapSize);
					m_i64ReadT = m_i64_size+1;
					//+1
				}
				else
				{
					DWORD dwHigh = (DWORD)(m_i64ReadFilePos>>32);
					DWORD dwLow =  (DWORD)m_i64ReadFilePos&0xFFFFFFFF;
					m_Raddr =MapViewOfFile(m_handleMap,FILE_MAP_READ,dwHigh,dwLow,m_dwMapSize);
					m_i64ReadT =m_i64ReadFilePos+m_i64T;
				}
				if(NULL == m_Raddr)
				{
					int err = GetLastError();
					cout<<"MapError"<<err<<endl;
					m_bLittleReadFlag = false ;
					m_i64WriteLen = 0;
					//UnmapViewOfFile(m_addr);
					m_Raddr = NULL;
					return 0;
				}
		 }
	//判断是否写完
		if(m_i64ReadFilePos ==m_i64_size)
	{
		m_bLittleReadFlag = false ;
		m_i64ReadLen = 0;
		m_i64ReadFilePos  = 0;	
		UnmapViewOfFile(m_Raddr);
		m_Raddr = NULL;
		m_i64ReadT = 0;
    //如果写完将各个变量清零处理:
	}
		m_i++;
		return m_lBufLen;
}
char * CMyFile::md5File(long lMd5Len)
{////////////
	 MD5_CTX mdContext;
	 unsigned char *pData = new unsigned char[DEF_ONE_MB];
	 memset(pData,0,DEF_ONE_MB);
	 char *file_md5;
	 int i;
	 //初始化	
	 MD5Init (&mdContext);
	 //读文件
	 __int64 dwReadSize = DEF_ONE_MB;
	 __int64 filePos = 0  ;
	 int index=0;
	 while (1)
	 {

		 if(filePos + DEF_ONE_MB>m_i64_size)
		 {
			 
			 dwReadSize = m_i64_size-filePos;
			ReadFileLittleData((char *)pData, dwReadSize);
	
			MD5Update (&mdContext, pData, dwReadSize);
			break;
		 }
		 else
		 {
			ReadFileLittleData((char *)pData,dwReadSize);	
		 }
		 filePos+=DEF_ONE_MB;
		// cout<<"第"<<index++<<"块"<<endl;
		MD5Update (&mdContext, pData, dwReadSize);
	 }
	 MD5Final (&mdContext);

	 file_md5 = (char *)malloc((lMd5Len + 1) * sizeof(char));
	 
	 if(file_md5 == NULL)
	 {
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	 }
	 memset(file_md5, 0, lMd5Len+1);

	 if(lMd5Len == 16)
	 {
		  for(i=4; i<12; i++)
		  {
				sprintf(&file_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else if(lMd5Len == 32)
	 {
		  for(i=0; i<16; i++)
		  {
				sprintf(&file_md5[i*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else
	 {
		  free(file_md5);
		  return NULL;
	 }
	 ////////////////////////////////////////////
	 return file_md5;
}
char * CMyFile::md5FileBlock(int nBlock,long lMd5Len)
{
	MD5_CTX mdContext;
	unsigned char *pData = new unsigned char[m_nBlockSize];
	 memset(pData,0,m_nBlockSize);
	 char *file_md5;
	 int i;
	 //初始化	
	 MD5Init (&mdContext);
	 //读文件
	 DWORD dwReadSize = m_nBlockSize;
	 __int64 filePos = 0  ;
	 int index=0;
	 if(0==ReadFile(nBlock,pData))
	 {
		delete pData;
		return NULL;
	 }
	 MD5Update (&mdContext, pData, m_nBlockSize);
	 MD5Final (&mdContext);

	 file_md5 = (char *)malloc((lMd5Len + 1) * sizeof(char));
	 
	 if(file_md5 == NULL)
	 {
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	 }
	 memset(file_md5, 0, lMd5Len+1);

	 if(lMd5Len == 16)
	 {
		  for(i=4; i<12; i++)
		  {
				sprintf(&file_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else if(lMd5Len == 32)
	 {
		  for(i=0; i<16; i++)
		  {
				sprintf(&file_md5[i*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else
	 {
		  free(file_md5);
		  return NULL;
	 }
	 ////////////////////////////////////////////
	 return file_md5;
	return NULL;
}
char * CMyFile::md5String(unsigned char *pData,long lBuflen,long lMd5Len)
{
	MD5_CTX mdContext;

	 char *string_md5;
	 int i;
	 //初始化	
	 MD5Init (&mdContext);
	 //读文件
	 DWORD dwReadSize = m_nBlockSize;
	 __int64 filePos = 0  ;
	 int index=0;
	 MD5Update (&mdContext, pData, lBuflen);
	 MD5Final (&mdContext);

	 string_md5 = (char *)malloc((lMd5Len + 1) * sizeof(char));
	 
	 if(string_md5 == NULL)
	 {
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	 }
	 memset(string_md5, 0, lMd5Len+1);

	 if(lMd5Len == 16)
	 {
		  for(i=4; i<12; i++)
		  {
				sprintf(&string_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else if(lMd5Len == 32)
	 {
		  for(i=0; i<16; i++)
		  {
				sprintf(&string_md5[i*2], "%02x", mdContext.digest[i]);
		  }
	 }
	 else
	 {
		  free(string_md5);
		  return NULL;
	 }
	 ////////////////////////////////////////////
	 return string_md5;
	return NULL;
}
char * CMyFile::ReadOneBlockMd5(int nBlock)//在文件的前几M写入配置文件的情况下
{
	if(m_nProfileSize == 0)
	{
		return NULL; 
	}
	char * md5 = new char [DEF_MD5_LENGTH];
	ReadFileEx(m_lFilenameLength+nBlock*DEF_MD5_LENGTH,md5,DEF_MD5_LENGTH);
	return md5;
}

bool  CMyFile::SaveFileInfo()//对主服务器接收的文件的配置进行保存
{
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//以二进制打开文件
	FILE *fp  ;
	fopen_s(&fp,szSavePath,"wb");
	if(NULL==fp)
	{
		//fclose(fp);
		return false ;
	}
	//序列化文件信息:
	char szSaveBuf[4096] ={0};

	long len  =SeriazeMainFile(szSaveBuf,4096);
	//序列化失败
	if(0==len)
	{
		return false; 
	}
//写入文件信息:
	fseek(fp,0,SEEK_SET);
		//写入文件大小
	fwrite(&len,sizeof(len),1,fp);
		//写入序列化数据
	fwrite(szSaveBuf,len,1,fp);
	//关闭文件
	fclose(fp);
	//删除指针
	//delete [] temp;
	return true ;
}
bool  CMyFile::ReadFileInfo()//对主服务器接收的文件的配置进行读取
{
	//读取文件
	//取得长度
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//以二进制打开文件

	FILE *fp  ;
	fopen_s(&fp,szSavePath,"rb");
	if(NULL==fp)
	{
		
		//fclose(fp);
		return false ;
	}
	//读取文件信息:
	int nReadlen = 0; 
	//要读多大
	fread(&nReadlen,sizeof(nReadlen),1,fp);
	if(0==nReadlen)
	{
		return false ;
	}
	char szBuf[1024];
	int nRead = fread(szBuf,1,nReadlen*sizeof(char),fp);
	if(nRead!=nReadlen)
	{
		//读失败
		return false ;
	}
	long len  = 0;
//序列化数据：
	UnSeriazeMainFile(szBuf,nReadlen);
	

	//len  =UnSeriaze(szBuf,sizeof(szBuf));
	////序列化失败
	//if(0==len)
	//{
	//	return false; 
	//}

	//关闭文件
	fclose(fp);
	this->SetBlockSize();
	return true ;
	
}
long CMyFile::SeriazeMainFile(char szbuf[],long lBufSize)
{
		//文件名长度
	*(long *)szbuf = m_lFilenameLength  ;
	szbuf+=sizeof(long);
	//文件名
	memcpy(szbuf,m_szFileName,m_lFilenameLength);
	szbuf+=m_lFilenameLength;
	//文件的大小
	*(__int64*)szbuf=m_i64_size  ;
	szbuf+= sizeof(__int64);
	//文件的全路径长度
	 *(long *)szbuf=m_lAllPathLength ;
	szbuf +=sizeof(long);
	//文件全路径
	memcpy(szbuf,m_chAllPath,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	// 已经写了的字节数
	*(__int64*)szbuf =m_i64WriteFilePos  ;
	szbuf+= sizeof(__int64);
	return sizeof(long)*2+sizeof(__int64)*2+m_lFilenameLength+m_lAllPathLength;
	

}
long CMyFile::UnSeriazeMainFile(char szbuf[],long lBufSize)
{
	//文件名长度
	m_lFilenameLength = *(long *)szbuf;
	szbuf+=sizeof(long);
	//文件名
	memcpy(m_szFileName,szbuf,m_lFilenameLength);
	szbuf+=m_lFilenameLength;
	//文件的大小
	m_i64_size = *(__int64*)szbuf;
	szbuf+= sizeof(__int64);
	//文件的全路径长度
	m_lAllPathLength = *(long *)szbuf;
	szbuf +=sizeof(long);
	//文件全路径
	memcpy(m_chAllPath,szbuf,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	// 已经写了的字节数
	m_i64WriteFilePos  =*(__int64*)szbuf;
	szbuf+= sizeof(__int64);
	return sizeof(long)*2+sizeof(__int64)*2+m_lFilenameLength+m_lAllPathLength;
}