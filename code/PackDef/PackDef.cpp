#include "PackDef.h"
long STRU_LOGIN_RQ::lMinLength =sizeof(int)+3*sizeof(long);//定义最小的长度
long STRU_LOGIN_RS::lMinLength = sizeof(long)+sizeof(int);

long STRU_SEND_FILE_BLOCK_RQ::lMinLength = 5*sizeof(long) +3*sizeof(int)+sizeof(short);

long STRU_SEND_FILE_BLOCK_RS::lMinLength = 5*sizeof(long) +3*sizeof(int)+sizeof(short);


long STRU_GET_BLOCK_IP_RQ::lMinLength = 7 *sizeof(long)+4*sizeof(int )+1*sizeof(__int64);
long STRU_GET_BLOCK_IP_RS::lMinLength = 5 *sizeof(long)+4*sizeof(int )+1*sizeof(__int64)+1*sizeof(short);

long STRU_NODE_SERVER_LOGIN_RQ::m_lMinLength = 4*sizeof(long) +2 *sizeof(int);
long STRU_SEND_NODE_FILE_RQ::m_lMinLength = 1*sizeof(long) +3 *sizeof(int)+sizeof(__int64);
long STRU_SEND_NODE_FILE_RS::m_lMinLength = 2*sizeof(long) +3 *sizeof(int);

long  STRU_REQUEST_MAIN_SERVER_RQ ::m_lMinLength =3 *sizeof(int);
long STRU_SEND_FILE_COMPLETE_RQ::m_lMinLength = 2*sizeof(int )+2*sizeof(long);
long STRU_REQUEST_MAIN_SERVER_RS::m_lMinLength =3 *sizeof(int);

long STRU_NONE_NODE_SERVER_IP_RS::m_lMinLength = 2*sizeof(int)+2*sizeof(long);

long STRU_RECV_DATA_LENGTH::m_lMinLength = 3 *sizeof(int);
long STRU_CLINET_CLOSE::m_lMinLength = 3*sizeof(int);
