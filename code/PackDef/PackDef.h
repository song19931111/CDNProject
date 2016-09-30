#pragma once 
#ifndef ____INCLUDE__PACKDEF__H____
#include <iostream>
#define DEF_USERNAME_SIZE (50)
#define DEF_PASSWORD_SIZE (50)
#define DEF_FILENAME_LENGTH (100)
#define DEF_PACKAGE_COUNT (1000)
#define DEF_MD5_LENGTH (33)  //32λmd5����ʽ
#define DEF_PROVINCE_SIZE (15)//ʡ��
#ifndef DEF_DISTRACT_LEN
#define DEF_DISTRACT_LEN (60) //��������
#endif
#define DEF_URL_LEN (200)
#ifndef DEF_IP_LEN
#define DEF_IP_LEN (20)
#endif 
#ifndef DEF_IDC_TYPE_LEN
#define DEF_IDC_TYPE_LEN (15)
#endif
//�������������
#define DEF_PACK_START (4000)
//�û���½�Զ���� 
#define DEF_LOGIN_RQ (DEF_PACK_START+1)  
#define DEF_LOGIN_RS (DEF_PACK_START+2)
#define DEF_SEND_FILE_BLOCK_RQ (DEF_PACK_START+3)  //������ͨ����
#define DEF_SEND_FILE_BLOCK_RS (DEF_PACK_START+4)   //������ͨ����
#define DEF_GET_BLOCK_IP_RQ (DEF_PACK_START+5)
#define DEF_GET_BLOCK_IP_RS  (DEF_PACK_START+6)
#define DEF_NODE_SERVER_LOGIN_RQ (DEF_PACK_START+7)
#define DEF_SEND_NODE_FILE_RQ (DEF_PACK_START+8)
#define DEF_SEND_NODE_FILE_RS (DEF_PACK_START+9)
#define DEF_REQUEST_MAIN_SERVER_RQ (DEF_PACK_START+10)
#define  DEF_SEND_FILE_COMPLETE_RQ  (DEF_PACK_START+11)
 #define  DEF_REQUEST_MAIN_SERVER_RS (DEF_PACK_START+12)
#define DEF_NONE_NODE_SERVER_IP_RS  (DEF_PACK_START+13)
#define DEF_RECV_DATA_LENGTH_RS (DEF_PACK_START+14)
#define DEF_CLIENT_CLOSE_RQ (DEF_PACK_START+15)
#define DEF_PACK_END (DEF_PACK_START+DEF_PACKAGE_COUNT)
////////////////////////////////////////////////////////////////////////////////////
#define DEF_SERVER_COMMAND_START (DEF_PACK_END+1)
#define DEF_COMMAND_REQUEST_FILE           DEF_SERVER_COMMAND_START+1
#define DEF_COMMAND_REQUEST_CLOSE       DEF_SERVER_COMMAND_START+2
#define DEF_COMMAND_LOGIN_SUCCESS			DEF_SERVER_COMMAND_START+3

///////////////////////////////////////////////////////////////////////////



