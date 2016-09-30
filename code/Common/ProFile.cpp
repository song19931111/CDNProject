#include "ProFile.h"
CProFile::CProFile()
{

}
bool CProFile::InitProfile(char *pAllPath)
{

	memcpy(m_szAllPath,pAllPath,strlen(pAllPath)+1);
	m_nFileCount = 0;
	/*if(NULL ==pAllPath)
	{
		return false;
	}
	FILE *fp;
	fopen_s(&fp,pAllPath,"wb");
	fclose(fp);*/
	return true;
}
bool CProFile::ReadProfile() //读取配置文件
{
	FILE *fp =NULL;
	fopen_s(&fp,m_szAllPath,"rb");
	char szReadFilelLen[4];
	char szInt[4];
	if(fp==NULL)
	{
		return  false;
	}
	fseek(fp,0,SEEK_SET);

		 if(-1== fread(szInt,4,1,fp))
		 {
			return  false;
		 }
		 int nCount  =*(int *)szInt;
		 for(int i=0;i<nCount ;i++)
		 {
			   if(-1 ==fread(szReadFilelLen,4,1,fp))
			   {
				   fclose(fp);
				   return false;
			   }
				STRU_FILE_NODE *pNode = new STRU_FILE_NODE;
				pNode->m_lFileNameLength=*(long*)szReadFilelLen;
				 if(-1 == fread(pNode->m_szFileName,pNode->m_lFileNameLength,1,fp))
				 {
					   fclose(fp);
						return false;
				 }
				m_ls_flie_node.push_back(pNode);
				fflush(fp);
		 }
	fclose(fp);
	return true ;
}
bool CProFile::UpdateProfile(char szFileName[])
{
	
	STRU_FILE_NODE *pNode = new STRU_FILE_NODE;
	strcpy_s(pNode->m_szFileName,DEF_FILENAME_LENGTH,szFileName);
	pNode->m_lFileNameLength =strlen(pNode->m_szFileName)+1;
	char szWrite[300] = {0};
	char * pTemp = szWrite;
	*(int*)pTemp = pNode->m_lFileNameLength;
	pTemp+=sizeof(int);
	memcpy(pTemp,pNode->m_szFileName,pNode->m_lFileNameLength);
	pTemp+=pNode->m_lFileNameLength;
	FILE *fp = NULL;
	char szInt[4];
	
	if(_access(m_szAllPath,00)==-1)
	{
		fopen_s(&fp,m_szAllPath,"a+");
		*(int *)szInt  =1;
		 fwrite(szInt,4,1,fp);
	}
	else
	{
		fopen_s(&fp,m_szAllPath,"a+");
		fseek(fp,0,SEEK_SET);
		fread(szInt,4,1,fp);
		fflush(fp);
		(*(int *)szInt)++;
		fseek(fp,0,SEEK_SET);
		fwrite(szInt,4,1,fp);
	}
	//if(-1 ==_access(m_szAllPath,00))
	//{
	// //不存在文件：
	//	fopen_s(&fp,m_szAllPath,"a+");
	//
	//}
	//fopen_s(&fp,m_szAllPath,"a+");
	
	fseek(fp,0,SEEK_END);
	fwrite(szWrite,sizeof(int)+pNode->m_lFileNameLength,1,fp);
	//fputc(EOF,fp);
	fclose(fp);
	//更新list
	return true;
}
bool CProFile::DelProfile(char szFileName[])
{
	//查找
	list<STRU_FILE_NODE *>::iterator ite= m_ls_flie_node.begin();
	while(ite !=m_ls_flie_node.end())
	{
		if(0 == strcmp((*ite)->m_szFileName,szFileName))
		{
			ite = m_ls_flie_node.erase(ite);

			//找到了，更新整个文件: 
		}
		break ;
	}
	UpDateAllProfile();
	return true;
}
void CProFile::UpDateAllProfile()
{
	list<STRU_FILE_NODE *>::iterator ite= m_ls_flie_node.begin();
	int size = m_ls_flie_node.size();
	if(0 == size)
	{
		return ;
	}
	char *pWriteBuffer =  new char[size*200];
	char *pTemp = pWriteBuffer;
	
	while(ite !=m_ls_flie_node.end())
	{
		*(int *)pTemp =(*ite)->m_lFileNameLength;
		pTemp +=sizeof(int);
		memcpy(pTemp,(*ite)->m_szFileName,(*ite)->m_lFileNameLength);
		pTemp+=(*ite)->m_lFileNameLength;
		ite++;
	}
	int nWriteLen = (pTemp-pWriteBuffer)/sizeof(char);
	FILE *fp = NULL;
	fopen_s(&fp,m_szAllPath,"wb");
	if(fp==NULL)
	{
		return  ;
	}
	fwrite(pWriteBuffer,nWriteLen,1,fp);
	fclose(fp);
	delete [] pWriteBuffer;
}