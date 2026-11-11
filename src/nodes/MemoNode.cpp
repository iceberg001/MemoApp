#include "../StdAfx.h"
#include "MemoNode.h"
#include "../NodeMessages.h"

using namespace DuiLib;

MemoNodeUI::MemoNodeUI()
    : list_(NULL), editTitle_(NULL), editContent_(NULL), filter_(-1)
{
    SetNodeName(_T("MemoNode"));
    BuildUI();
    Refresh();
}

LPCTSTR MemoNodeUI::GetClass() const { return _T("MemoNodeUI"); }

LPVOID MemoNodeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("MemoNodeUI")) == 0) return static_cast<MemoNodeUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

void MemoNodeUI::OnNodeMessage(const CNodeMessage& msg)
{
}

void MemoNodeUI::BuildUI()
{
    CVerticalLayoutUI* root = nodeui::V();
    root->SetChildPadding(10);

    // 标题行
    CHorizontalLayoutUI* header = nodeui::H(34);
    header->SetChildPadding(8);
    CLabelUI* title = nodeui::Label(_T("备忘录"), COL_TEXT, 34, 1);
    title->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(title);
    CLabelUI* tip = nodeui::Label(_T("支持记录账户密码等各类信息，可一键复制"), COL_TEXT_TIP, 34, 2);
    tip->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(tip);
    CControlUI* spacer = new CControlUI;
    header->Add(spacer);
    root->Add(header);

    // 新建区
    CVerticalLayoutUI* addCard = nodeui::Card();
    addCard->SetInset(CDuiRect(12, 10, 12, 10));
    addCard->SetChildPadding(8);
    CHorizontalLayoutUI* addRow1 = nodeui::H(30);
    addRow1->SetChildPadding(8);
    editTitle_ = nodeui::Edit(_T("memoTitle"), 0, 30);
    editTitle_->SetAttribute(_T("bordersize"), _T("1"));
    editTitle_->SetAttribute(_T("bordercolor"), _T("#FF7A7A7A"));
    addRow1->Add(editTitle_);
    // 分类选择（单选 Option：选中=青绿底白字，未选中=白底灰字描边）
    for (int i = 0; i < 3; ++i) {
        COptionUI* opt = new COptionUI;
        opt->SetText(Categories(i));
        opt->SetGroup(_T("memoCategory"));
        opt->SetFixedWidth(84);
        opt->SetFixedHeight(28);
        opt->SetFont(2);
        opt->SetTextColor(COL_TEXT_SUB);
        opt->SetBkColor(COL_CARD_BG);
        opt->SetBorderSize(1);
        opt->SetBorderColor(COL_BORDER);
        opt->SetAttribute(_T("align"), _T("center"));
        opt->SetSelectedBkColor(COL_BRAND);
        opt->SetSelectedTextColor(0xFFFFFFFF);
        if (i == 2) opt->SetAttribute(_T("selected"), _T("true"));
        opt->OnNotify += MakeDelegate(this, &MemoNodeUI::OnCategoryClick);
        catBtns_.push_back(opt);
        addRow1->Add(opt);
    }
    addCard->Add(addRow1);

    editContent_ = nodeui::RichEdit(_T("memoContent"), 64);
    editContent_->SetAttribute(_T("bordersize"), _T("1"));
    editContent_->SetAttribute(_T("bordercolor"), _T("#FF7A7A7A"));
    addCard->Add(editContent_);
    CHorizontalLayoutUI* addRow2 = nodeui::H(32);
    addRow2->SetChildPadding(8);
    CControlUI* pad = new CControlUI;
    addRow2->Add(pad);
    CButtonUI* btnAdd = nodeui::Button(_T("btn_add_memo"), _T("+ 保存备忘"), COL_BRAND, 0xFFFFFFFF, 110, 30);
    btnAdd->OnNotify += MakeDelegate(this, &MemoNodeUI::OnAddClick);
    addRow2->Add(btnAdd);
    addCard->Add(addRow2);
    root->Add(addCard);

    // 筛选行（单选 Option：选中=品牌青绿底白字，未选中=浅底灰字）
    CHorizontalLayoutUI* filters = nodeui::H(30);
    filters->SetChildPadding(8);
    LPCTSTR filterNames[4] = { _T("全部"), _T("账户密码"), _T("工作"), _T("生活") };
    for (int i = 0; i < 4; ++i) {
        TCHAR fname[32];
        _stprintf_s(fname, _T("mfilter_%d"), i - 1);   // -1=全部 / 0..2=分类
        COptionUI* opt = new COptionUI;
        opt->SetName(fname);
        opt->SetText(filterNames[i]);
        opt->SetGroup(_T("memoFilter"));
        opt->SetFixedWidth(84);
        opt->SetFixedHeight(28);
        opt->SetFont(2);
        opt->SetTextColor(COL_TEXT_SUB);               // 未选中：灰字
        opt->SetBkColor(COL_CARD_BG);                  // 未选中：白底
        opt->SetBorderSize(1);
        opt->SetBorderColor(COL_BORDER);
        opt->SetAttribute(_T("align"), _T("center"));
        opt->SetSelectedBkColor(COL_BRAND);            // 选中：青绿底
        opt->SetSelectedTextColor(0xFFFFFFFF);         // 选中：白字
        if (i == 0) opt->SetAttribute(_T("selected"), _T("true"));
        opt->OnNotify += MakeDelegate(this, &MemoNodeUI::OnFilterClick);
        filterBtns_.push_back(opt);
        filters->Add(opt);
    }
    root->Add(filters);

    // 列表
    list_ = nodeui::V();
    list_->SetAttribute(_T("vscrollbar"), _T("true"));
    nodeui::StyleVScrollbar(list_);
    list_->SetChildPadding(8);
    root->Add(list_);

    this->Add(root);
}