//����ÿ�����ṹ�嶼����Ҫ�����ͺ����л��ͷ����еķ���
//������ṹ�����
struct STRU_PACKAGE_BASE{
public:
	int m_nProtocolSize ; //Э����Ĵ�С
	int m_nProtocolType; //Э���������
	virtual long Seriaze(char szbuf[],long lBufLen)=0; //���յ����������л����ṹ������ڽ���
	virtual long UnSeriaze(char szbuf[],long lbufLen)=0; //��Ҫ���͵����ݴӽṹ�巴���л����ַ�������ȥ
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�û���¼��
struct  STRU_LOGIN_RQ :public STRU_PACKAGE_BASE{
private:
	static long lMinLength ;
private:
	char m_szUsername[DEF_USERNAME_SIZE];
	long m_lUsernameLength;
	char m_szPassword[DEF_PASSWORD_SIZE];
	long m_lPasswordLength;
	unsigned long m_ip;
public:
	long Seriaze(char szbuf[],long lBufLen)
	{
		if(lBufLen<lMinLength+DEF_USERNAME_SIZE+DEF_PASSWORD_SIZE||NULL==szbuf)
		{
			return 0;
		}
	//���л��û���
		memcpy(szbuf,m_szUsername,m_lUsernameLength);//ע��/0Ҳ����������
		szbuf+=m_lUsernameLength;
	//���л��û�������
		*(long*)szbuf  = m_lUsernameLength;
		szbuf+=sizeof(m_lUsernameLength);
	//���л�����
		memcpy(szbuf,m_szPassword,m_lPasswordLength);
		szbuf+=m_lPasswordLength;
	//���л����볤��
		*(long*)szbuf = m_lPasswordLength;
		szbuf+=sizeof(m_lPasswordLength);
	//���л�IP:
		*(long*)szbuf = m_ip;
		szbuf+=sizeof(m_ip);
		return lMinLength+m_lUsernameLength+m_lPasswordLength;
	}
	long UnSeriaze(char szbuf[],long lBufLen)
	{
		if(lBufLen<lMinLength+DEF_USERNAME_SIZE+DEF_PASSWORD_SIZE||NULL==szbuf)
		{
			return false;
		}
	//�����л��û���
		memcpy(m_szUsername,szbuf,m_lUsernameLength);//ע��/0Ҳ����������
		szbuf+=m_lUsernameLength;
	//���л��û�������
		m_lUsernameLength=*(long*)szbuf ;
		szbuf+=sizeof(m_lUsernameLength);
	//���л�����
		memcpy(m_szPassword,szbuf,m_lPasswordLength);
		szbuf+=m_lPasswordLength;
	//���л����볤��
		m_lPasswordLength=*(long*)szbuf  ;
		szbuf+=sizeof(m_lPasswordLength);
	//���л�IP:
		m_ip=*(long*)szbuf ;
		szbuf+=sizeof(m_ip);
		return STRU_LOGIN_RQ::lMinLength+m_lUsernameLength+m_lPasswordLength;
	}
};
enum status{enum_login_falied=10000,enum_login_success};

struct STRU_LOGIN_RS :public STRU_PACKAGE_BASE{
private :
	static long lMinLength ;
private:
	long m_status ; //�����û���½��״̬
public :
	long Seriaze(char szbuf[],long lBufLen)
	{
		if(lBufLen<lMinLength)
		{
			return 0;
		}
	//���л�״̬
		*(long*)szbuf = m_status;
		szbuf+=sizeof(long);
		return lMinLength;
	}
	long UnSeriaze(char szbuf[],long lBufLen)
	{
		if(lBufLen<lMinLength||NULL==szbuf)
		{
			return false;
		}
		//�����л�״̬
		m_status  = *(long*)szbuf;
		szbuf+=sizeof(long);
		return STRU_LOGIN_RS::lMinLength;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//��ڵ�����������ļ���,�ڵ��������һ���С�
struct STRU_SEND_FILE_BLOCK_RQ:public STRU_PACKAGE_BASE{
public :
	STRU_SEND_FILE_BLOCK_RQ():m_lBlockSize(0),m_ipServer(0),m_ipUser(0),m_lFileNameLength(0)
	{
		m_nProtocolType = DEF_SEND_FILE_BLOCK_RQ;
		memset(m_szFileName,0,sizeof(m_szFileName));
	}
	static long lMinLength;
	unsigned long m_ipUser;
	unsigned long m_ipServer;
	short m_shPort; 
	long m_lKey;
	long  m_lBlockSize;   //�ļ���Ĵ�С 
	int m_nBlockIndex;  //���
	char m_szFileName[DEF_FILENAME_LENGTH]; //�ļ���
	long m_lFileNameLength ;//�ļ�������

	long m_lFileKey;   //�ļ�key
public:
	long Seriaze(char szbuf[],long lBufsize)
	{
		if(STRU_SEND_FILE_BLOCK_RQ::lMinLength+m_lFileNameLength>lBufsize)
		{
			return false ;
		}
		*(int*)szbuf  = m_nProtocolSize;
		szbuf+=sizeof(int);

		*(int*)szbuf  = m_nProtocolType;
		szbuf+=sizeof(int);

		*(long*)szbuf = m_ipUser;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_ipServer;
		szbuf+=sizeof(long);

		*(short*)szbuf = m_shPort;
		szbuf += sizeof(short);

		*(long*)szbuf = m_lKey;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_lBlockSize;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_nBlockIndex;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_lFileNameLength;
		szbuf+=sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);  //����/0
		szbuf+=m_lFileNameLength;
		
		*(long*)szbuf = m_lFileKey;
		szbuf+=sizeof(long);

		return lMinLength+m_lFileNameLength;
	}
	long UnSeriaze(char szbuf[],long lBufsize)
	{
		//����Ҫ��������С�����ͣ��Ѿ���ǰ����������
		m_ipUser=*(long*)szbuf  ;
		szbuf+=sizeof(long);

		m_ipServer=*(long*)szbuf ;
		szbuf+=sizeof(long);

		m_shPort=*(short*)szbuf ;
		szbuf += sizeof(short);

		m_lKey   = *(long*)szbuf;
		szbuf +=sizeof(long);

		m_lBlockSize=*(long*)szbuf ;
		szbuf+=sizeof(long);

		m_nBlockIndex=*(int*)szbuf;
		szbuf+=sizeof(long);

		m_lFileNameLength=*(long*)szbuf;
		szbuf+=sizeof(long);

		memcpy(m_szFileName,szbuf,m_lFileNameLength);  //����/0
		szbuf+=m_lFileNameLength;
	
		m_lFileKey =*(long*)szbuf  ;
		szbuf+=sizeof(long);

		return lMinLength+m_lFileNameLength;
	}
};

struct STRU_SEND_FILE_BLOCK_RS:public STRU_PACKAGE_BASE{
	//����˵Ļظ���
public :
	STRU_SEND_FILE_BLOCK_RS():m_lBlockSize(0),m_ipServer(0),m_ipUser(0),m_lFileNameLength(0)
	{
		m_nProtocolType = DEF_SEND_FILE_BLOCK_RS;
		memset(m_szFileName,0,sizeof(m_szFileName));
	}
	static long lMinLength;
	unsigned long m_ipUser;
	unsigned long m_ipServer;
	short m_shPort; 
	long m_lKey;
	long  m_lBlockSize;   //�ļ���Ĵ�С 
	int m_nBlockIndex;  //���
	char m_szFileName[DEF_FILENAME_LENGTH]; //�ļ���
	long m_lFileNameLength ;//�ļ�������
	char md5[DEF_MD5_LENGTH];
public:
	long Seriaze(char szbuf[],long lBufsize)
	{
		if(STRU_SEND_FILE_BLOCK_RS::lMinLength+m_lFileNameLength>lBufsize)
		{
			return false ;
		}
		*(int*)szbuf  = m_nProtocolSize;
		szbuf+=sizeof(int);

		*(int*)szbuf  = m_nProtocolType;
		szbuf+=sizeof(int);

		*(long*)szbuf = m_ipUser;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_ipServer;
		szbuf+=sizeof(long);

		*(short*)szbuf = m_shPort;
		szbuf += sizeof(short);

		*(long*)szbuf = m_lKey;
		szbuf+=sizeof(long);

		*(long*)szbuf = m_lBlockSize;
		szbuf+=sizeof(long);

		*(int*)szbuf = m_nBlockIndex;
		szbuf+=sizeof(long);


		*(long*)szbuf = m_lFileNameLength;
		szbuf+=sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);  //����/0
		szbuf+=m_lFileNameLength;
		
