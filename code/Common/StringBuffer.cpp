#include "StringBuffer.h"
CStringBuffer::CStringBuffer(int nBufSize):m_pBuf(NULL),m_nHeadIndex(0),m_nTailIndex(0)
{
	
	if(0  ==nBufSize||nBufSize<0)
	{
		m_nBufferSize= DEF_MAX_STRINGBUFFER_SIZE;
	}
	else
	{
		m_nBufferSize =  nBufSize;
	}
	m_pBuf  =new char[m_nBufferSize+1];
	memset(m_pBuf,0,m_nBufferSize+1);
	memset(m_arrFrontBuffer,0,sizeof(m_arrFrontBuffer));
}

char * CStringBuffer::GetFront(int n_frontLength,int mode)
{
	//两种模式：1、只看不取2。取
	if(mode!=enum_only_read&&mode!=enum_get)
	{
		return NULL;
	}
	//判断参数

	if(n_frontLength<=0||n_frontLength>m_nBufferSize)
	{
		return NULL;
	}
	lock.Lock(); 
		//判断缓存是否是空//判断是否有足够大的缓存可以取
	if(m_nHeadIndex==m_nTailIndex || n_frontLength>GetStringBufferSize())
	{
		lock.UnLock(); 
		return NULL;
	}
	
	//拷贝新的Front 
	memcpy(m_arrFrontBuffer,m_pBuf+m_nHeadIndex,n_frontLength);
	if(mode ==enum_get)
	{
		m_nHeadIndex = (m_nHeadIndex+n_frontLength)%(m_nBufferSize+1);
		//否则指针不偏移
	}
	
	lock.UnLock();
	return m_arrFrontBuffer;
}
bool CStringBuffer::addData(char *pData ,int nLength)
{
	//判断参数,//判断缓存是否满了
	lock.Lock();
	if(pData==NULL||nLength>m_nBufferSize-GetStringBufferSize())
	{
		lock.UnLock();
		return false ;
	}
	//添加字符串
	
	memcpy(m_pBuf+m_nTailIndex,pData,nLength);
	m_nTailIndex = (m_nTailIndex+nLength+m_nBufferSize+1)%(m_nBufferSize+1);
	lock.UnLock();
	return true ;

}

int CStringBuffer::GetStringBufferSize()
{
	
	int nSize =  (m_nTailIndex-m_nHeadIndex+m_nBufferSize+1)%(m_nBufferSize+1);
	return nSize;
}