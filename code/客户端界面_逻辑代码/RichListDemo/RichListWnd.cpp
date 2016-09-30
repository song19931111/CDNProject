#include "StdAfx.h"
#include "RichListWnd.h"
#include "Kernel.h"
extern MyLock m_lock_control;
extern map<CDuiString ,STRU_LIST_CONTROL*>m_mp_control;
//////////////////////////////////////////////////////////////////////////
///


DUI_BEGIN_MESSAGE_MAP(CPage1, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK,OnItemClick)
DUI_END_MESSAGE_MAP()

CPage1::CPage1()
{
	m_pPaintManager = NULL;
}

void CPage1::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	m_pPaintManager = pPaintMgr;
}

void CPage1::OnClick(TNotifyUI& msg)
{
	if(msg.pSender->GetName() == _T("down_ico"))
	{                
		CControlUI *find_ctrl =m_pPaintManager->FindSubControlByName(msg.pSender->GetParent()->GetParent(), _T("down_name"));

		if(find_ctrl)
		{
			MessageBox(NULL, 
				find_ctrl->GetText()+_T(" 演示未选中行中的按钮触发动作，依该按钮父结点的找到所属行listcontainer.."), 
				_T("DUILIB DEMO"), MB_OK);   
			((CLabelUI *)find_ctrl)->SetText(_T("由程序动态设置后的名称..."));
		}
		}
	else if(msg.pSender->GetName() == _T("down_del"))
	{
		CListUI *down_list = 
			static_cast<CListUI*>(m_pPaintManager->FindControl(_T("down_list_tab")));
		if(!down_list)
			return;

		down_list->RemoveAt(down_list->GetCurSel());                   
	}
	else if(msg.pSender->GetName() == _T("down_new"))
	{
		CListUI *down_list = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("down_list_tab")));
		if(!down_list)
			return;

		CListContainerElementUI *new_node = new CListContainerElementUI;
		new_node->ApplyAttributeList(_T("height=\"45\""));

		CHorizontalLayoutUI *new_h_lay = new CHorizontalLayoutUI;
		new_h_lay->ApplyAttributeList(_T("float=\"false\" ")\
			_T("childpadding=\"10\" inset=\"3,5,3,5\""));

		CButtonUI *new_btn_1 = new CButtonUI;
		new_btn_1->ApplyAttributeList(
			_T("name=\"down_ico\" float=\"false\" ")\
			_T("bordersize=\"0\" width=\"32\" maxheight=\"26\" ")\
			_T("bkimage=\"downlist_app.png\" ")\
			_T("normalimage=\"file='downlist_run.png' dest='20,14,32,26'\""));

		CVerticalLayoutUI *new_v_lay = new CVerticalLayoutUI;
		new_h_lay->Add(new_btn_1);
		new_h_lay->Add(new_v_lay);

		CLabelUI *new_label = new CLabelUI;
		new_label->ApplyAttributeList(_T("textcolor=\"#FFAAAAAA\" showhtml=\"true\""));
		new_label->SetText(_T("new added item.exe"));
		new_label->SetName(_T("down_name"));
		CProgressUI *new_progress = new CProgressUI;
		new_progress->SetMinValue(0);
		new_progress->SetMaxValue(100);
		new_progress->SetValue(1);
		new_progress->SetMaxWidth(200);
		new_progress->SetMaxHeight(7);
		new_progress->SetForeImage(_T("progress_fore.png"));
		new_progress->SetName(_T("down_progress"));
		new_v_lay->Add(new_label);
		new_v_lay->Add(new_progress);

		CLabelUI *new_label2 = new CLabelUI;
		CLabelUI *new_label3 = new CLabelUI;
		CVerticalLayoutUI *new_v_lay2 = new CVerticalLayoutUI;
		new_h_lay->Add(new_v_lay2);
		new_v_lay2->Add(new_label2);
		new_v_lay2->Add(new_label3);
		new_label2->ApplyAttributeList(
			_T("align=\"right\" text=\"\" textcolor=\"#FFAAAAAA\" showhtml=\"true\""));
		new_label3->ApplyAttributeList(
			_T("align=\"right\" text=\"0.00K/34.33M \" textcolor=\"#FFAAAAAA\" showhtml=\"true\""));

		new_node->Add(new_h_lay);
		down_list->Add(new_node);
	}
}

void CPage1::OnSelectChanged( TNotifyUI &msg )
{

}

void CPage1::OnItemClick( TNotifyUI &msg )
{

}

//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CPage2, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK,OnItemClick)
DUI_END_MESSAGE_MAP()


CPage2::CPage2()
{
	m_pPaintManager = NULL;
}