		memcpy(szbuf,md5,DEF_MD5_LENGTH);
		szbuf+= DEF_MD5_LENGTH;
		return lMinLength+m_lFileNameLength+DEF_MD5_LENGTH;
	}
	long UnSeriaze(char szbuf[],long lBufsize)
	{
		

		m_ipUser=*(long*)szbuf  ;
		szbuf+=sizeof(long);

		m_ipServer=*(long*)szbuf ;
		szbuf+=sizeof(long);

		m_shPort=*(short*)szbuf ;
		szbuf += sizeof(short);

		m_lKey=*(long*)szbuf ;
		szbuf+=sizeof(long);

		m_lBlockSize=*(long*)szbuf ;
		szbuf+=sizeof(long);

		m_nBlockIndex=*(int*)szbuf;
		szbuf+=sizeof(long);

		m_lFileNameLength=*(long*)szbuf;
		szbuf+=sizeof(long);

		memcpy(m_szFileName,szbuf,m_lFileNameLength);  //����/0
		szbuf+=m_lFileNameLength;
	
		memcpy(md5,szbuf,DEF_MD5_LENGTH);
		szbuf+= DEF_MD5_LENGTH;

		return lMinLength+m_lFileNameLength+DEF_MD5_LENGTH;
	}
};
struct STRU_CONNET_SOCKET_INFO_RQ{
	//����socket����ͨ�����ӽڵ������,��֪��key��ͨ��key�ҵ���Ӧ���ļ���Ϣ,�����ļ�
public :
	//�̶�8�ֽڰ�
	long m_ip;
	long m_lKey;
public :
	long Seriaze(char buf[],int length)
	{
		if(length <sizeof(m_ip)+sizeof(m_lKey))
		{
			return 0 ;
		}
		*(long*)buf = m_ip;
		buf+=sizeof(m_ip);

		*(long*)buf = m_lKey;
		buf+=sizeof(m_lKey);

		return sizeof(m_ip)+sizeof(m_lKey);
	}
	long UnSeriaze(char buf[],int length)
	{
		if(length< sizeof(m_ip)+sizeof(m_lKey))
		{
			return 0;
		}
		m_ip=*(long*)buf ;
		buf+=sizeof(m_ip);

		m_lKey=*(long*)buf ;
		buf+=sizeof(m_lKey);

		return sizeof(m_ip)+sizeof(m_lKey);
	}
};
struct STRU_SEND_CLOSE_RESULT{
	int m_nResult ; 
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//���������������Ľڵ�
struct STRU_NONE_NODE_SERVER_IP_RS:STRU_PACKAGE_BASE{
public:
	static long m_lMinLength ;
	long m_lFileNameLength;
	long m_lFileBlockKey;
	char m_szFileName[DEF_FILENAME_LENGTH];
	long UnSeriaze(char szbuf[],long lBuflen)
	{
		m_lFileNameLength = *(long *)szbuf;	
		szbuf+=sizeof(int);

		m_lFileBlockKey = *(long *)szbuf;	
		szbuf+=sizeof(int);

		memcpy(m_szFileName,szbuf,m_lFileNameLength);
		szbuf+=m_lFileNameLength;

		return m_lFileNameLength+sizeof(int )+sizeof(int);

	}
	long Seriaze(char szbuf[],long lBuflen)
	{
		return 0;
	}



};





struct STRU_GET_BLOCK_IP_RQ:public STRU_PACKAGE_BASE{
public:
	static long lMinLength ;
public:
	long ipServer ;
	long ipClient ;
	//����:
	long m_lDistractLen ;
	long m_lIDCTypeLen ;
	long m_lUrlLen; //URL�ĳ���
	char m_szDistract[DEF_DISTRACT_LEN];
	char m_szIDCType[DEF_IDC_TYPE_LEN];
	char m_szUrl[DEF_URL_LEN];
	long m_lFileNameLength;
	char  m_szFileName[DEF_FILENAME_LENGTH]; //�ļ���
	long m_lKey ;//�ļ���key
	__int64 m_i64FileSize; //�ļ��Ĵ�С
	int m_nBlockIndex ; //���
	int m_nBlockSize; 
		long Seriaze(char szbuf [],long lBufSize)
	{
		if(lBufSize<  STRU_GET_BLOCK_IP_RQ::lMinLength + m_lFileNameLength+m_lDistractLen+m_lIDCTypeLen)
		{
			return 0;
		}
		*(int *)szbuf  = m_nProtocolSize;
		szbuf += sizeof(int );

		*(int *)szbuf  = m_nProtocolType;
		szbuf += sizeof(int );

		*(long *)szbuf  = ipServer;
		szbuf += sizeof(long);

		*(long *)szbuf  = ipClient;
		szbuf += sizeof(long);

		*(long *)szbuf  = m_lDistractLen;
		szbuf += sizeof(long);

		*(long *)szbuf  = m_lIDCTypeLen;
		szbuf += sizeof(long);

		*(long *)szbuf  = m_lUrlLen;
		szbuf += sizeof(long);

		memcpy(szbuf,m_szDistract,m_lDistractLen);
		szbuf+=m_lDistractLen;

		memcpy(szbuf,m_szIDCType,m_lIDCTypeLen);
		szbuf+=m_lIDCTypeLen;

		memcpy(szbuf,m_szUrl,m_lUrlLen);
		szbuf+=m_lUrlLen;


		*(long *)szbuf  = m_lFileNameLength;
		szbuf += sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);
		szbuf += m_lFileNameLength;

