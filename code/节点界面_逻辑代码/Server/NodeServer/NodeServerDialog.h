#pragma once
#include "../DuiLib/UIlib.h"
using namespace DuiLib;
#pragma comment(lib, "../../../Lib/DuiLib_d.lib")
#include "Kernel.h"
class CNodeServerDialog : public WindowImplBase,public IUIToKernel{
public:
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile() ;
	virtual LPCTSTR GetWindowClassName(void) const ;
	virtual LRESULT CNodeServerDialog::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnClick(TNotifyUI& msg);
	LRESULT  OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public :
	CTextUI *m_pTextUpload;
	CTextUI *m_pTextDownload;
	CTextUI *m_pTextStatus;
	CTextUI * m_pTextClientCount;
	CButtonUI *m_pBtnStart;
	CKernel m_kernel;

public :
	void OnBtnStart();
public://IUIToKernel£º½Ó¿Ú£º
	void NofifyUILoginSuccess();
	void NotifyUIUpdataClientCount(int n) ;
	int NotifyKernelReturnUploadByte();
};