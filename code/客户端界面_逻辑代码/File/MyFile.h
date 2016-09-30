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
#define DEF_MD5_LENGTH (33)  //32λmd5����ʽ
#endif 
//�ļ�ӳ�䣬һ��д��128MB�������ζ�д
class CMyFile{
public:
	CMyFile(const char *pAllPath);
	CMyFile(const char *pFileName,const char *pPath,__int64 i64_size =  0);  //��С����Ϊ�գ������ȡ�����ļ�
	bool InitFile();
	bool InitFileMappingInfo();
	bool UnInitFIle();
	bool SaveBlockInfo();
	bool ReadBlockInfo();
	bool SaveFileInfo();//�������������յ��ļ������ý��б���
	bool ReadFileInfo();//�������������յ��ļ������ý��ж�ȡ
	~CMyFile();
public:
	void  SetFileName(char szFileName[]);
	static long lMinLength ; 
	long WriteFile(int m_nPosition,char *pData);
	long CreateInitFile();//�������ļ���Сһ�����ļ�:
	long ReadFile(int m_nPosition,void *pData);
	long WriteFilleEx(__int64 i64FileIndex ,char *pData , long m_lBufLen);
	long ReadFileEx(__int64 i64FileIndex ,char *pData , long m_lReadLen);
	long WriteFileLittleData(char *pData , long m_lBufLen);
	long ReadFileLittleData(char *pData , long m_lBufLen);
	char * md5File(long lMd5Len);
	char * md5FileBlock(int nBlockIndex,long lMd5Leni);
	char * CMyFile::md5String(unsigned char *pData,long lBuflen,long lMd5Len);
	char * ReadOneBlockMd5(int nBlock);//���ļ���ǰ��Mд�������ļ��������
public:
	//Get����:
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
 int m_nProfileSize; //�����ļ��Ĵ�С
 char *m_pTempCache; //ÿ���ļ�����ʱ������
 long m_lTempCacheLength ;
 	//__int64 m_i64HasWrittenIndex;  //�Ѿ�д�˵��ֽ�
private:
	HANDLE m_fileHandle;
	char m_szPath[100];
	long m_lPathLength;  //�ļ���·��
	char m_szFileName[100];
	long m_lFilenameLength;
	char *m_pFileBlockInfo;
	
	int m_nTotalBlock;
	int m_nBlockSize;  // ���С
	__int64 m_i64_size;   //�ļ���С
	__int64 m_i64T; //�ļ��ļ�ֵ
	__int64 m_i64WriteT; //д��ֵ
	__int64 m_i64ReadT; // ����ֵ
	/////////////////////////////////
	void * m_Raddr; //����С�����ļ�ӳ�䱣��ĵ�ַ
	void * m_Waddr; //����С�����ļ�ӳ�䱣��ĵ�ַ
	bool  m_bLittleReadFlag;
	bool  m_bLittleWriteFlag;
	__int64 m_i64ReadFilePos;
	__int64 m_i64WriteFilePos;
	__int64 m_i64WriteLen;  //�Ѿ������ֽ�
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
	MyLock m_lock_file_block ;//��д�ļ�����
	MyLock m_lock_open_file_mappong ;
	//ƴ���ļ�

private :
	//�ļ�ӳ��Ĳ���:
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
	 //��
	friend int main();
};
#endif //____INCLUDE__MYFILE__H____