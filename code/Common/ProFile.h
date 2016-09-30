#pragma once
#ifndef ____INCLUDE__PROFILE__H____
#define ____INCLUDE__PROFILE__H____
#ifndef DEF_FILENAME_LENGTH 
#define DEF_FILENAME_LENGTH 200
#define DEF_FILEPATH_LENGTH 200
#endif 
#include <list>
#include "io.h"
using namespace std;
struct STRU_WRITE_FILE_INFO{};
struct STRU_FILE_NODE{
	char m_szFileName[200];
	long m_lFileNameLength ;
	
};
class CProFile{
private :
	char  m_szAllPath[200];
	int m_nFileCount ;
public :
	list<STRU_FILE_NODE *>m_ls_flie_node;
public :
	CProFile();
    bool InitProfile(char szPath[]);
	bool ReadProfile(); //∂¡»°≈‰÷√Œƒº˛
	bool UpdateProfile(char szFileName[]);
	bool DelProfile(char szFileName[]);
	void UpDateAllProfile();
};
#endif