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
	 * 构造函数
	 */
	CListUIEx();

	/**
	 * 析构函数
	 */
	virtual ~CListUIEx();
	int InsertItem(CDuiString strFileName);
	int InsertItem(int nItem, int nHeight, CListContainerElementUI *pListItem);
	void SetItemData(int nItem, int nColumn, LPCTSTR Text, LPCTSTR Name);
	void SetItemData(int nItem, int nColumn, CControlUI* pControl);
	/**
	 * 根据索引获取行控件
	 * 
	 * @param	iIndex					行数,从0到最大行数
	 * 
	 * @return	成功时返回子控件地址,否则返回NULL
	 */
	CListContainerElementUI* GetListItem(int iIndex);

	/**
	 * 获取具体位置的控件
	 * 
	 * @param	iIndex					行数,从0到最大行数
	 * @param	iSubIndex					列数,从0到最大行数
	 * 
	 * @return	成功时返回子控件地址,否则返回NULL
	 */
	CHorizontalLayoutUI* GetListSubItem(int iIndex, int iSubIndex);
};
#endif