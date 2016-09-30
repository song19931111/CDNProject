#pragma once


//////////////////////////////////////////////////////////////////////////
///
#include "ListEx.h"
#include"DownDialog.h"
#include "Kernel.h"
#define DEF_TIMER_ID_UPDATE_SPPEND 10002
class CPage1 : public CNotifyPump
{
public:
	CPage1();
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemClick( TNotifyUI &msg );
private:
	CPaintManagerUI* m_pPaintManager;
};


//////////////////////////////////////////////////////////////////////////
///

class CPage2 : public CNotifyPump
{
public:
	CPage2();
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemClick( TNotifyUI &msg );
private:
	CPaintManagerUI* m_pPaintManager;
};

//////////////////////////////////////////////////////////////////////////
///

class CRichListWnd : public WindowImplBase,public IRichListWndToDownloadDialolg,public IUIToKerel
{
public:
	CRichListWnd(void);
	~CRichListWnd(void);

	virtual void OnFinalMessage( HWND );
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName( void ) const;
	virtual void Notify( TNotifyUI &msg );
	virtual LRESULT OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual void InitWindow();
	virtual LRESULT OnMouseHover( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual LRESULT OnChar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CDuiString GetZIPFileName() const;
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT  OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemClick( TNotifyUI &msg );

private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI *m_pAddBtn;
	CPage1 m_Page1;
	CPage2 m_Page2;
	CListUIEx *m_pList; 
	CListUI *m_pListTip;
private :
	void AddNewUrlTask();
	void NotifyRichListOk(CDuiString strUrl,CDuiString strPath);
	void  NofifyUITip(string strTip);

	//void NotifyUIUpdateCompleteFile(char *pFile,__int64 i64Size);
   
	void NofityUIFileInfo(string strFileName,__int64 fileSize,int HasWrittenByte);
private :
	CKernel m_kernel;
};
