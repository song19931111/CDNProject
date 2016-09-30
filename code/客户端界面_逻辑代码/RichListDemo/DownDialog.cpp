#include "DownDialog.h"
DUI_BEGIN_MESSAGE_MAP(CDownDialog, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

void CDownDialog::InitWindow()
{
	m_pOK = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btn_ok")));
	m_pCancel = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btn_cancel")));
	m_pEditUrl = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("TextUrl")));
	m_pPath = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("TextPath")));
	m_pEditUrl->SetText("https://www.python.org/ftp/python/2.7.11/python-2.7.11.amd64.msi");
	m_pPath->SetText("./");
}
void CDownDialog::Init(IRichListWndToDownloadDialolg *pRichList)
{
	m_pRichList = pRichList;
}
void CDownDialog::Notify( TNotifyUI &msg )
{
	WindowImplBase::Notify(msg);
}
void CDownDialog::OnClick(TNotifyUI& msg)
{
	if(msg.pSender ==m_pOK)
	{
		//MessageBox(m_hWnd,"OK","提示",MB_OK);
		CDuiString strUrl = m_pEditUrl->GetText();
		CDuiString strPath = m_pPath->GetText();
		m_pRichList->NotifyRichListOk(strUrl,strPath);
		this->Close();
	}
	if(msg.pSender ==m_pCancel)
	{
		this->Close();
		//MessageBox(m_hWnd,"Canc","提示",MB_OK);
	}
}