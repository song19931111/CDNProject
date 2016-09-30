
#define WIN32_LEAN_AND_MEAN
#include <comdef.h>
#include <exdisp.h>
#include "NodeServerDialog.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	//CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("\\skin"));
	//CPaintManagerUI::SetResourceZip(_T("RichListRes.zip"));
	
	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;

	CNodeServerDialog * pFrame = new CNodeServerDialog();
	if( pFrame == NULL ) return 0;
	pFrame->Create(NULL, _T("node_server"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 316, 447);
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);

	
	//::MessageBox(NULL, _T("alert!"), _T("Duilib Demo"), MB_OK);

	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	return 0;
}
