#define WIN32_LEAN_AND_MEAN
#include "NodeServerDialog.h"
#define DEF_TIMER_ID_UPDATE_SPPEND 10001
void CNodeServerDialog:: OnClick(TNotifyUI& msg)
{
	if(msg.pSender ==m_pBtnStart)
	{
		OnBtnStart();
	}
}
void CNodeServerDialog::InitWindow()
{
	//HWND hwnd = m_PaintManager.GetPaintWindow();
    //SetTimer(hwnd, 1, 1000,    NULL);
	m_pTextUpload  = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("uploadText")));
	m_pTextDownload  = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("downLoadText")));
	m_pTextStatus = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("statusText")));
	m_pBtnStart  = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_start")));
	m_pTextClientCount  =static_cast<CTextUI*>(m_PaintManager.FindControl(_T("clientCountText"))); 
	m_kernel.OpenKernel(this);
	HWND hwnd = m_PaintManager.GetPaintWindow();
    SetTimer(hwnd, DEF_TIMER_ID_UPDATE_SPPEND, 1000,    NULL);

}
CDuiString CNodeServerDialog:: GetSkinFolder()
{
	return _T("skin\\");
}
CDuiString CNodeServerDialog:: GetSkinFile()
{
	return _T("node.xml");
}
LPCTSTR CNodeServerDialog:: GetWindowClassName(void) const
{
			return _T("NodeServer");
} 
LRESULT  CNodeServerDialog::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam == DEF_TIMER_ID_UPDATE_SPPEND)
	{
		int nDownloadByte  =m_kernel.NotifyKernelReturnDownloadByte();
		int nUploadByte  =m_kernel.NotifyKernelReturnUploadByte();
		double dDownloadByte   = 0.0;
		double  dUploadByte =0.0;
//		char szbuf[50];
		CDuiString strUpload;
		CDuiString strDownLoad;
		if(nUploadByte>1024&&nUploadByte<1024*1024)
		{
			nUploadByte/=1024;
			strUpload.Format("%dKB",nUploadByte);
		}
		else if(nUploadByte<1024)
		{
			strUpload.Format("%dB",nUploadByte);
		}
		else
		{
			dUploadByte = double(nUploadByte)/(1024*1024);
			strUpload.Format("%.1lfMB",dUploadByte);
		}

		if(nDownloadByte>1024&&nDownloadByte<1024*1024)
		{
			nDownloadByte/=1024;
			strDownLoad.Format("%dKB",nDownloadByte);
		}
		else if(nDownloadByte<1024)
		{
			strDownLoad.Format("%dB",nDownloadByte);
		}
		else
		{
			dDownloadByte = double(nDownloadByte)/(1024*1024);
			strDownLoad.Format("%.1lfMB",dDownloadByte);
		}
		m_pTextDownload->SetText(strDownLoad);
		m_pTextUpload->SetText(strUpload);
	}
	//MessageBox(m_hWnd,"定时器","定时器",MB_OK);
	return 1L;
}
LRESULT CNodeServerDialog::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 {
        LRESULT lRes = 0;
        switch (uMsg)
        {
        case WM_TIMER: lRes = OnTimer(uMsg, wParam, lParam, bHandled); break;
        }
        //bHandled = FALSE;
        
		return WindowImplBase::HandleCustomMessage(uMsg,wParam,lParam,bHandled);
 }
void CNodeServerDialog::OnBtnStart()
{
	//HWND hwnd = m_PaintManager.GetPaintWindow();
  //  SetTimer(hwnd, 1, 1000,    NULL);
	m_kernel.NofityKernelSendFileRequest();
}
void CNodeServerDialog::NofifyUILoginSuccess()
{
	m_pTextStatus->SetText("登录主服务器成功");
}
void  CNodeServerDialog::NotifyUIUpdataClientCount(int n) 
{
	CDuiString str;
	str.Format("客户端数量:%d",n);
	m_pTextClientCount->SetText(str);
}