void CPage2::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	m_pPaintManager = pPaintMgr;
}

void CPage2::OnClick(TNotifyUI& msg)
{
	MessageBox(NULL,"哈哈","提示",MB_OK);
}

void CPage2::OnSelectChanged( TNotifyUI &msg )
{

}

void CPage2::OnItemClick( TNotifyUI &msg )
{

}

//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CRichListWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK,OnItemClick)
DUI_END_MESSAGE_MAP()

CRichListWnd::CRichListWnd(void)
{
	m_Page1.SetPaintMagager(&m_PaintManager);
	m_Page2.SetPaintMagager(&m_PaintManager);

	AddVirtualWnd(_T("page1"),&m_Page1);
	AddVirtualWnd(_T("page2"),&m_Page2);
}

CRichListWnd::~CRichListWnd(void)
{
	RemoveVirtualWnd(_T("page1"));
	RemoveVirtualWnd(_T("page2"));
}

void CRichListWnd::OnFinalMessage( HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

DuiLib::CDuiString CRichListWnd::GetSkinFolder()
{
#ifdef _DEBUG
	return _T("skin\\RichListRes\\");
#else
	return _T("skin\\");
#endif
	
}

DuiLib::CDuiString CRichListWnd::GetSkinFile()
{
	return _T("duilib.xml");
}

UILIB_RESOURCETYPE CRichListWnd::GetResourceType() const
{
#ifdef _DEBUG
	return UILIB_FILE;
#else
	return UILIB_ZIP;
#endif
}

DuiLib::CDuiString CRichListWnd::GetZIPFileName() const
{
	return _T("RichListRes.zip");
}

LPCTSTR CRichListWnd::GetWindowClassName( void ) const
{
	return _T("RichListWnd");
}

void CRichListWnd::OnClick( TNotifyUI &msg )
{
	if(msg.pSender ==m_pAddBtn)
	{
		AddNewUrlTask();
	}
	if( msg.pSender == m_pCloseBtn ) 
	{ 
		PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
		return; 
	}else if( msg.pSender == m_pMinBtn ) 
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
		return; 
	}else if( msg.pSender == m_pMaxBtn ) 
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
		return; 
	}else if( msg.pSender == m_pRestoreBtn ) 
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		return;
	}
	else if( msg.pSender->GetName() == _T("quitbtn") ) 
	{
		PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
	}
}

void CRichListWnd::OnSelectChanged( TNotifyUI &msg )
{
	if(msg.pSender->GetName() == _T("down_list"))
	{
		static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_main")))->SelectItem(0);
	}
	else if(msg.pSender->GetName() == _T("down_his"))
	{
		static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_main")))->SelectItem(1);
	}
}

void CRichListWnd::OnItemClick( TNotifyUI &msg )
{
	/*TCHAR alert_msg[64] = {0};
	int index = ((CListContainerElementUI *)msg.pSender)->GetIndex();
	wsprintf(alert_msg, _T("选中了行%d, 查找本行内的下载项目名..."), index);
	MessageBox(NULL, alert_msg, _T("DUILIB DEMO"), MB_OK);            
	CControlUI *find_ctrl =m_PaintManager.FindSubControlByName(msg.pSender, _T("down_name"));

	if(find_ctrl)
	{
		MessageBox(NULL, 
			find_ctrl->GetText()+_T(" 选中行的下载项目名称.."), 
			_T("DUILIB DEMO"), MB_OK);   
		((CLabelUI *)find_ctrl)->SetText(_T("由程序动态设置后的名称..."));
	}
	else
	{
		MessageBox(NULL, _T("本测试行未为控件设置name，故找不到要操作的控件"), 
			_T("DUILIB DEMO"), MB_OK);   
	}

	find_ctrl =m_PaintManager.FindSubControlByName(msg.pSender, _T("down_progress"));

	if(find_ctrl)
	{
		TCHAR alert_msg[256] = {0};
		wsprintf(alert_msg, _T("进度条值:%d"), ((CProgressUI *)find_ctrl)->GetValue());
		MessageBox(NULL, alert_msg, _T("DUILIB DEMO"), MB_OK);   

		((CProgressUI *)find_ctrl)->SetValue(30);
		MessageBox(NULL, _T("修改了进度条值"), _T("DUILIB DEMO"), MB_OK);   
	}*/
}

void CRichListWnd::Notify( TNotifyUI &msg )
{
	return WindowImplBase::Notify(msg);
}

