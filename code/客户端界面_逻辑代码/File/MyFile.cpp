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
	//��ȡ�Ѵ��ڵ��ļ����� 
	memset(m_chAllPath,0,sizeof(m_chAllPath));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(m_szPath,0,sizeof(m_szPath));
	memset(m_url,0,sizeof(m_url));
	//���ļ���ʽ����
	m_fileHandle =::CreateFile(pAllPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	int err = GetLastError();
	if(INVALID_HANDLE_VALUE == m_fileHandle)
	{

	    return ;
	}
	//��ȡ�ļ���С:
	DWORD dwSizeHigh;
	DWORD dwSizeLow;
	dwSizeLow = ::GetFileSize(m_fileHandle,&dwSizeHigh);
	m_i64_size = ((__int64)dwSizeHigh<<32)+dwSizeLow;
	strcpy_s(m_chAllPath,200,pAllPath);
	m_lAllPathLength = strlen(pAllPath) +1;
	CloseHandle(m_fileHandle);
	m_fileHandle = NULL;
}
CMyFile::CMyFile(const char *pFileName,const char *pPath, __int64 i64_size):m_pFileBlockInfo(NULL),m_bOpenFileMap(false),m_handleMap(NULL),m_bLittleReadFlag(false),m_bLittleWriteFlag(false),m_Raddr(NULL),m_Waddr(NULL),m_i64WriteFilePos(0),m_i64ReadFilePos(0),m_i64WriteLen(0),m_i64ReadLen(0),m_nProfileSize(0)//�����ڵ��ļ�����
{
	m_lTempCacheLength = 0;
	m_pTempCache =  NULL;
	m_i64WriteFilePos = 0; 
	m_i64_size = 0;
	m_nHasWrittenBlock = 0;
	m_nProfileSize = 0;
	
	//�����ڵ��ļ�����
	//�������
	if(NULL==pFileName|| NULL==pPath)
	{
		return ;
	}
	memset(m_chAllPath,0,sizeof(m_chAllPath));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(m_szPath,0,sizeof(m_szPath));
	memset(m_url,0,sizeof(m_url));
	//�ļ�����
	m_lFilenameLength  =strlen(pFileName)+1;
	//·�����ȣ�
	m_lPathLength  =strlen(pPath)+1;
	//�ļ���С
	m_i64_size = i64_size;
	//�ļ���
	memcpy(m_szFileName,pFileName,m_lFilenameLength);
	//�ļ�·��
	memcpy(m_szPath,pPath,m_lPathLength);
	//��ʼ���ļ���ȫ·��
	//ƴ���ļ�
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
	//���ÿ��С:
	SetBlockSize();
	//�õ��ļ��ܿ���
	if(0 == m_i64_size)
	{
		return false ; 
	}
	m_nTotalBlock  = m_i64_size/m_nBlockSize;
	if(0!=m_i64_size%m_nBlockSize|| 0==m_nTotalBlock)
	{
		m_nTotalBlock+=1;
	}
	//��ʼ���ļ�����Ϣ
	m_pFileBlockInfo = new char[m_nTotalBlock];
	memset(m_pFileBlockInfo,0,m_nTotalBlock);
	if(NULL == m_pFileBlockInfo)
	{
		return false;
	}
	//��ʼ���ļ�ӳ�������
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwGran = sys.dwAllocationGranularity;
	m_dwMapSize =  160*dwGran;   //�ļ�������
	m_i64T =(__int64) 0.6 * m_dwMapSize;

	
	
	return true;

}
bool CMyFile::InitFileMappingInfo()
{
	//�ڲ���Ҫ���д��ʱ�򣬵���
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwGran = sys.dwAllocationGranularity;
	m_dwMapSize =320*dwGran;   //�ļ�������
	m_i64T =(double) 0.6 * m_dwMapSize;
	return true; 
	
}
//��д��ɺ����д�ļ�����
bool CMyFile::SaveBlockInfo()
{
	if(m_szPath==NULL)
	{
		return false ;
	}
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//�Զ����ƴ��ļ�
	FILE *fp  ;
	fopen_s(&fp,szSavePath,"wb");
	if(NULL==fp)
	{
		//fclose(fp);
		return false ;
	}
	//���л��ļ���Ϣ:
	int nSize = lMinLength+100+100+m_nTotalBlock*sizeof(char);
	char  *pTemp = new char[nSize];
	char * temp = pTemp ; 
	memset(pTemp, 0,nSize);
	long len  =Seriaze(pTemp,nSize);
	//���л�ʧ��
	if(0==len)
	{
		delete pTemp;
		pTemp = NULL;
		return false; 
	}
//д���ļ���Ϣ:
	fseek(fp,0,SEEK_SET);
		//д���ļ���С
	fwrite(&len,sizeof(len),1,fp);
		//д�����л�����
	fwrite(pTemp,len,1,fp);
	//�ر��ļ�
	fclose(fp);
	//ɾ��ָ��
	//delete [] temp;
	temp  =NULL;
	pTemp = NULL;
	return true ;
}
bool CMyFile::ReadBlockInfo()
{
	//��ȡ�ļ�
	//ȡ�ó���
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//�Զ����ƴ��ļ�

	FILE *fp  = fopen(szSavePath,"rb");
	if(NULL==fp)
	{
		
		//fclose(fp);
		return false ;
	}
	//��ȡ�ļ���Ϣ:
	int nReadlen = 0; 
	//Ҫ�����
	fread(&nReadlen,sizeof(nReadlen),1,fp);
	if(0==nReadlen)
	{
		return false ;
	}
	char szBuf[1024];
	int nRead = fread(szBuf,1,nReadlen*sizeof(char),fp);
	if(nRead!=nReadlen)
	{
		//��ʧ��
		return false ;
	}
	long len  = 0;
	len  =UnSeriaze(szBuf,sizeof(szBuf));
	//���л�ʧ��
	if(0==len)
	{
		return false; 
	}

	//�ر��ļ�
	fclose(fp);
	this->SetBlockSize();
	return true ;
}

