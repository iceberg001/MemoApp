#include "../StdAfx.h"
#include "TodoNode.h"
#include "../NodeMessages.h"

using namespace DuiLib;

TodoNodeUI::TodoNodeUI()
    : list_(NULL), statToday_(NULL), statDoing_(NULL), statDone_(NULL),
      editNew_(NULL), filter_(-1)
{
    SetNodeName(_T("TodoNode"));
    BuildUI();
    Refresh();
}

LPCTSTR TodoNodeUI::GetClass() const { return _T("TodoNodeUI"); }

LPVOID TodoNodeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("TodoNodeUI")) == 0) return static_cast<TodoNodeUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

void TodoNodeUI::OnNodeMessage(const CNodeMessage& msg)
{
    // 周报导入待办后无需处理；预留
}

void TodoNodeUI::BuildUI()
{
    CVerticalLayoutUI* root = nodeui::V();
    root->SetChildPadding(10);

    // 标题行
    CHorizontalLayoutUI* header = nodeui::H(34);
    header->SetChildPadding(8);
    CLabelUI* title = nodeui::Label(_T("待办事项"), COL_TEXT, 34, 1);
    title->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(title);
    CControlUI* spacer = new CControlUI;
    header->Add(spacer);
    CButtonUI* btnAdd = nodeui::Button(_T("btn_add_todo"), _T("+ 新建待办"), COL_BRAND, 0xFFFFFFFF, 110, 30);
    btnAdd->OnNotify += MakeDelegate(this, &TodoNodeUI::OnAddClick);
    header->Add(btnAdd);
    root->Add(header);

    // 统计卡
    CHorizontalLayoutUI* stats = nodeui::H(58);
    stats->SetChildPadding(10);
    struct { LPCTSTR name; CLabelUI** out; DWORD color; } defs[3] = {
        { _T("未开始"), &statToday_, COL_TODO_GRAY },
        { _T("进行中"), &statDoing_, COL_WARN },
        { _T("已完成"), &statDone_,  COL_BRAND_MID },
    };
    for (int i = 0; i < 3; ++i) {
        CVerticalLayoutUI* card = nodeui::V();
        card->SetBkColor(COL_SURFACE);
        card->SetInset(CDuiRect(14, 8, 14, 8));
        CLabelUI* cap = nodeui::Label(defs[i].name, COL_TEXT_TIP, 20, 2);
        card->Add(cap);
        CLabelUI* num = nodeui::Label(_T("0"), defs[i].color, 24, 1);
        *defs[i].out = num;
        card->Add(num);
        stats->Add(card);
    }
    root->Add(stats);

    // 筛选行
    CHorizontalLayoutUI* filters = nodeui::H(30);
    filters->SetChildPadding(8);
    LPCTSTR filterNames[4] = { _T("全部"), _T("未开始"), _T("进行中"), _T("已完成") };
    for (int i = 0; i < 4; ++i) {
        TCHAR fname[32];
        _stprintf_s(fname, _T("filter_%d"), i - 1);
        CButtonUI* b = nodeui::Button(fname, filterNames[i], COL_SURFACE, COL_TEXT_SUB, 72, 26);
        if (i == 0) { b->SetBkColor(COL_BORDER); b->SetTextColor(COL_TEXT); }
        b->SetFont(2);
        b->OnNotify += MakeDelegate(this, &TodoNodeUI::OnFilterClick);
        filterBtns_.push_back(b);
        filters->Add(b);
    }
    root->Add(filters);

    // 新建行
    CHorizontalLayoutUI* addRow = nodeui::H(34);
    addRow->SetChildPadding(8);
    editNew_ = nodeui::RichEdit(_T("todoEdit"), 0, 30);
    editNew_->SetAttribute(_T("bordersize"), _T("1"));
    editNew_->SetAttribute(_T("bordercolor"), _T("#FF7A7A7A"));
    editNew_->SetText(_T(""));
    addRow->Add(editNew_);
    CButtonUI* btnAdd2 = nodeui::Button(_T("btn_add_todo2"), _T("添加"), COL_BRAND, 0xFFFFFFFF, 70, 30);
    btnAdd2->OnNotify += MakeDelegate(this, &TodoNodeUI::OnAddClick);
    addRow->Add(btnAdd2);
    root->Add(addRow);

    // 任务列表（可滚动）
    list_ = nodeui::V();
    list_->SetAttribute(_T("vscrollbar"), _T("true"));
    nodeui::StyleVScrollbar(list_);
    list_->SetChildPadding(8);
    root->Add(list_);

    this->Add(root);
}

