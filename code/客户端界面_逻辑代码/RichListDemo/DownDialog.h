#pragma once
#include "..\DuiLib\UIlib.h"
using namespace DuiLib;
class IRichListWndToDownloadDialolg{
public :
	virtual void NotifyRichListOk(CDuiString strUrl,CDuiString strPath) = 0;
};
class CDownDialog : public WindowImplBase
{
public :
	void Init(IRichListWndToDownloadDialolg *pRichList);
	virtual CDuiString GetSkinFolder()
	{
		return _T("skin\\RichListRes\\");
	}
	virtual CDuiString GetSkinFile() 
	{
		return _T("DownDialog.xml");
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return _T("DownDialog");
	}
	virtual void Notify( TNotifyUI &msg );
	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void InitWindow();
	void SetPaintManager(CPaintManagerUI* pPaintManager )
	{
		//m_pPaintManager = pPaintManager;
	}
private:
	//CPaintManagerUI* m_pPaintManager;
	CButtonUI *m_pOK;
	CButtonUI *m_pCancel ;
	CEditUI *m_pEditUrl;
	CEditUI *m_pPath ;
	IRichListWndToDownloadDialolg *m_pRichList;
};