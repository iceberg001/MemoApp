#include "StdAfx.h"
#include "MainWindow.h"
#include "resource.h"

// 从 exe 内嵌 ZIPRES 资源加载皮肤（zip 含 main.xml 与 png 图标）
static void InitResource()
{
    HMODULE hExe = GetModuleHandle(NULL);
    HRSRC hResource = ::FindResource(hExe, MAKEINTRESOURCE(IDR_SKIN), _T("ZIPRES"));
    if (hResource != NULL) {
        HGLOBAL hGlobal = ::LoadResource(hExe, hResource);
        if (hGlobal != NULL) {
            DWORD dwSize = ::SizeofResource(hExe, hResource);
            if (dwSize > 0) {
                LPBYTE pData = (LPBYTE)::LockResource(hGlobal);
                CPaintManagerUI::SetResourceZip(pData, dwSize, NULL);
            }
        }
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetResourceType(UILIB_ZIPRESOURCE);
    InitResource();

    HRESULT Hr = ::CoInitialize(NULL);
    if (SUCCEEDED(Hr)) {
        CMainWindow* pWindow = new CMainWindow();
        if (pWindow != NULL) {
            pWindow->Create(NULL, _T("备忘录工作台"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
            pWindow->CenterWindow();
            pWindow->ShowWindow(nCmdShow > SW_HIDE ? SW_SHOW : SW_SHOW);
            CPaintManagerUI::MessageLoop();
        }
        ::CoUninitialize();
    }
    return 0;
}