void TodoNodeUI::UpdateStats()
{
    int c0 = 0, c1 = 0, c2 = 0;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].status == 0) ++c0;
        else if (items_[i].status == 1) ++c1;
        else ++c2;
    }
    if (statToday_) statToday_->SetText(TStrFromW(std::to_wstring(c0)).GetData());
    if (statDoing_) statDoing_->SetText(TStrFromW(std::to_wstring(c1)).GetData());
    if (statDone_)  statDone_->SetText(TStrFromW(std::to_wstring(c2)).GetData());
}

void TodoNodeUI::SaveAndNotify()
{
    DataStore::GetInstance().SaveTodos(items_);
    UpdateStats();
    CNodeMessage msg(NODE_MSG_DATA_CHANGED, _T("todos"));
    PostNodeMessage(msg.msgType, msg.msgData);
}

void TodoNodeUI::Refresh()
{
    if (!list_) return;
    list_->RemoveAll();
    items_ = DataStore::GetInstance().LoadTodos();

    for (size_t i = 0; i < items_.size(); ++i) {
        TodoItem& it = items_[i];
        if (filter_ >= 0 && it.status != filter_) continue;

        CVerticalLayoutUI* card = nodeui::Card();
        card->SetInset(CDuiRect(12, 8, 12, 8));
        card->SetChildPadding(6);
        // 固定高度 = 上下inset8 + 行1(24) + 间距6 + 行2(14) + 下inset8 = 60。
        // 若不固定高度，垂直布局会把子项压缩塞进可视区而不是触发滚动条。
        card->SetFixedHeight(60);

        // 第一行：勾选 + 标题 + 状态徽章 + 截止 + 删除
        CHorizontalLayoutUI* row1 = nodeui::H(24);
        row1->SetChildPadding(8);

        TCHAR idbuf[32];
        _stprintf_s(idbuf, _T("%I64u"), it.id);

        CButtonUI* chk = new CButtonUI;
        TCHAR cname[64];
        _stprintf_s(cname, _T("chk_%s"), idbuf);
        chk->SetName(cname);
        chk->SetText((it.status == 2) ? _T("✔") : _T("○"));
        chk->SetTextColor(it.status == 2 ? COL_BRAND_MID : COL_TEXT_TIP);
        chk->SetFixedWidth(24);
        chk->SetFixedHeight(24);
        chk->SetFont(1);
        chk->OnNotify += MakeDelegate(this, &TodoNodeUI::OnRowCheck);
        row1->Add(chk);

        CLabelUI* lbl = nodeui::Label(TStrFromW(it.title).GetData(), COL_TEXT, 24, 0);
        lbl->SetAttribute(_T("valign"), _T("vcenter"));
        if (it.status == 2) lbl->SetAttribute(_T("textcolor"), _T("#FF888780"));
        row1->Add(lbl);

        LPCTSTR statusText[3] = { _T("未开始"), _T("进行中"), _T("已完成") };
        DWORD statusFg[3] = { COL_TODO_GRAY, COL_WARN, COL_BRAND_MID };
        DWORD statusBg[3] = { COL_SURFACE, COL_WARN_BG, COL_BRAND_BG };
        row1->Add(nodeui::Badge(statusText[it.status], statusFg[it.status], statusBg[it.status], 60));

        CLabelUI* due = nodeui::Label(TStrFromW(it.due).GetData(), COL_TEXT_TIP, 24, 2);
        due->SetFixedWidth(80);
        due->SetAttribute(_T("valign"), _T("vcenter"));
        due->SetAttribute(_T("align"), _T("right"));
        row1->Add(due);

        CButtonUI* del = nodeui::Button((CDuiString(_T("del_")) + idbuf).GetData(), _T("✕"),
                                        COL_CARD_BG, COL_DANGER, 24, 24);
        del->OnNotify += MakeDelegate(this, &TodoNodeUI::OnRowDelete);
        row1->Add(del);
        card->Add(row1);

        // 第二行：进度条 + 百分比
        CHorizontalLayoutUI* row2 = nodeui::H(14);
        row2->SetChildPadding(8);
        CControlUI* pad = new CControlUI;
        pad->SetFixedWidth(24);
        row2->Add(pad);

        CSliderUI* slider = new CSliderUI;
        TCHAR sname[64];
        _stprintf_s(sname, _T("sl_%s"), idbuf);
        slider->SetName(sname);
        slider->SetMinValue(0);
        slider->SetMaxValue(100);
        slider->SetValue(it.progress);
        slider->SetAttribute(_T("bkcolor"), _T("#FFF4F3EF"));
        slider->SetAttribute(_T("forecolor"), _T("#FFBA7517"));
        slider->SetFixedHeight(14);
        slider->OnNotify += MakeDelegate(this, &TodoNodeUI::OnSliderChanged);
        row2->Add(slider);

        TCHAR pct[16];
        _stprintf_s(pct, _T("%d%%"), it.progress);
        CLabelUI* pctLbl = nodeui::Label(pct, COL_TEXT_SUB, 14, 2);
        pctLbl->SetFixedWidth(44);
        pctLbl->SetAttribute(_T("valign"), _T("vcenter"));
        row2->Add(pctLbl);

        card->Add(row2);
        list_->Add(card);
    }

    UpdateStats();
}

