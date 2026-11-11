#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "StdAfx.h"
#include "nodes/TodoNode.h"
#include "nodes/MemoNode.h"
#include "nodes/NoteNode.h"
#include "nodes/ReportNode.h"

#define WM_TRAY_CALLBACK (WM_APP + 100)
#define ID_TRAY_MENU_SHOW 2001
#define ID_TRAY_MENU_EXIT 2002

// 主窗口：左侧导航 + TabLayout 页面容器，每个页面是一个 Node 组件
// 支持：托盘图标、关闭最小化到托盘
class CMainWindow : public DuiLib::WindowImplBase
{
public:
    DUI_DECLARE_MESSAGE_MAP()

    DuiLib::CDuiString GetSkinFile();
    LPCTSTR GetWindowClassName() const;
    UINT GetClassStyle() const;
    void OnFinalMessage(HWND hWnd);
    void InitWindow();
    void Notify(DuiLib::TNotifyUI& msg);
    void OnClick(DuiLib::TNotifyUI& msg);
    LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    void SwitchPage(int index);

    // 图标与托盘
    void InitAppIcon();
    void InitTrayIcon();
    void RemoveTrayIcon();
    void ShowMainWindow();
    void HideToTray();
    void ShowTrayMenu();

    DuiLib::CTabLayoutUI*              pages_;
    std::vector<DuiLib::CButtonUI*>    navBtns_;
    int                                currentPage_;

    NOTIFYICONDATAW                    nid_;
    HICON                              hIconBig_;
    HICON                              hIconSmall_;
    bool                               trayAdded_;
    bool                               balloonShown_;
};

#endif