bool CMyFile::UnInitFIle()
{
	//ɾ���ļ�����Ϣ:
	if(NULL!=m_pFileBlockInfo)
	{
		delete m_pFileBlockInfo;
	}
	//ɾ���ļ�ӳ��:
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
	//TODO������һ���ķֿ���㷨
	m_nBlockSize = DEF_ONE_MB;
}
long CMyFile::Seriaze(char szbuf[],long lBufSize)
{
	//У���С
	if(lMinLength+100+100+m_nTotalBlock>lBufSize)
	{
		return 0;
	}
	//·������
	*(long*)szbuf = m_lPathLength;
	szbuf+=sizeof(long);
	//·��
	memcpy(szbuf,m_szPath,m_lPathLength);
	szbuf +=m_lPathLength;
	//�ļ�������
	*(long*)szbuf =  m_lFilenameLength;
	szbuf+=sizeof(long);
	//�ļ���
	memcpy(szbuf,m_szFileName,m_lFilenameLength);
	szbuf +=m_lFilenameLength;
	//�ļ��ܿ���
	*(int*)szbuf =  m_nTotalBlock;
	szbuf+=sizeof(int);
	//�ļ���Ϣ��
	memcpy(szbuf,m_pFileBlockInfo,m_nTotalBlock);
	szbuf+=m_nTotalBlock;	
	//�ļ���С
	*(__int64*)szbuf =  m_i64_size;
	szbuf+=sizeof(__int64);
	//�ļ�������:
	*(DWORD*)szbuf = m_dwMapSize;
	szbuf +=sizeof(DWORD);
	//�ļ���ȫ·������:
	*(long*)szbuf = m_lAllPathLength;
	szbuf+=sizeof(long);
	//�ļ�ȫ·��
	memcpy(szbuf,m_chAllPath,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	//�գ�L:
	m_lUrlLen  =strlen(m_url)+1;

	*(long*)szbuf = m_lUrlLen;
	szbuf+=sizeof(long);

	memcpy(szbuf,m_url,m_lUrlLen);
	szbuf+=m_lUrlLen;

	//�����ļ��Ĵ�С
	*(int *)szbuf  =m_nProfileSize;
	//�Ѿ���д�Ŀ�����
	*(int *) szbuf  =m_nHasWrittenBlock;
	szbuf +=sizeof(int );


	
	return lMinLength+m_lPathLength+m_lFilenameLength+m_lAllPathLength+m_lUrlLen+m_nTotalBlock;
}
long CMyFile::UnSeriaze(char szbuf[],long lBufSize)
{
		//У���С
	if(lMinLength+100+100+m_nTotalBlock>lBufSize)
	{
		return 0;
	}
	//·������
	 m_lPathLength= *(long*)szbuf ;
	szbuf+=sizeof(long);
	//·��
	memcpy(m_szPath,szbuf,m_lPathLength);
	szbuf +=m_lPathLength;
	//�ļ�������
	m_lFilenameLength=*(long*)szbuf  ;
	szbuf+=sizeof(long);
	//�ļ���
	memcpy(m_szFileName,szbuf,m_lFilenameLength);
	szbuf +=m_lFilenameLength;
	//�ļ��ܿ���
	m_nTotalBlock=*(int*)szbuf  ;
	szbuf+=sizeof(int);
	//����ռ�
	m_pFileBlockInfo  =new char[m_nTotalBlock];
	//�ļ���Ϣ��
	memcpy(m_pFileBlockInfo,szbuf,m_nTotalBlock);
	szbuf+=m_nTotalBlock;
	
	//�ļ���С
	m_i64_size =*(__int64*)szbuf ;
	szbuf+=sizeof(__int64);
	//�ļ�������:
	m_dwMapSize = *(DWORD*)szbuf ;
	szbuf +=sizeof(DWORD);
	//�ļ���ȫ·������:
	m_lAllPathLength = *(long*)szbuf  ;
	szbuf+=sizeof(long);
	//�ļ�ȫ·��
	memcpy(m_chAllPath,szbuf,m_lAllPathLength);
	szbuf+=m_lAllPathLength;

	m_lUrlLen=*(long*)szbuf  ;
	szbuf+=sizeof(long);

	memcpy(m_url,szbuf,m_lUrlLen);
	szbuf+=m_lUrlLen;

		//�����ļ��Ĵ�С
	m_nProfileSize =*(int *)szbuf  ;
	//�Ѿ���д�Ŀ�����
	m_nHasWrittenBlock=*(int *) szbuf  ;
	szbuf +=sizeof(int );

	return lMinLength+m_lPathLength+m_lFilenameLength+m_lAllPathLength+m_lUrlLen;
}
long CMyFile::ReadFile(int m_nPosition,void *pData)
{
	//���ض����ĳ��ȣ���ȡ�����ݴ���pData
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
	//��ø��ļ������ڵ�λ�ã�
	__int64 filePos  = m_nPosition*m_nBlockSize;
	DWORD dwLowPos= (int)filePos;
	DWORD dwHighPos = (filePos-dwLowPos)>>32;
	//��������ڴ����ʼ��ַ
	void *addr  = NULL;
	DWORD dwMapSize = 0;//ӳ��Ĵ�С
	if(m_nPosition==m_nTotalBlock-1)
	{
			//���һ��
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
	//д������
	memcpy(pData,addr,dwMapSize);
	//����ӳ��
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
	//��ø��ļ������ڵ�λ�ã�
	__int64 filePos  = m_nPosition*m_nBlockSize;
	DWORD dwLowPos= (int)filePos;
	DWORD dwHighPos = (filePos-dwLowPos)>>32;
	//��������ڴ����ʼ��ַ
	void *addr  = NULL;
	DWORD dwMapSize = 0;//ӳ��Ĵ�С
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
	//д������
	memcpy(addr,pData,dwMapSize);
	//����ӳ��
	UnmapViewOfFile(addr);
	return dwMapSize;
 }

bool CMyFile::OpenFileMap(int nFileMode)
{

	//�Ժ��ַ�ʽ���ļ�ӳ��
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
	//����һ����Сһ���ļ�:
	 if(true ==OpenFileMap(CREATE_ALWAYS))
	 {
		 return m_i64_size;
	 }
	 return 0;
}
long CMyFile::WriteFilleEx(__int64 i64FileIndex ,char *pData , long m_lBufLen)//���ļ����ĸ�λ�ÿ�ʼд��д�೤
{
	//һ��д�������
	//���ļ�ӳ��
	if(false == m_bOpenFileMap)
	{
		if(false ==OpenFileMap(OPEN_ALWAYS))
		{
			cout<<"OpenMaping ErrCode"<<endl;
			return 0;
		}
	}
	//int n_lastDataLength  = 0; //���һ�����ݵĳ���:
	//ƴ���ļ�
	DWORD dwLow  =(DWORD)m_i64_size;
	DWORD dwHigh =(DWORD)((m_i64_size-dwLow)>>32);
	//��ȡ�ļ�ӳ��Ĵ���
	int nMapCount  = m_lBufLen/m_dwMapSize;
	if(0!=m_lBufLen%m_dwMapSize||0 == nMapCount)
	{
		nMapCount += 1;
	}
	DWORD dwMapSize = 	m_dwMapSize; //ÿ��ӳ����ļ���С
	for(int nMapIndex=0;nMapIndex<nMapCount;nMapIndex++)
	{

		if(1 == nMapCount ||nMapIndex==nMapCount-1)
		{
			//���һ��Ҫд������ֽ�:
			DWORD nLastByte  = m_lBufLen- (nMapCount-1)*m_dwMapSize;
			//������һ��ӳ����ֽ���
			dwMapSize  =nLastByte;
		}
		//���ӳ���ַ:
		__int64 filePos_index= i64FileIndex+m_dwMapSize*nMapIndex;
		DWORD filePosLow = (DWORD)filePos_index;
		DWORD filePosHigh = (DWORD)((filePos_index-filePosLow)>>32);
		void *addr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,filePosHigh,filePosLow,dwMapSize);
		if(addr==NULL)
		{
			 int err = ::GetLastError();
			 cout<<"����"<<err<<endl;
			return 0;
		}
	//	lock.Lock();
		memcpy(addr,pData,dwMapSize);
	//	lock.UnLock();
		pData+=dwMapSize;
	}//for
		return m_lBufLen;
}
long CMyFile::ReadFileEx(__int64 i64FileIndex ,char *pData , long m_lReadLen)//���ļ����ĸ�λ�ÿ�ʼ�������೤
{
	int nReadLen  = 0;
	//���ļ�ӳ��
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
	int n_lastDataLength  = 0; //���һ�����ݵĳ���:
	//ƴ���ļ�
	DWORD dwLow  =(DWORD)m_i64_size;
	DWORD dwHigh =(DWORD)((m_i64_size-dwLow)>>32);
	//��ȡ�ļ�ӳ��Ĵ���
	int nMapCount  = m_lReadLen/m_dwMapSize;
	if(0!=m_lReadLen%m_dwMapSize||0 == nMapCount)
	{
		nMapCount += 1;
	}
	DWORD dwMapSize = 	m_dwMapSize; //ÿ��ӳ����ļ���С
	for(int nMapIndex=0;nMapIndex<nMapCount;nMapIndex++)
	{
		//�ر��ļ����
	//1M1M��д���ļ�ӳ�䣺
		//��ȡд�����	
		int nWriteMapCount = m_dwMapSize/m_nBlockSize;
		if(0!=m_dwMapSize%m_nBlockSize|| 0==nWriteMapCount)
		{
					nWriteMapCount++;
		}
		if(1 == nMapCount ||nMapIndex==nMapCount-1)
		{//С�ڵ����ļ����ȵ����,�������һ��д��
			//���һ��Ҫд������ֽ�:
			DWORD nLastByte  = m_lReadLen- (nMapCount-1)*m_dwMapSize;
			//���һ��ӳ��ĵ�ַ:
			dwMapSize  =nLastByte;
		}
		__int64 filePos_index= i64FileIndex+m_dwMapSize*nMapIndex;
		DWORD filePosLow = (DWORD)filePos_index;
		DWORD filePosHigh = (DWORD)((filePos_index-filePosLow)>>32);
		void *addr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,filePosHigh,filePosLow,dwMapSize);
		if(addr==NULL)
		{
			 int err = ::GetLastError();
			 cout<<"����"<<err<<endl;
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
			//����������ļ������ȵı�����дС���ݽ�ʧ��;
			return false ;
		}
		
		//1.���ļ�ӳ��
		if(false == m_bOpenFileMap)
		{
			if(false  == OpenFileMap(OPEN_EXISTING))
			{
				int err = GetLastError();
				cout<<"OpenFileMap"<<err<<endl;
				return 0;
			}
		}
		//2.�����ļ�ӳ�䣬д����
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
		//д����
		char *pTempData = (char *)m_Waddr;
		memcpy(pTempData+m_i64WriteLen,pData,m_lBufLen); 
		//�ļ�λ������
		m_i64WriteFilePos+= m_lBufLen;
		//�ļ�д������ݳ�������
		m_i64WriteLen +=m_lBufLen;
	//����Ƿ񳬹��˼�ֵ
		if(m_i64WriteFilePos>m_i64WriteT)
		{//�������
				//ȡ���ļ�ӳ��
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
				//�ڴ˵�ַ��,�����µ��ļ� ӳ����µ�ַ
				__int64 dwMapSize =m_i64_size-m_i64WriteFilePos;
				if(dwMapSize<m_dwMapSize)
				{
					m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,(DWORD)(m_i64WriteFilePos>>32),(DWORD)m_i64WriteFilePos,dwMapSize);
					m_i64WriteT = m_i64_size+1 ;
				}
				else
				{
					m_Waddr =MapViewOfFile(m_handleMap,FILE_MAP_WRITE,(DWORD)(m_i64WriteFilePos>>32),(DWORD)m_i64WriteFilePos,m_dwMapSize);
				//��ֵ����
				m_i64WriteT =m_i64WriteFilePos+m_i64T;
				}
				m_i64WriteLen =  0  ;  //����Ѿ�д������
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
	//�ж��Ƿ�д��
		if(m_i64WriteFilePos ==m_i64_size)
	{
		//���д�꽫�����������㴦��:
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
			//����������ļ������ȵı�����дС���ݽ�ʧ��;
			return false ;
		}
		//1.���ļ�ӳ��
		if(false == m_bOpenFileMap)
		{

			if(false  == OpenFileMap(OPEN_EXISTING))
			{
				int err = GetLastError();
				cout<<"OpenFileMap"<<err<<endl;
				return 0;
			}
		}
		//2.�����ļ�ӳ�䣬������
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
		//������
		char *pTempData = (char *)m_Raddr;
		memcpy(pData,pTempData+m_i64ReadLen,m_lBufLen); 
		//�ļ�λ������
		m_i64ReadFilePos+= m_lBufLen;
		//�ļ�д�����������
		m_i64ReadLen +=m_lBufLen;
	//����Ƿ񳬹��˼�ֵ
		if(m_i64ReadFilePos>=m_i64ReadT)
		{//�������
				//ȡ���ļ�ӳ��
				UnmapViewOfFile(m_Raddr);
				m_i64ReadLen = 0;
				m_Raddr  = NULL;
				//�ڴ˵�ַ��,�����µ��ļ� ӳ����µ�ַ
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
	//�ж��Ƿ�д��
		if(m_i64ReadFilePos ==m_i64_size)
	{
		m_bLittleReadFlag = false ;
		m_i64ReadLen = 0;
		m_i64ReadFilePos  = 0;	
		UnmapViewOfFile(m_Raddr);
		m_Raddr = NULL;
		m_i64ReadT = 0;
    //���д�꽫�����������㴦��:
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
	 //��ʼ��	
	 MD5Init (&mdContext);
	 //���ļ�
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
		// cout<<"��"<<index++<<"��"<<endl;
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
	 //��ʼ��	
	 MD5Init (&mdContext);
	 //���ļ�
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
	 //��ʼ��	
	 MD5Init (&mdContext);
	 //���ļ�
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
char * CMyFile::ReadOneBlockMd5(int nBlock)//���ļ���ǰ��Mд�������ļ��������
{
	if(m_nProfileSize == 0)
	{
		return NULL; 
	}
	char * md5 = new char [DEF_MD5_LENGTH];
	ReadFileEx(m_lFilenameLength+nBlock*DEF_MD5_LENGTH,md5,DEF_MD5_LENGTH);
	return md5;
}

bool  CMyFile::SaveFileInfo()//�������������յ��ļ������ý��б���
{
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//�Զ����ƴ��ļ�
	FILE *fp  ;
	fopen_s(&fp,szSavePath,"wb");
	if(NULL==fp)
	{
		//fclose(fp);
		return false ;
	}
	//���л��ļ���Ϣ:
	char szSaveBuf[4096] ={0};

	long len  =SeriazeMainFile(szSaveBuf,4096);
	//���л�ʧ��
	if(0==len)
	{
		return false; 
	}
//д���ļ���Ϣ:
	fseek(fp,0,SEEK_SET);
		//д���ļ���С
	fwrite(&len,sizeof(len),1,fp);
		//д�����л�����
	fwrite(szSaveBuf,len,1,fp);
	//�ر��ļ�
	fclose(fp);
	//ɾ��ָ��
	//delete [] temp;
	return true ;
}
bool  CMyFile::ReadFileInfo()//�������������յ��ļ������ý��ж�ȡ
{
	//��ȡ�ļ�
	//ȡ�ó���
	char szSavePath[DEF_MAX_PATH_SIZE]={0};
	strncpy_s(szSavePath,m_chAllPath,m_lAllPathLength);
	strcat_s(szSavePath,DEF_MAX_PATH_SIZE,".cfg");
	//�Զ����ƴ��ļ�

	FILE *fp  ;
	fopen_s(&fp,szSavePath,"rb");
	if(NULL==fp)
	{
		
		//fclose(fp);
		return false ;
	}
	//��ȡ�ļ���Ϣ:
	int nReadlen = 0; 
	//Ҫ�����
	fread(&nReadlen,sizeof(nReadlen),1,fp);
	if(0==nReadlen)
	{
		return false ;
	}
	char szBuf[1024];
	int nRead = fread(szBuf,1,nReadlen*sizeof(char),fp);
	if(nRead!=nReadlen)
	{
		//��ʧ��
		return false ;
	}
	long len  = 0;
//���л����ݣ�
	UnSeriazeMainFile(szBuf,nReadlen);
	

	//len  =UnSeriaze(szBuf,sizeof(szBuf));
	////���л�ʧ��
	//if(0==len)
	//{
	//	return false; 
	//}

	//�ر��ļ�
	fclose(fp);
	this->SetBlockSize();
	return true ;
	
}
long CMyFile::SeriazeMainFile(char szbuf[],long lBufSize)
{
		//�ļ�������
	*(long *)szbuf = m_lFilenameLength  ;
	szbuf+=sizeof(long);
	//�ļ���
	memcpy(szbuf,m_szFileName,m_lFilenameLength);
	szbuf+=m_lFilenameLength;
	//�ļ��Ĵ�С
	*(__int64*)szbuf=m_i64_size  ;
	szbuf+= sizeof(__int64);
	//�ļ���ȫ·������
	 *(long *)szbuf=m_lAllPathLength ;
	szbuf +=sizeof(long);
	//�ļ�ȫ·��
	memcpy(szbuf,m_chAllPath,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	// �Ѿ�д�˵��ֽ���
	*(__int64*)szbuf =m_i64WriteFilePos  ;
	szbuf+= sizeof(__int64);
	return sizeof(long)*2+sizeof(__int64)*2+m_lFilenameLength+m_lAllPathLength;
	

}
long CMyFile::UnSeriazeMainFile(char szbuf[],long lBufSize)
{
	//�ļ�������
	m_lFilenameLength = *(long *)szbuf;
	szbuf+=sizeof(long);
	//�ļ���
	memcpy(m_szFileName,szbuf,m_lFilenameLength);
	szbuf+=m_lFilenameLength;
	//�ļ��Ĵ�С
	m_i64_size = *(__int64*)szbuf;
	szbuf+= sizeof(__int64);
	//�ļ���ȫ·������
	m_lAllPathLength = *(long *)szbuf;
	szbuf +=sizeof(long);
	//�ļ�ȫ·��
	memcpy(m_chAllPath,szbuf,m_lAllPathLength);
	szbuf+=m_lAllPathLength;
	// �Ѿ�д�˵��ֽ���
	m_i64WriteFilePos  =*(__int64*)szbuf;
	szbuf+= sizeof(__int64);
	return sizeof(long)*2+sizeof(__int64)*2+m_lFilenameLength+m_lAllPathLength;
}