bool TodoNodeUI::OnAddClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    if (!editNew_) return false;
    std::wstring title = editNew_->GetText().GetData();
    // 去除空白
    size_t p = title.find_first_not_of(L" \t");
    if (p == std::wstring::npos) return true;
    TodoItem it;
    it.id = DataStore::GetInstance().NewId();
    it.title = title;
    it.status = 0;
    it.progress = 0;
    it.due = DataStore::GetInstance().TodayStr();
    it.updated = nodeui::NowStr().GetData();
    items_.push_back(it);
    SaveAndNotify();
    Refresh();
    editNew_->SetText(_T(""));
    return true;
}

bool TodoNodeUI::OnFilterClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    LPCTSTR name = msg->pSender->GetName().GetData();
    if (_tcsncmp(name, _T("filter_"), 7) != 0) return false;
    filter_ = _ttoi(name + 7);
    for (size_t i = 0; i < filterBtns_.size(); ++i) {
        int val = (int)i - 1;
        filterBtns_[i]->SetBkColor(val == filter_ ? COL_BORDER : COL_SURFACE);
        filterBtns_[i]->SetTextColor(val == filter_ ? COL_TEXT : COL_TEXT_SUB);
    }
    Refresh();
    return true;
}

static uint64_t ParseId(LPCTSTR name, LPCTSTR prefix)
{
    return _wtoi64(name + wcslen(prefix));
}

bool TodoNodeUI::OnRowCheck(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    uint64_t id = ParseId(msg->pSender->GetName().GetData(), L"chk_");
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].id == id) {
            if (items_[i].status == 2) {
                items_[i].status = 0;
                items_[i].progress = 0;
            } else {
                items_[i].status = 2;
                items_[i].progress = 100;
            }
            items_[i].updated = nodeui::NowStr().GetData();
            break;
        }
    }
    SaveAndNotify();
    Refresh();
    return true;
}

bool TodoNodeUI::OnRowDelete(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    uint64_t id = ParseId(msg->pSender->GetName().GetData(), L"del_");
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].id == id) {
            items_.erase(items_.begin() + i);
            break;
        }
    }
    SaveAndNotify();
    Refresh();
    return true;
}

bool TodoNodeUI::OnSliderChanged(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg) return false;
    if (msg->sType != DUI_MSGTYPE_VALUECHANGED) return false;
    CSliderUI* slider = static_cast<CSliderUI*>(msg->pSender);
    uint64_t id = ParseId(slider->GetName().GetData(), L"sl_");
    int value = slider->GetValue();
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].id == id) {
            items_[i].progress = value;
            items_[i].status = (value >= 100) ? 2 : (value > 0 ? 1 : 0);
            items_[i].updated = nodeui::NowStr().GetData();
            break;
        }
    }
    SaveAndNotify();
    Refresh();
    return true;
}