void MemoNodeUI::SaveAndNotify()
{
    DataStore::GetInstance().SaveMemos(items_);
    CNodeMessage msg(NODE_MSG_DATA_CHANGED, _T("memos"));
    PostNodeMessage(msg.msgType, msg.msgData);
}

void MemoNodeUI::Refresh()
{
    if (!list_) return;
    list_->RemoveAll();
    items_ = DataStore::GetInstance().LoadMemos();

    for (size_t i = 0; i < items_.size(); ++i) {
        MemoItem& it = items_[i];
        bool isPwd = (it.category == L"账户密码");
        if (filter_ >= 0 && it.category != Categories(filter_)) continue;

        CVerticalLayoutUI* card = nodeui::Card();
        card->SetBorderSize(0);
        card->SetBkColor(COL_CARD_BG);
        card->SetInset(CDuiRect(12, 8, 12, 8));
        card->SetChildPadding(6);
        // 分类色条
        card->SetAttribute(_T("bordersize"), _T("0,0,0,1"));
        card->SetAttribute(_T("bordercolor"), isPwd ? _T("#FFBA7517") : _T("#FF378ADD"));

        CHorizontalLayoutUI* row = nodeui::H(24);
        row->SetChildPadding(8);
        CLabelUI* title = nodeui::Label(TStrFromW(it.title).GetData(), COL_TEXT, 24, 0);
        title->SetAttribute(_T("valign"), _T("vcenter"));
        row->Add(title);
        row->Add(nodeui::Badge(TStrFromW(it.category).GetData(),
            isPwd ? COL_WARN : COL_INFO, isPwd ? COL_WARN_BG : COL_INFO_BG, 70));
        CControlUI* spacer = new CControlUI;
        row->Add(spacer);
        CLabelUI* upd = nodeui::Label(TStrFromW(it.updated).GetData(), COL_TEXT_TIP, 24, 2);
        upd->SetAttribute(_T("valign"), _T("vcenter"));
        row->Add(upd);

        TCHAR idbuf[32];
        _stprintf_s(idbuf, _T("%I64u"), it.id);

        CButtonUI* copy = nodeui::Button((CDuiString(_T("copy_")) + idbuf).GetData(), _T("复制"),
                                         COL_SURFACE, COL_BRAND, 56, 24);
        copy->SetFont(2);
        copy->OnNotify += MakeDelegate(this, &MemoNodeUI::OnRowCopy);
        row->Add(copy);
        CButtonUI* del = nodeui::Button((CDuiString(_T("mdel_")) + idbuf).GetData(), _T("✕"),
                                        COL_CARD_BG, COL_DANGER, 24, 24);
        del->OnNotify += MakeDelegate(this, &MemoNodeUI::OnRowDelete);
        row->Add(del);
        card->Add(row);

        CTextUI* content = new CTextUI;
        content->SetText(TStrFromW(it.content).GetData());
        content->SetTextColor(COL_TEXT_SUB);
        content->SetFont(2);
        content->SetAttribute(_T("multiline"), _T("true"));
        content->SetAttribute(_T("align"), _T("left"));
        card->Add(content);

        list_->Add(card);
    }
}

bool MemoNodeUI::OnCategoryClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_SELECTCHANGED) return false;
    return true;
}

bool MemoNodeUI::OnFilterClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_SELECTCHANGED) return false;
    LPCTSTR name = msg->pSender->GetName().GetData();
    if (_tcsncmp(name, _T("mfilter_"), 8) != 0) return false;
    filter_ = _ttoi(name + 8);
    Refresh();
    return true;
}

static void CopyTextToClipboard(HWND hwnd, const std::wstring& text)
{
    if (!OpenClipboard(hwnd)) return;
    EmptyClipboard();
    SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (h) {
        void* p = GlobalLock(h);
        if (p) {
            memcpy(p, text.c_str(), bytes);
            GlobalUnlock(h);
            SetClipboardData(CF_UNICODETEXT, h);
        }
    }
    CloseClipboard();
}

bool MemoNodeUI::OnRowCopy(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    uint64_t id = _wtoi64(msg->pSender->GetName().GetData() + 5);
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].id == id) {
            HWND hwnd = m_pManager ? m_pManager->GetPaintWindow() : NULL;
            CopyTextToClipboard(hwnd, items_[i].content);
            break;
        }
    }
    return true;
}

bool MemoNodeUI::OnRowDelete(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    uint64_t id = _wtoi64(msg->pSender->GetName().GetData() + 5);
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

bool MemoNodeUI::OnAddClick(void* param)
{
    TNotifyUI* param_msg = static_cast<TNotifyUI*>(param);
    if (!param_msg || param_msg->sType != DUI_MSGTYPE_CLICK) return false;
    if (!editTitle_ || !editContent_) return false;
    std::wstring title = editTitle_->GetText().GetData();
    if (title.find_first_not_of(L" \t") == std::wstring::npos) return true;

    MemoItem it;
    it.id = DataStore::GetInstance().NewId();
    it.title = title;
    // 当前选中的分类
    for (size_t i = 0; i < catBtns_.size(); ++i) {
        if (catBtns_[i]->IsSelected()) {
            it.category = catBtns_[i]->GetText().GetData();
            break;
        }
    }
    if (it.category.empty()) it.category = L"生活";
    it.content = editContent_->GetText().GetData();
    it.updated = nodeui::NowStr().GetData();
    items_.push_back(it);
    SaveAndNotify();
    Refresh();
    editTitle_->SetText(_T(""));
    editContent_->SetText(_T(""));
    return true;
}