		*(long *)szbuf  = m_lKey;
		szbuf += sizeof(long);

		*(__int64*)szbuf +=m_i64FileSize;
		szbuf+=sizeof(__int64);

		*(int *)szbuf  = m_nBlockIndex;
		szbuf += sizeof(int );

		*(int *)szbuf  = m_nBlockSize;
		szbuf += sizeof(int );
		return STRU_GET_BLOCK_IP_RQ::lMinLength + m_lFileNameLength+m_lDistractLen+m_lIDCTypeLen+m_lUrlLen;
	}
		long UnSeriaze(char szbuf [],long lBufSize)
	{ //����Ҫ���
		return 0 ;
	}
};
struct STRU_GET_BLOCK_IP_RS:public STRU_PACKAGE_BASE{
public:
	static long lMinLength ;
public :
	long ipServer ;
	long ipClient ;
	long m_lFileNameLength;
	char m_szFileName[DEF_FILENAME_LENGTH]; //�ļ���
	long m_lKey ;//�ļ���key
	int m_nBlockIndex ; //���
	int m_nBlockSize; 
	long m_lConnectIp;
	short m_shConnectPort ;
	long Seriaze(char szbuf [],long lBufSize)
	{
		if(STRU_GET_BLOCK_IP_RS::lMinLength+m_lFileNameLength>lBufSize)
		{
			return 0 ;
		}
		*(int *)szbuf  = m_nProtocolSize;
		szbuf += sizeof(int );

		*(int *)szbuf  = m_nProtocolType;
		szbuf += sizeof(int );

		*(long *)szbuf  = ipServer;
		szbuf += sizeof(long);

		*(long *)szbuf  = ipClient;
		szbuf += sizeof(long);

		*(long *)szbuf  = m_lFileNameLength;
		szbuf += sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);
		szbuf += m_lFileNameLength;

