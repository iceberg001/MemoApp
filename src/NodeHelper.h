#ifndef __NODEHELPER_H__
#define __NODEHELPER_H__

#include "StdAfx.h"

// 公共配色（对应 UI 设计资源包 theme.css 浅色主题）
#define COL_BRAND        0xFF0F6E56
#define COL_BRAND_MID    0xFF1D9E75
#define COL_BRAND_BG     0xFFE1F5EE
#define COL_TEXT         0xFF2C2C2A
#define COL_TEXT_SUB     0xFF5F5E5A
#define COL_TEXT_TIP     0xFF888780
#define COL_PAGE_BG      0xFFF1EFE8
#define COL_CARD_BG      0xFFFFFFFF
#define COL_SURFACE      0xFFF4F3EF
#define COL_BORDER       0xFFD3D1C7
#define COL_WARN         0xFFBA7517
#define COL_WARN_BG      0xFFFAEEDA
#define COL_INFO         0xFF378ADD
#define COL_INFO_BG      0xFFE6F1FB
#define COL_DANGER       0xFFE24B4A
#define COL_TODO_GRAY    0xFF888780
#define COL_NOTE_AMBER   0xFFFAC775
#define COL_NOTE_TEAL    0xFF9FE1CB
#define COL_NOTE_BLUE    0xFFB5D4F4
#define COL_NOTE_PINK    0xFFF4C0D1
#define COL_NOTE_GREEN   0xFFC0DD97

