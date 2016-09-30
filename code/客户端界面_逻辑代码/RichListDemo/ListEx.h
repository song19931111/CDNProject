#pragma once
#ifndef LISTEX_H
#define LISTEX_H

#include "MyLock.h"

class CListContainerElementUIEx;
class CListUIEx;
#include "..\DuiLib\UIlib.h"
using namespace DuiLib;
#include <map>
using namespace std;
struct STRU_LIST_CONTROL{
	CButtonUI m_button;
	CLabelUI m_label[4];
	CProgressUI m_progess;
};
class CListUIEx : public CListUI
{
public :
	
public:
	/**
	 * ���캯��
	 */
	CListUIEx();

	/**
	 * ��������
	 */
	virtual ~CListUIEx();
	int InsertItem(CDuiString strFileName);
	int InsertItem(int nItem, int nHeight, CListContainerElementUI *pListItem);
	void SetItemData(int nItem, int nColumn, LPCTSTR Text, LPCTSTR Name);
	void SetItemData(int nItem, int nColumn, CControlUI* pControl);
	/**
	 * ����������ȡ�пؼ�
	 * 
	 * @param	iIndex					����,��0���������
	 * 
	 * @return	�ɹ�ʱ�����ӿؼ���ַ,���򷵻�NULL
	 */
	CListContainerElementUI* GetListItem(int iIndex);

	/**
	 * ��ȡ����λ�õĿؼ�
	 * 
	 * @param	iIndex					����,��0���������
	 * @param	iSubIndex					����,��0���������
	 * 
	 * @return	�ɹ�ʱ�����ӿؼ���ַ,���򷵻�NULL
	 */
	CHorizontalLayoutUI* GetListSubItem(int iIndex, int iSubIndex);
};
#endif