		*(long *)szbuf  = m_lKey;
		szbuf += sizeof(long);

		*(int *)szbuf  = m_nBlockIndex;
		szbuf += sizeof(int );

		*(int *)szbuf  = m_nBlockSize;
		szbuf += sizeof(int );

		*(long *)szbuf  = m_lConnectIp;
		szbuf += sizeof(long);

		*(short *)szbuf  = m_shConnectPort;
		szbuf += sizeof(long);
		return  STRU_GET_BLOCK_IP_RS::lMinLength +m_lFileNameLength;
	}
		long UnSeriaze(char szbuf [],long lBufSize)
	{
		/*m_nProtocolSize = *(int *)szbuf  ;
		szbuf += sizeof(int );

		m_nProtocolType = *(int *)szbuf   ;
		szbuf += sizeof(int );*/

		ipServer = *(long *)szbuf   ;
		szbuf += sizeof(long);

		ipClient = *(long *)szbuf  ;
		szbuf += sizeof(long);

		m_lFileNameLength =*(long *)szbuf  ;
		szbuf += sizeof(long);

		memcpy(m_szFileName,szbuf,m_lFileNameLength);
		szbuf += m_lFileNameLength;

		m_lKey = *(long *)szbuf  ;
		szbuf += sizeof(long);

		m_nBlockIndex = *(int *)szbuf   ;
		szbuf += sizeof(int );

		m_nBlockSize = *(int *)szbuf  ;
		szbuf += sizeof(int );

		m_lConnectIp = *(long *)szbuf   ;
		szbuf += sizeof(long);

		m_shConnectPort  =*(short *)szbuf   ;
		szbuf += sizeof(long);

		return STRU_GET_BLOCK_IP_RS::lMinLength +m_lFileNameLength;
	}
};
struct STRU_NODE_SERVER_LOGIN_RQ:public STRU_PACKAGE_BASE{
public:
	STRU_NODE_SERVER_LOGIN_RQ()
	{
		memset(m_szDistract,0,sizeof(m_szDistract));
		memset(m_szIDCType,0,sizeof(m_szIDCType));
	}
	static long m_lMinLength ;
	long m_lDistractLength;
	char m_szDistract[DEF_DISTRACT_LEN];  //������������
	long m_lIDCTypeLength ;
	char m_szIDCType[DEF_IDC_TYPE_LEN];//������ͨ�����ȵ�
	unsigned long m_lipNode;
	unsigned long m_lipServer;
	long Seriaze(char szbuf[],long lBufSize)
	{
		if( lBufSize<STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lDistractLength+m_lIDCTypeLength)
		{
			return 0;
		}
		*(int *)szbuf  = m_nProtocolSize;
		szbuf+=sizeof(int);

		*(int *)szbuf  = m_nProtocolType;
		szbuf+=sizeof(int);

		*( long *)szbuf  = m_lDistractLength;
		szbuf+=sizeof(long);

		memcpy(szbuf,m_szDistract,m_lDistractLength);
		szbuf+=m_lDistractLength;

		*(long *)szbuf  = m_lIDCTypeLength;
		szbuf+=sizeof(long);

		memcpy(szbuf,m_szIDCType,m_lIDCTypeLength);
		szbuf+=m_lIDCTypeLength;

		*(unsigned long *)szbuf  = m_lipNode;
		szbuf+=sizeof(long);

		*(unsigned long *)szbuf  = m_lipServer;
		szbuf+=sizeof(long);
		return STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lDistractLength+m_lIDCTypeLength;
	}
	long UnSeriaze(char szbuf[],long lBufSize)
	{
		m_lDistractLength = *(long *)szbuf  ;
		szbuf+=sizeof(long);

		memcpy(m_szDistract,szbuf,m_lDistractLength);
		szbuf+=m_lDistractLength;

		m_lIDCTypeLength = *(long *)szbuf   ;
		szbuf+=sizeof(long);

		memcpy(m_szIDCType,szbuf,m_lIDCTypeLength);
		szbuf+=m_lIDCTypeLength;

		m_lipNode =*(unsigned long *)szbuf   ;
		szbuf+=sizeof(long);

		m_lipServer = *( unsigned long *)szbuf  ;
		szbuf+=sizeof(long);
		return STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lDistractLength+m_lIDCTypeLength;
	}
};
struct STRU_SEND_NODE_FILE_RQ :public STRU_PACKAGE_BASE{
public :
	static long m_lMinLength; 
	long m_lFileNameLength ;// �ļ�������
	char m_szFileName[DEF_FILENAME_LENGTH];
	int m_nMd5BlockSize; //MD5��Ϣ��ռ�Ĵ�С
	__int64 m_i64FileSize ; //�ļ�����ʵ��С
	long Seriaze(char szbuf[],long lBufSize)
	{
		if( lBufSize<STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lFileNameLength)
		{
			return 0;
		}
		*(long *)szbuf  = m_nProtocolSize;
		szbuf +=sizeof(long);

		*(long *)szbuf  = m_nProtocolType;
		szbuf +=sizeof(long);

		*(long *)szbuf  = m_lFileNameLength;
		szbuf +=sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);
		szbuf +=m_lFileNameLength;

