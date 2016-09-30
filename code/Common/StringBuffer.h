#pragma once 
#ifndef ____INCLUDE__STRINGBUFFER__H____
#define ____INCLUDE__STRINGBUFFER__H____
#include <iostream>
#include "MyLock.h"
#define DEF_MAX_STRINGBUFFER_SIZE (1048576)
#define DEF_MAX_FRONT_LEN (2048)
using namespace std;

enum {enum_only_read=2222,enum_get};
class CStringBuffer{
public:
	CStringBuffer(int nBufferSize = 0);
	char * GetFront(int n_frontLength,int);
	bool addData(char *pData ,int nLength);
	int GetStringBufferSize();
	void Clear();
private :
	char *m_pBuf; 
	char m_arrFrontBuffer[DEF_MAX_FRONT_LEN];
	int m_nHeadIndex;
	int m_nTailIndex; 
	int m_nBufferSize; 
	MyLock lock;

};

#endif //____INCLUDE__STRINGBUFFER__H____