LRESULT CRichListWnd::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// 解决ie控件收不到滚动消息的问题
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	::ScreenToClient(m_PaintManager.GetPaintWindow(), &pt);
	CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ie")));
	if( pControl && pControl->IsVisible() ) {
		RECT rc = pControl->GetPos();
		if( ::PtInRect(&rc, pt) ) {
			return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		}
	}

	bHandled = FALSE;
	return 0;
}
LRESULT  CRichListWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam == DEF_TIMER_ID_UPDATE_SPPEND)
	{
		map<string,int>mp_node_server = m_kernel.NofiyKernelReturnNodeServerByte();
		map<string,int>mp_http  = m_kernel.NotifyKernelReturnHttpDownloadByte();
		map<string,int>::iterator ite_node = mp_node_server.begin();
		map<string,int>::iterator ite_http = mp_http.begin();
		while(ite_node!=mp_node_server.end())
		{
			int nSpeed  =ite_node->second;
			double dbSpeedNode;
			CDuiString strNodeSpeed;
			m_lock_control.Lock();
			map<CDuiString ,STRU_LIST_CONTROL*>::iterator ite_control  =m_mp_control.find(ite_node->first.c_str());
			if(ite_control != m_mp_control.end())
			{
					if(nSpeed <1024)
					{
						strNodeSpeed.Format("+%dB",nSpeed);
					}
					else if(nSpeed>1024&&nSpeed<1024*1024)
					{
						nSpeed/=1024;
						strNodeSpeed.Format("+%dKB",nSpeed);
					}
					else 
					{
							dbSpeedNode  =(double)nSpeed/(1024*1024);
							strNodeSpeed.Format("+%.1lfMB",dbSpeedNode);
					}

					ite_control->second->m_label[3].SetText(strNodeSpeed);
			}
			m_lock_control.UnLock();
			ite_node++;
		}
		while(ite_http!=mp_http.end())
		{
			int nSpeed  =ite_http->second;
			double dbSpeedHttp;
			CDuiString strHttpSpeed;
			m_lock_control.Lock();
			map<CDuiString ,STRU_LIST_CONTROL*>::iterator ite_control  =m_mp_control.find(ite_http->first.c_str());
			if(ite_control != m_mp_control.end())
			{
					if(nSpeed <1024)
					{
						strHttpSpeed.Format("%dB",nSpeed);
					}
					else if(nSpeed>=1024&&nSpeed<1024*1024)
					{
						nSpeed/=1024;
						strHttpSpeed.Format("%dKB",nSpeed);
					}
					else 
					{
							dbSpeedHttp  =(double)nSpeed/(1024*1024);
							strHttpSpeed.Format("%.1lfMB",dbSpeedHttp);
					}

					ite_control->second->m_label[2].SetText(strHttpSpeed);
			}
			m_lock_control.UnLock();
			ite_http++;
		}
	}//定时器
	return 1L;
}
LRESULT CRichListWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 {
        LRESULT lRes = 0;
        switch (uMsg)
        {
        case WM_TIMER: lRes = OnTimer(uMsg, wParam, lParam, bHandled); break;
        }
        //bHandled = FALSE;
        
		return WindowImplBase::HandleCustomMessage(uMsg,wParam,lParam,bHandled);
 }

LRESULT CRichListWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE ) {
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) {
		if( !bZoomed ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}

void CRichListWnd::InitWindow()
{
	m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
	m_pAddBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("down_new")));
//	m_list.InsertItem(0);
	m_pList = static_cast<CListUIEx*>(m_PaintManager.FindControl(_T("down_list_tab")));
	m_pListTip = static_cast<CListUI*>(m_PaintManager.FindControl(_T("list_tip")));


	m_kernel.Init(this);
	//设置定时器
	HWND hwnd = m_PaintManager.GetPaintWindow();
    SetTimer(hwnd, DEF_TIMER_ID_UPDATE_SPPEND, 1000,    NULL);
	//for (int i = 0; i < 100; i++)
 //       {
 //           CListTextElementUI* pListElement = new CListTextElementUI;
 //           pListElement->SetTag(i);
 //           m_pListTip->Add(pListElement);
	//		CDuiString str ;

	//		str.Format("%d",i);
 //           pListElement->SetText(0, str);
 //         //  pListElement->SetText(1, _T("haha"));
	//		
 //       }
	
	//m_pListTip->Add(textui);
	//m_pListTip->Add(textui);
	//m_pListTip->Add(textui);
	/*pList->InsertItem(0);
	pList->InsertItem(1);
	pList->InsertItem(2);
	pList->InsertItem(3);*/
	//pList->InsertItem(1);
	//pList->InsertItem(2);
	//CDialogBuilder builder;
	//CListContainerElementUI* pLine = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	//CListContainerElementUI pCopy= *pLine;
	//pList->Add(pLine);
	//delete pLine;
	//pLine = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	//pList->AddAt(pLine,4);
	/*pList->Add(&pCopy);*/
	//CListContainerElementUI* pLine1 = (CListContainerElementUI*)(builder.Create(_T("listContainer1.xml"),(UINT)0, this));
	
	//CListContainerElementUI* pLine2 = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	//CListContainerElementUI* pLine1 = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	//CListContainerElementUI* pLine2 = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	//* pLine3 = (CListContainerElementUI*)(builder.Create(_T("listContainer.xml"),(UINT)0, this));
	/*if( pLine != NULL ) 
	{*/
		//pList->Add(pLine);
		
		
		//pList->AddAt(pLine1,1);
		//pList->AddAt(pLine2,2);
		//pList->AddAt(pLine3,3);
	//pList->InsertItem(0, 60, pLine); //此函数是经过二次封装的
	//}

}