		*(int *)szbuf  = m_nMd5BlockSize;
		szbuf +=sizeof(int);

		*(__int64 *)szbuf  = m_i64FileSize;
		szbuf +=sizeof(__int64);
		return STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lFileNameLength;
	}
	long UnSeriaze(char szbuf[],long lBufSize)
	{
		m_lFileNameLength = *(long *)szbuf ;
		szbuf +=sizeof(long);

		memcpy(m_szFileName,szbuf,m_lFileNameLength);
		szbuf +=m_lFileNameLength;

		m_nMd5BlockSize = *(int *)szbuf   ;
		szbuf +=sizeof(int);

		m_i64FileSize  =*(__int64 *)szbuf  ;
		szbuf +=sizeof(__int64);
		return STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength+m_lFileNameLength;
	}
};
struct STRU_SEND_NODE_FILE_RS:public STRU_PACKAGE_BASE{
public :
	static long m_lMinLength; 
	long m_lFileNameLength ;// �ļ�������
	char m_szFileName[DEF_FILENAME_LENGTH];
	int m_nStartByte ;// ���ĸ��ֽڿ�ʼ����
	long m_lFileKey; 
	long Seriaze(char szbuf[],long lBufSize)
	{
		if( lBufSize<STRU_SEND_NODE_FILE_RS::m_lMinLength+m_lFileNameLength)
		{
			return 0;
		}
		*(int *)szbuf  = m_nProtocolSize;
		szbuf +=sizeof(int);

		*(int *)szbuf  = m_nProtocolType;
		szbuf +=sizeof(int);

		*(long *)szbuf  = m_lFileNameLength;
		szbuf +=sizeof(long);

		memcpy(szbuf,m_szFileName,m_lFileNameLength);
		szbuf +=m_lFileNameLength;

		*(long *)szbuf  = m_nStartByte;
		szbuf +=sizeof(long);

		*(long *)szbuf  = m_lFileKey;
		szbuf +=sizeof(long);
		
		return STRU_SEND_NODE_FILE_RS::m_lMinLength+m_lFileNameLength;
	}
	long UnSeriaze(char szbuf[],long lBufSize)
	{
		return 0;
	}
};
struct STRU_RESEND_FILE_RQ{
//����md5��֤���Զ���������������͵İ�:
public :
	long m_lFileNameLength ;// �ļ�������
	char m_szFileName[DEF_FILENAME_LENGTH];
	int m_nStartByte ;
	int m_nEndByte ;
	long Seriaze(char szbuf[],long lBufSize)
	{

	}
	long UnSeriaze(char szbuf[],long lBufSize)
	{
		return 0 ;
	}
};
struct STRU_REQUEST_MAIN_SERVER_RQ :public STRU_PACKAGE_BASE{
public :
	static long m_lMinLength ;
	int m_nCommand ;
	long Seriaze(char szbuf[],long lBufsize)
	{
		if(lBufsize<STRU_REQUEST_MAIN_SERVER_RQ::m_lMinLength)
		{
			return  0;
		}
		*(int *)szbuf =m_nProtocolSize;
		szbuf +=sizeof(int );

		*(int *)szbuf =m_nProtocolType;
		szbuf +=sizeof(int );

		*(int *)szbuf =m_nCommand;
		szbuf +=sizeof(int );

		return m_lMinLength;
	}
	long UnSeriaze(char szbuf[],long lBuflen)
	{
		return 0;
	}
};
struct STRU_REQUEST_MAIN_SERVER_RS :public STRU_PACKAGE_BASE{
public :
	static long m_lMinLength ;
	int m_nCommand ;
	long Seriaze(char szbuf[],long lBufsize)
	{
		if(lBufsize<STRU_REQUEST_MAIN_SERVER_RQ::m_lMinLength)
		{
			return  0;
		}
		*(int *)szbuf =m_nProtocolSize;
		szbuf +=sizeof(int );

		*(int *)szbuf =m_nProtocolType;
		szbuf +=sizeof(int );

		*(int *)szbuf =m_nCommand;
		szbuf +=sizeof(int );

		return m_lMinLength;
	}
	long UnSeriaze(char szbuf[],long lBuflen)
	{

		m_nCommand = *(int *)szbuf ;
		szbuf +=sizeof(int );
		return m_lMinLength;
	}
};
struct STRU_SEND_FILE_COMPLETE_RQ:public STRU_PACKAGE_BASE{
public :
	static long m_lMinLength ; 
	long m_lFileKey ;
	long m_lFileNameLength;
	
