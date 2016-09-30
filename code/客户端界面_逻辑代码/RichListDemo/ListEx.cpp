#include "ListEx.h"
map<CDuiString ,STRU_LIST_CONTROL*>m_mp_control;//每一项的控件
MyLock m_lock_control;
CListUIEx::CListUIEx()
{
}

CListUIEx::~CListUIEx()
{
}
int CListUIEx::InsertItem(CDuiString strFileName)
{
	
	STRU_LIST_CONTROL *pControlSet = new STRU_LIST_CONTROL;
	m_lock_control.Lock();
	m_mp_control[strFileName] = pControlSet;
	m_lock_control.UnLock();
	CListContainerElementUI *pListItem = new CListContainerElementUI;
	//pListItem->SetFixedHeight(nHeight);/*固定一个行高*/
	pListItem->SetAttribute("height","45");
	CHorizontalLayoutUI *pHorizontal  =new CHorizontalLayoutUI;
	pHorizontal->SetAttribute("bkcolor","#006B93B2");
	pHorizontal->SetAttribute("float","false");
	pHorizontal->SetAttribute("childpadding","10");
	pHorizontal->SetAttribute("inset","3,5,3,5");
	pControlSet->m_button.SetAttribute("float","false");
	pControlSet->m_button.SetAttribute("bordersize","0");
	pControlSet->m_button.SetAttribute("width","32");
	pControlSet->m_button.SetAttribute("bkcolor2","FF757676");
	pControlSet->m_button.SetAttribute("bordercolor","#FF000000");
	pControlSet->m_button.SetAttribute("maxheight","26");
	pControlSet->m_button.SetAttribute("bkimage","downlist_app.png");
	pControlSet->m_button.SetAttribute("normalimage","file='downlist_ok.png' dest='20,14,32,26'");
	pControlSet->m_button.SetAttribute("hotimage","file='downlist_pause.png' dest='15,9,32,26");
	//<Progress name="down_progress" float="false" foreimage="file='progress_fore.png'" min="0" max="100" value="100" maxheight="7" maxwidth="200"/>
/////////////////////////////////////////////
	CVerticalLayoutUI *pVertical_1 = new CVerticalLayoutUI;
	pControlSet->m_label[0].SetAttribute("name","down_name");
	pControlSet->m_label[0].SetAttribute("text",strFileName);
	pControlSet->m_label[0].SetAttribute("textcolor","#FFAAAAAA");
	pControlSet->m_label[0].SetAttribute("showhtml","true");
	pVertical_1->Add(&pControlSet->m_label[0]);
	pControlSet->m_progess.SetAttribute("name","down_progress");
	pControlSet->m_progess.SetAttribute("float","false");
	pControlSet->m_progess.SetAttribute("foreimage","progress_fore.png");
	pControlSet->m_progess.SetAttribute("min","0");
	pControlSet->m_progess.SetAttribute("max","100");
	pControlSet->m_progess.SetAttribute("value","100");
	pControlSet->m_progess.SetAttribute("maxheight","7");
	pControlSet->m_progess.SetAttribute("maxwidth","200");
		pControlSet->m_progess.SetAttribute("Visable","True");
	pVertical_1->Add(&pControlSet->m_progess);
/////////////////////////////////////////////////////////////////////////////
	CVerticalLayoutUI *pVertical_2 = new CVerticalLayoutUI;
	pVertical_2->SetAttribute("width","80");
	pControlSet->m_label[1].SetAttribute("name","down_size");
	pControlSet->m_label[1].SetAttribute("align","right");
	pControlSet->m_label[1].SetAttribute("text","0M");
	pControlSet->m_label[1].SetAttribute("textcolor","#FFAAAAAA");
	pControlSet->m_label[1].SetAttribute("showhtml","#true");
	CHorizontalLayoutUI *pHorizontal_1  =new CHorizontalLayoutUI;
	pControlSet->m_label[2].SetAttribute("name","down_http_speed");
	pControlSet->m_label[2].SetAttribute("text","0KB");
	pControlSet->m_label[2].SetAttribute("textcolor","FFAAAAAA");
	pControlSet->m_label[2].SetAttribute("showhtml","true");
	
	pControlSet->m_label[3].SetAttribute("name","down_node_speed");
	pControlSet->m_label[3].SetAttribute("text","0KB");
	pControlSet->m_label[3].SetAttribute("textcolor","FFAAAAAA");
	pControlSet->m_label[3].SetAttribute("showhtml","true");
	pHorizontal_1->Add(&pControlSet->m_label[2]);
	pHorizontal_1->Add(&pControlSet->m_label[3]);
	pVertical_2->Add(&pControlSet->m_label[1]);
	pVertical_2->Add(pHorizontal_1);
	//<Label name="down_size" align="right" text="2.04M " textcolor="#FFAAAAAA" showhtml="true"></Label>
/////////////////////////////////////////////////////////////////////////////

	pHorizontal->Add(&pControlSet->m_button);
	pHorizontal->Add(pVertical_1);
	//pHorizontal->Add(&pControlSet->m_progess);
	pHorizontal->Add(pVertical_2);
	pListItem->Add(pHorizontal);
	/*pListItem->m_pHeader = CListUI::GetHeader();
	if (NULL != pListItem->m_pHeader)
	{
		int nHeaderCount = pListItem->m_pHeader->GetCount();
		for (int i = 0; i < nHeaderCount; i++)
		{
			pListItem->Add(new CHorizontalLayoutUI);
		}
	}*/
	if ( !CListUI::AddAt(pListItem, 0) )
	{
		delete pListItem;
		pListItem = NULL;
		return -1;
	}
	return 0;
}


int CListUIEx::InsertItem(int nItem, int nHeight, CListContainerElementUI *pListItem)
{
	pListItem->SetFixedHeight(nHeight);	
	//pListItem->m_pHeader =  CListUI::GetHeader();
	if ( !CListUI::AddAt(pListItem, nItem) )
	{
		delete pListItem;
		pListItem = NULL;
		return -1;
	}
	
	return nItem;
}

void CListUIEx::SetItemData(int nItem,
							int nColumn,
							LPCTSTR Text, LPCTSTR Name)
{
	CLabelUI *pLabel = new CLabelUI;
	pLabel->SetText(Text);//控件属性就根据需求设置吧,我简单设置一下
	pLabel->SetTextStyle(DT_CENTER);
	pLabel->SetAttribute("endellipsis", "true");
	pLabel->SetName(Name);
	SetItemData(nItem, nColumn, pLabel);//添加到父控件
}

void CListUIEx::SetItemData(int nItem, int nColumn,	CControlUI* pControl)
{
	CHorizontalLayoutUI *pSubHor = GetListSubItem(nItem, nColumn);
	pSubHor->SetAttribute("inset", "3,1,3,1");
	pSubHor->Add(pControl);//添加到父控件
}

CListContainerElementUI* CListUIEx::GetListItem(int iIndex)
{
	return static_cast<CListContainerElementUI*>(CListUI::GetItemAt(iIndex));
}

CHorizontalLayoutUI* CListUIEx::GetListSubItem(int iIndex, int iSubIndex)
{
	//获取具体行控件
	CListContainerElementUI *pListItem = static_cast<CListContainerElementUI*>(CListUI::GetItemAt(iIndex));
	if (pListItem == NULL)
		return NULL;
	return static_cast<CHorizontalLayoutUI*>(pListItem->GetItemAt(iSubIndex));
}
