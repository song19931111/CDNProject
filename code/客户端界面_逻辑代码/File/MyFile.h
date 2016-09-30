#pragma warning( disable:4996)
#pragma once
#ifndef ____INCLUDE__MYFILE__H____
#define ____INCLUDE__MYFILE__H____
#include <iostream>
#include <windows.h>
#include "md5.h"
using namespace std;
#include "MyLock.h"
#define DEF_ONE_MB  (1048576)
#define DEF_MAX_PATH_SIZE (200)
#define DEF_FILENAME_SIZE (100)
#define DEF_FILEPATH_SIZE (200)
#define DEF_URL_LEN (200)
#ifndef DEF_MD5_LENGTH
#define DEF_MD5_LENGTH (33)  //32位md5处理方式
#endif 
//文件映射，一次写入128MB。避免多次读写
class CMyFile{
public:
	CMyFile(const char *pAllPath);
	CMyFile(const char *pFileName,const char *pPath,__int64 i64_size =  0);  //大小设置为空，代表读取配置文件
	bool InitFile();
	bool InitFileMappingInfo();
	bool UnInitFIle();
	bool SaveBlockInfo();
	bool ReadBlockInfo();
	bool SaveFileInfo();//对主服务器接收的文件的配置进行保存
	bool ReadFileInfo();//对主服务器接收的文件的配置进行读取
	~CMyFile();
public:
	void  SetFileName(char szFileName[]);
	static long lMinLength ; 
	long WriteFile(int m_nPosition,char *pData);
	long CreateInitFile();//创建跟文件大小一样的文件:
	long ReadFile(int m_nPosition,void *pData);
	long WriteFilleEx(__int64 i64FileIndex ,char *pData , long m_lBufLen);
	long ReadFileEx(__int64 i64FileIndex ,char *pData , long m_lReadLen);
	long WriteFileLittleData(char *pData , long m_lBufLen);
	long ReadFileLittleData(char *pData , long m_lBufLen);
	char * md5File(long lMd5Len);
	char * md5FileBlock(int nBlockIndex,long lMd5Leni);
	char * CMyFile::md5String(unsigned char *pData,long lBuflen,long lMd5Len);
	char * ReadOneBlockMd5(int nBlock);//在文件的前几M写入配置文件的情况下
public:
	//Get方法:
	int GetBlockSize()
	{
		return m_nBlockSize;
	}
	int  GetTotalBlock()
	{
		return m_nTotalBlock;
	}
	__int64 GetFileSize()
	{
		return m_i64_size;
	}
	void  SetFileSize(__int64 i64_size)
	{
		m_i64_size  = i64_size;
	}
	//Set
	bool SetBlockInfo(int nBlock,int value )
	{
		if(nBlock>m_nTotalBlock || value==m_pFileBlockInfo[nBlock])
		{
			return false ;
		}
		m_lock_file_block.Lock();
		m_pFileBlockInfo[nBlock]= value;
		m_lock_file_block.UnLock();
		return true; 
	}
	char * GetFileName()
	{
		return m_szFileName;
	}
	int GetBlockInfo(int nIndex)
	{
		if(nIndex>m_nBlockSize-1)
		{
			return -1;
		}
		return m_pFileBlockInfo[nIndex];
	}
	bool isAllBlockHasBeenWritten()
	{
		bool bFlag =true ;
		for(int i=0;i<m_nTotalBlock;i++)
		{
			if(m_pFileBlockInfo[i]<=0)
			{
				bFlag= false; 
			}
		}
		return bFlag;
	}
	
public:
 int m_nHasWrittenBlock;
 int m_nProfileSize; //配置文件的大小
 char *m_pTempCache; //每个文件的临时缓冲区
 long m_lTempCacheLength ;
 	//__int64 m_i64HasWrittenIndex;  //已经写了的字节
private:
	HANDLE m_fileHandle;
	char m_szPath[100];
	long m_lPathLength;  //文件的路径
	char m_szFileName[100];
	long m_lFilenameLength;
	char *m_pFileBlockInfo;
	
	int m_nTotalBlock;
	int m_nBlockSize;  // 块大小
	__int64 m_i64_size;   //文件大小
	__int64 m_i64T; //文件的槛值
	__int64 m_i64WriteT; //写槛值
	__int64 m_i64ReadT; // 读槛值
	/////////////////////////////////
	void * m_Raddr; //进行小数据文件映射保存的地址
	void * m_Waddr; //进行小数据文件映射保存的地址
	bool  m_bLittleReadFlag;
	bool  m_bLittleWriteFlag;
	__int64 m_i64ReadFilePos;
	__int64 m_i64WriteFilePos;
	__int64 m_i64WriteLen;  //已经读的字节
	__int64 m_i64ReadLen;
public :
	__int64 GetWriteFilePos()
	{
		return m_i64WriteFilePos;
	}
///////////////////////////////////
public:
	char m_url[DEF_URL_LEN];
	long m_lUrlLen;
private :
	bool m_bOpenFileMap;
	char m_chAllPath[DEF_MAX_PATH_SIZE];
	long m_lAllPathLength;
	int m_i;
	MyLock m_lock_file_block ;//读写文件块锁
	MyLock m_lock_open_file_mappong ;
	//拼接文件

private :
	//文件映射的参数:
	DWORD m_dwMapSize;
	HANDLE m_handleMap;
private :
	void SetBlockSize();
	long Seriaze(char szbuf[],long lBufSize);
	long UnSeriaze(char szbuf[],long lBufSize);
	long SeriazeMainFile(char szbuf[],long lBufSize);
	long UnSeriazeMainFile(char szbuf[],long lBufSize);
	bool OpenFileMap(int nFileMode);
////////////////////////////////////////////////////////////////////////////////
	 //测
	friend int main();
};
#endif //____INCLUDE__MYFILE__H____