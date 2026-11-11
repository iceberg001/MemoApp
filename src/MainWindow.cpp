#include "StdAfx.h"
#include "MainWindow.h"
#include "resource.h"

using namespace DuiLib;

DUI_BEGIN_MESSAGE_MAP(CMainWindow, WindowImplBase)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CDuiString CMainWindow::GetSkinFile()
{
    return _T("main.xml");
}

LPCTSTR CMainWindow::GetWindowClassName() const
{
    return _T("MemoAppWnd");
}

UINT CMainWindow::GetClassStyle() const
{
    return CS_DBLCLKS;
}

void CMainWindow::OnFinalMessage(HWND hWnd)
{
    RemoveTrayIcon();
    CNodeUIManager::GetInstance().Clear();
    __super::OnFinalMessage(hWnd);
    // 主窗口已销毁，必须投递 WM_QUIT 结束 MessageLoop，否则进程残留
    ::PostQuitMessage(0);
    delete this;
}

void CMainWindow::InitWindow()
{
    pages_ = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("pageContainer")));
    currentPage_ = -1;
    trayAdded_ = false;
    balloonShown_ = false;

    if (pages_) {
        pages_->Add(new TodoNodeUI);
        pages_->Add(new MemoNodeUI);
        pages_->Add(new NoteNodeUI);
        pages_->Add(new ReportNodeUI);
    }
    navBtns_.push_back(static_cast<CButtonUI*>(m_pm.FindControl(_T("nav_todo"))));
    navBtns_.push_back(static_cast<CButtonUI*>(m_pm.FindControl(_T("nav_memo"))));
    navBtns_.push_back(static_cast<CButtonUI*>(m_pm.FindControl(_T("nav_note"))));
    navBtns_.push_back(static_cast<CButtonUI*>(m_pm.FindControl(_T("nav_report"))));
    SwitchPage(0);

    InitAppIcon();
    InitTrayIcon();
}

void CMainWindow::SwitchPage(int index)
{
    if (!pages_ || index == currentPage_) return;
    currentPage_ = index;
    pages_->SelectItem(index);
    for (size_t i = 0; i < navBtns_.size(); ++i) {
        if (navBtns_[i]) {
            navBtns_[i]->SetBkColor((int)i == index ? COL_BORDER : 0x00000000);
        }
    }
}

void CMainWindow::OnClick(TNotifyUI& msg)
{
    LPCTSTR name = msg.pSender->GetName().GetData();
    if (_tcsicmp(name, _T("closebtn")) == 0) {
        HideToTray();                       // 关闭 = 隐藏到托盘
    } else if (_tcsicmp(name, _T("minbtn")) == 0) {
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
    } else if (_tcsicmp(name, _T("maxbtn")) == 0) {
        if (IsZoomed(*this)) SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
        else                SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    } else if (_tcsicmp(name, _T("nav_todo")) == 0) {
        SwitchPage(0);
    } else if (_tcsicmp(name, _T("nav_memo")) == 0) {
        SwitchPage(1);
    } else if (_tcsicmp(name, _T("nav_note")) == 0) {
        SwitchPage(2);
    } else if (_tcsicmp(name, _T("nav_report")) == 0) {
        SwitchPage(3);
    }
}

void CMainWindow::Notify(TNotifyUI& msg)
{
    __super::Notify(msg);
}

// ---------- 应用图标 ----------

void CMainWindow::InitAppIcon()
{
    HMODULE hMod = GetModuleHandle(NULL);
    hIconBig_   = (HICON)LoadImageW(hMod, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON,
                                    GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    hIconSmall_ = (HICON)LoadImageW(hMod, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON,
                                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (hIconBig_)   ::SendMessageW(*this, WM_SETICON, ICON_BIG, (LPARAM)hIconBig_);
    if (hIconSmall_) ::SendMessageW(*this, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall_);
}

// ---------- 托盘图标 ----------

void CMainWindow::InitTrayIcon()
{
    ZeroMemory(&nid_, sizeof(nid_));
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = *this;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAY_CALLBACK;
    nid_.hIcon = hIconSmall_ ? hIconSmall_ : (HICON)LoadImageW(GetModuleHandle(NULL),
                    MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    lstrcpyW(nid_.szTip, L"备忘录工作台");
    trayAdded_ = Shell_NotifyIconW(NIM_ADD, &nid_) != FALSE;
}

void CMainWindow::RemoveTrayIcon()
{
    if (trayAdded_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        trayAdded_ = FALSE;
    }
}

void CMainWindow::HideToTray()
{
    ::ShowWindow(GetHWND(), SW_HIDE);
    if (!balloonShown_) {
        nid_.uFlags = NIF_INFO;
        lstrcpyW(nid_.szInfo, L"程序已隐藏到托盘，点击托盘图标可重新打开");
        lstrcpyW(nid_.szInfoTitle, L"备忘录工作台");
        nid_.dwInfoFlags = NIIF_INFO;
        Shell_NotifyIconW(NIM_MODIFY, &nid_);
        nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;   // 恢复常规标志
        balloonShown_ = true;
    }
}

void CMainWindow::ShowMainWindow()
{
    ::ShowWindow(GetHWND(), SW_SHOW);
    SetForegroundWindow(*this);
}

void CMainWindow::ShowTrayMenu()
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_TRAY_MENU_SHOW, L"显示主界面(&S)");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TRAY_MENU_EXIT, L"退出(&X)");
    SetForegroundWindow(*this);           // 保证菜单点击后能正常消失
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, *this, NULL);
    DestroyMenu(menu);
}

// ---------- 自定义 Windows 消息 ----------

LRESULT CMainWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (uMsg == WM_TRAY_CALLBACK) {
        bHandled = TRUE;
        if (lParam == WM_LBUTTONUP) {
            if (::IsWindowVisible(*this)) HideToTray();
            else ShowMainWindow();
        } else if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu();
        }
        return 0;
    }
    if (uMsg == WM_COMMAND && lParam == 0) {   // 仅托盘菜单命令（菜单命令 lParam 为 0；
        bHandled = TRUE;                        // 编辑框 EN_CHANGE 通知 lParam 为控件句柄，
        WORD id = LOWORD(wParam);               // 必须放行给 m_pm.MessageHandler 做反射同步文本）
        if (id == ID_TRAY_MENU_SHOW) ShowMainWindow();
        else if (id == ID_TRAY_MENU_EXIT) { RemoveTrayIcon(); Close(); }
        return 0;
    }
    bHandled = FALSE;
    return 0;
}