namespace nodeui {

// 统一滚动条皮肤。组件在挂接到 PaintManager 之前就创建了滚动条，
// main.xml 的 <Default VScrollBar> 不会生效（EnableScrollBar 仅在 m_pManager
// 已存在时应用默认属性），因此这里直接逐项设置图片样式。
inline void StyleVScrollbar(DuiLib::CContainerUI* c)
{
    if (!c) return;
    DuiLib::CScrollBarUI* sb = c->GetVerticalScrollBar();
    if (!sb) return;
    sb->SetFixedWidth(16);
    sb->SetAttribute(_T("bknormalimage"),    _T("file='icons/scroll_track.png'"));
    sb->SetAttribute(_T("thumbnormalimage"), _T("file='icons/scroll_thumb.png' corner='2,8,2,8'"));
    sb->SetAttribute(_T("thumbhotimage"),    _T("file='icons/scroll_thumb_hot.png' corner='2,8,2,8'"));
    sb->SetAttribute(_T("thumbpushedimage"), _T("file='icons/scroll_thumb_pushed.png' corner='2,8,2,8'"));
    sb->SetAttribute(_T("button1normalimage"), _T("file='icons/scroll_up.png'"));
    sb->SetAttribute(_T("button1hotimage"),    _T("file='icons/scroll_up.png'"));
    sb->SetAttribute(_T("button1pushedimage"), _T("file='icons/scroll_up.png'"));
    sb->SetAttribute(_T("button2normalimage"), _T("file='icons/scroll_down.png'"));
    sb->SetAttribute(_T("button2hotimage"),    _T("file='icons/scroll_down.png'"));
    sb->SetAttribute(_T("button2pushedimage"), _T("file='icons/scroll_down.png'"));
}

inline DuiLib::CVerticalLayoutUI* V(int height = 0)
{
    DuiLib::CVerticalLayoutUI* p = new DuiLib::CVerticalLayoutUI;
    if (height > 0) p->SetFixedHeight(height);
    return p;
}

inline DuiLib::CHorizontalLayoutUI* H(int height = 0)
{
    DuiLib::CHorizontalLayoutUI* p = new DuiLib::CHorizontalLayoutUI;
    if (height > 0) p->SetFixedHeight(height);
    return p;
}

inline DuiLib::CLabelUI* Label(LPCTSTR text, DWORD color = COL_TEXT, int height = 0, int font = 0)
{
    DuiLib::CLabelUI* p = new DuiLib::CLabelUI;
    p->SetText(text);
    p->SetTextColor(color);
    p->SetFont(font);
    if (height > 0) p->SetFixedHeight(height);
    return p;
}

inline DuiLib::CButtonUI* Button(LPCTSTR name, LPCTSTR text, DWORD bkcolor,
                                 DWORD textcolor = 0xFFFFFFFF, int width = 0, int height = 30)
{
    DuiLib::CButtonUI* p = new DuiLib::CButtonUI;
    p->SetName(name);
    p->SetText(text);
    p->SetBkColor(bkcolor);
    p->SetTextColor(textcolor);
    p->SetTextPadding(DuiLib::CDuiRect(8, 0, 8, 0));
    if (width > 0) p->SetFixedWidth(width);
    if (height > 0) p->SetFixedHeight(height);
    p->SetAttribute(_T("align"), _T("center"));
    return p;
}

inline DuiLib::CEditUI* Edit(LPCTSTR name, int width = 0, int height = 30)
{
    DuiLib::CEditUI* p = new DuiLib::CEditUI;
    p->SetName(name);
    p->SetFixedHeight(height);
    if (width > 0) p->SetFixedWidth(width);
    p->SetAttribute(_T("bkcolor"), _T("#FFFFFFFF"));
    p->SetAttribute(_T("border"), _T("1"));
    p->SetAttribute(_T("bordercolor"), _T("#FFB4B2A9"));
    p->SetTextColor(COL_TEXT);
    p->SetFont(0);
    p->SetTextPadding(DuiLib::CDuiRect(8, 0, 8, 0));
    return p;
}

inline DuiLib::CRichEditUI* RichEdit(LPCTSTR name, int height, bool multiline = true)
{
    DuiLib::CRichEditUI* p = new DuiLib::CRichEditUI;
    p->SetName(name);
    p->SetFixedHeight(height);
    if (multiline) {
        p->SetAttribute(_T("multiline"), _T("true"));
        p->SetAttribute(_T("vscrollbar"), _T("true"));
        p->SetAttribute(_T("wanttab"), _T("false"));
        StyleVScrollbar(p);
    }
    p->SetAttribute(_T("bkcolor"), _T("#FFFFFFFF"));
    p->SetAttribute(_T("border"), _T("1"));
    p->SetAttribute(_T("bordercolor"), _T("#FFB4B2A9"));
    p->SetTextColor(COL_TEXT);
    p->SetFont(0);
    return p;
}

// 卡片容器：白底 + 1px 边框 + 圆角感（duilib 无圆角矩形，用细边框代替）
inline DuiLib::CVerticalLayoutUI* Card(int height = 0)
{
    DuiLib::CVerticalLayoutUI* p = new DuiLib::CVerticalLayoutUI;
    if (height > 0) p->SetFixedHeight(height);
    p->SetBkColor(COL_CARD_BG);
    p->SetBorderSize(1);
    p->SetBorderColor(COL_BORDER);
    p->SetChildPadding(4);
    return p;
}

inline DuiLib::CLabelUI* Badge(LPCTSTR text, DWORD fg, DWORD bg, int width = 0)
{
    DuiLib::CLabelUI* p = new DuiLib::CLabelUI;
    p->SetText(text);
    p->SetTextColor(fg);
    p->SetBkColor(bg);
    p->SetFont(2);
    if (width > 0) p->SetFixedWidth(width);
    p->SetFixedHeight(22);
    p->SetAttribute(_T("align"), _T("center"));
    return p;
}

inline CDuiString NowStr()
{
    time_t t = time(NULL);
    struct tm lt;
    localtime_s(&lt, &t);
    TCHAR buf[64] = { 0 };
    _stprintf_s(buf, _T("%04d-%02d-%02d %02d:%02d"),
        lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min);
    return CDuiString(buf);
}

} // namespace nodeui

#endif