	char m_szFileName[DEF_FILENAME_LENGTH];
	

	long Seriaze(char szbuf[],long lBufSize)
	{
		if(lBufSize<m_lMinLength+m_lFileNameLength)
		{
			return  0;
		}
	*(int *)szbuf =m_nProtocolSize;
	szbuf +=sizeof(int );

	*(int *)szbuf =m_nProtocolType;
	szbuf +=sizeof(int );

	*(long *)szbuf =m_lFileKey;
	szbuf +=sizeof(long);


	*(long *)szbuf =m_lFileNameLength;
	szbuf +=sizeof(long);

	memcpy(szbuf,m_szFileName,m_lFileNameLength);
	szbuf +=m_lFileNameLength;
	return m_lMinLength+m_lFileNameLength;
	}
	long UnSeriaze(char szbuf[],long lBufSize)
	{
		return  0;
	}
};
struct  STRU_RECV_DATA_LENGTH:public STRU_PACKAGE_BASE{
static long m_lMinLength ;
int m_nLengthByte;
long Seriaze(char subuf[],long lBuflen)
{
	return 0;
};
long UnSeriaze(char subuf[],long lBuflen)
{
	return 0;
};

};
enum {enum_normal_close,enum_unnormal_close};
struct STRU_CLINET_CLOSE:public STRU_PACKAGE_BASE{
public:
	static long m_lMinLength ; 
	int m_nStatus; //0
	long Seriaze(char szbuf[],long lBuflen)
	{
			*(int *)szbuf =m_nProtocolSize;
		szbuf +=sizeof(int );

		*(int *)szbuf =m_nProtocolType;
		szbuf +=sizeof(int );
		return m_lMinLength;
		
	}
	long UnSeriaze(char szbuf[],long lBuflen)
	{
	
		m_nStatus = *(int *)szbuf ;
		szbuf +=sizeof(int);
		return sizeof(int);
	}

};
#define ____INCLUDE__PACKDEF__H____
#endif	//____INCLUDE__PACKDEF__H____