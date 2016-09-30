#include "ErrCode.h"
CErrCode::CErrCode()
{

}
CErrCode::~CErrCode()
{

}
char * CErrCode::GetReSult(int errCode)
{
	switch (errCode)
	{
		case 2001:
			break;
		case enum_para_error :
			break;
	}
	return "aaa";
}