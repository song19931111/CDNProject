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
	//����ģʽ��1��ֻ����ȡ2��ȡ
	if(mode!=enum_only_read&&mode!=enum_get)
	{
		return NULL;
	}
	//�жϲ���

	if(n_frontLength<=0||n_frontLength>m_nBufferSize)
	{
		return NULL;
	}
	lock.Lock(); 
		//�жϻ����Ƿ��ǿ�//�ж��Ƿ����㹻��Ļ������ȡ
	if(m_nHeadIndex==m_nTailIndex || n_frontLength>GetStringBufferSize())
	{
		lock.UnLock(); 
		return NULL;
	}
	
	//�����µ�Front 
	memcpy(m_arrFrontBuffer,m_pBuf+m_nHeadIndex,n_frontLength);
	if(mode ==enum_get)
	{
		m_nHeadIndex = (m_nHeadIndex+n_frontLength)%(m_nBufferSize+1);
		//����ָ�벻ƫ��
	}
	
	lock.UnLock();
	return m_arrFrontBuffer;
}
bool CStringBuffer::addData(char *pData ,int nLength)
{
	//�жϲ���,//�жϻ����Ƿ�����
	lock.Lock();
	if(pData==NULL||nLength>m_nBufferSize-GetStringBufferSize())
	{
		lock.UnLock();
		return false ;
	}
	//����ַ���
	
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