LRESULT CRichListWnd::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	CControlUI* pHover = m_PaintManager.FindControl(pt);
	if( pHover == NULL ) return 0;
	/*演示悬停在下载列表的图标上时，动态变换下载图标状态显示*/
	if(pHover->GetName() == _T("down_ico"))
	{
		MessageBox(NULL, _T("鼠标在某控件例如按钮上悬停后，对目标控件操作，这里改变了状态图标大小"), _T("DUILIB DEMO"), MB_OK);
		((CButtonUI *)pHover)->ApplyAttributeList(
			_T("normalimage=\"file='downlist_pause.png' dest='15,9,32,26'\""));                
	}
	return 0;
}

LRESULT CRichListWnd::OnChar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	/*演示键盘消息的处理*/
	TCHAR press_char = (TCHAR)wParam;
	if(press_char == VK_BACK)
	{
		MessageBox(NULL, _T("按下了回退键"), _T("DUILIB DEMO"), MB_OK);
	}
	else
	{
		bHandled = FALSE;
	}
	return 0;
}
///////////自己写的函数
void  CRichListWnd::AddNewUrlTask()
{

	CDownDialog *pDownDiag;
	pDownDiag = new CDownDialog;
	pDownDiag->Init(this);
	
	pDownDiag->Create(NULL, _T("下载"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 500, 400);
	pDownDiag->CenterWindow();
	//pDownDiag->ShowModal();
	::ShowWindow(*pDownDiag, SW_SHOW);
 
}
//void CRichListWnd::NotifyUIUpdateCompleteFile(char *pFile,__int64 i64Size)
//{
//
//}
   
void CRichListWnd::NotifyRichListOk(CDuiString strUrl,CDuiString strPath)
{
	 // 1.解析出文件名
	int nIndex  =strUrl.ReverseFind('//');
	//获取文件名
	CDuiString strFileName = strUrl.Right(strUrl.GetLength()-(nIndex+1));

	//插入列表项
	m_pList->InsertItem(strFileName);


  // 将文件名和URL传给kernel
	m_kernel.StartDownLoad(strUrl.GetData(),strPath.GetData(),strFileName.GetData());


	  


	
	
}
MyLock m_lock_tip;
void  CRichListWnd::NofifyUITip(string strTip)
{

	//Kernel回调的通知显示：
	m_lock_tip.Lock();
		CListTextElementUI* pListElement = new CListTextElementUI;
       // pListElement->SetTag(i);
        m_pListTip->Add(pListElement);
		CDuiString str (strTip.c_str());
		//str.Format("%d",i);
        pListElement->SetText(0, str);
		m_lock_tip.UnLock();
}

void CRichListWnd::NofityUIFileInfo(string strFileName,__int64 fileSize,int HasWrittenByte)
{
	CDuiString str_size ;
	double dsize;
	if( fileSize<1024)
	{
		str_size.Format("大小%dB",fileSize);
	}
	else if(fileSize>=1024&&fileSize<1024*1024)
	{
		dsize  =double(fileSize)/1024;
		str_size.Format("大小%.1lfKB",dsize);
	}
	else if(fileSize>=1048576&&fileSize<1048576*1024)
	{
			dsize  =double(fileSize)/1048576;
			str_size.Format("大小%.1lfMB",dsize);
	}
	else
	{
		dsize  =double(fileSize)/1073741824;
		str_size.Format("大小%.1lfGB",dsize);
	}
	m_lock_control.Lock();
	map<CDuiString,STRU_LIST_CONTROL *>::iterator ite_control = m_mp_control.find(strFileName.c_str());
	if(ite_control != m_mp_control.end())
	{
		ite_control->second->m_label[1].SetText(str_size);
	}
	//下载进度
	m_lock_control.UnLock();
}