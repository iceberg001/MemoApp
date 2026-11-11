#include "../StdAfx.h"
#include "NoteNode.h"
#include "../NodeMessages.h"

using namespace DuiLib;

DWORD NoteNodeUI::NoteBg(int c)
{
    static DWORD colors[5] = { COL_NOTE_AMBER, COL_NOTE_TEAL, COL_NOTE_BLUE, COL_NOTE_PINK, COL_NOTE_GREEN };
    if (c < 0 || c > 4) c = 0;
    return colors[c];
}

NoteNodeUI::NoteNodeUI()
    : wall_(NULL), editTitle_(NULL), editContent_(NULL), color_(0)
{
    SetNodeName(_T("NoteNode"));
    BuildUI();
    Refresh();
}

LPCTSTR NoteNodeUI::GetClass() const { return _T("NoteNodeUI"); }

LPVOID NoteNodeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("NoteNodeUI")) == 0) return static_cast<NoteNodeUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

void NoteNodeUI::OnNodeMessage(const CNodeMessage& msg)
{
}

void NoteNodeUI::BuildUI()
{
    CVerticalLayoutUI* root = nodeui::V();
    root->SetChildPadding(10);

    // 标题行
    CHorizontalLayoutUI* header = nodeui::H(34);
    header->SetChildPadding(8);
    CLabelUI* title = nodeui::Label(_T("便签"), COL_TEXT, 34, 1);
    title->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(title);
    CControlUI* spacer = new CControlUI;
    header->Add(spacer);
    root->Add(header);

    // 新建便签区
    CVerticalLayoutUI* addCard = nodeui::Card();
    addCard->SetInset(CDuiRect(12, 10, 12, 10));
    addCard->SetChildPadding(8);
    CHorizontalLayoutUI* addRow1 = nodeui::H(30);
    addRow1->SetChildPadding(8);
    editTitle_ = nodeui::Edit(_T("noteTitle"), 0, 30);
    editTitle_->SetAttribute(_T("bordersize"), _T("1"));
    editTitle_->SetAttribute(_T("bordercolor"), _T("#FF7A7A7A"));
    addRow1->Add(editTitle_);
    // 颜色选择
    for (int i = 0; i < 5; ++i) {
        COptionUI* opt = new COptionUI;
        opt->SetGroup(_T("noteColor"));
        opt->SetFixedWidth(24);
        opt->SetFixedHeight(24);
        opt->SetBkColor(NoteBg(i));
        opt->SetAttribute(_T("selectedcolor"), _T("#FF2C2C2A"));
        if (i == 0) opt->SetAttribute(_T("selected"), _T("true"));
        opt->OnNotify += MakeDelegate(this, &NoteNodeUI::OnColorClick);
        colorBtns_.push_back(opt);
        addRow1->Add(opt);
    }
    CControlUI* spacer1 = new CControlUI;
    addRow1->Add(spacer1);
    CButtonUI* btnAdd = nodeui::Button(_T("btn_add_note"), _T("+ 新建便签"), COL_BRAND, 0xFFFFFFFF, 110, 30);
    btnAdd->OnNotify += MakeDelegate(this, &NoteNodeUI::OnAddClick);
    addRow1->Add(btnAdd);
    addCard->Add(addRow1);

    editContent_ = nodeui::RichEdit(_T("noteContent"), 64);
    editContent_->SetAttribute(_T("bordersize"), _T("1"));
    editContent_->SetAttribute(_T("bordercolor"), _T("#FF7A7A7A"));
    addCard->Add(editContent_);
    root->Add(addCard);
	
    // 便签墙（Tile 自动换列）
    wall_ = new CTileLayoutUI;
    wall_->SetAttribute(_T("vscrollbar"), _T("true"));
    nodeui::StyleVScrollbar(wall_);
    wall_->SetChildPadding(10);
    wall_->SetInset(CDuiRect(0, 4, 0, 4));
    root->Add(wall_);

    this->Add(root);
}

void NoteNodeUI::SaveAndNotify()
{
    DataStore::GetInstance().SaveNotes(items_);
    CNodeMessage msg(NODE_MSG_DATA_CHANGED, _T("notes"));
    PostNodeMessage(msg.msgType, msg.msgData);
}

void NoteNodeUI::Refresh()
{
    if (!wall_) return;
    wall_->RemoveAll();
    items_ = DataStore::GetInstance().LoadNotes();

    for (size_t i = 0; i < items_.size(); ++i) {
        NoteItem& it = items_[i];
        CVerticalLayoutUI* card = nodeui::V(170);
        card->SetBkColor(NoteBg(it.color));
        card->SetInset(CDuiRect(12, 10, 12, 10));
        card->SetChildPadding(6);
        card->SetFixedWidth(270);

        CHorizontalLayoutUI* row = nodeui::H(24);
        row->SetChildPadding(6);
        CLabelUI* title = nodeui::Label(TStrFromW(it.title).GetData(), COL_TEXT, 24, 0);
        title->SetAttribute(_T("valign"), _T("vcenter"));
        row->Add(title);
        CControlUI* spacer = new CControlUI;
        row->Add(spacer);
        TCHAR idbuf[32];
        _stprintf_s(idbuf, _T("%I64u"), it.id);
        CButtonUI* del = nodeui::Button((CDuiString(_T("ndel_")) + idbuf).GetData(), _T("✕"),
                                        NoteBg(it.color), COL_TEXT_SUB, 24, 24);
        del->OnNotify += MakeDelegate(this, &NoteNodeUI::OnRowDelete);
        row->Add(del);
        card->Add(row);

        CTextUI* content = new CTextUI;
        content->SetText(TStrFromW(it.content).GetData());
        content->SetTextColor(COL_TEXT_SUB);
        content->SetFont(2);
        content->SetAttribute(_T("multiline"), _T("true"));
        content->SetAttribute(_T("align"), _T("left"));
        card->Add(content);

        CLabelUI* date = nodeui::Label(TStrFromW(it.date).GetData(), COL_TEXT_TIP, 18, 2);
        card->Add(date);

        wall_->Add(card);
    }
}

bool NoteNodeUI::OnColorClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_SELECTCHANGED) return false;
    for (size_t i = 0; i < colorBtns_.size(); ++i) {
        if (colorBtns_[i] == msg->pSender) { color_ = (int)i; break; }
    }
    return true;
}

bool NoteNodeUI::OnRowDelete(void* param)
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

bool NoteNodeUI::OnAddClick(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    if (!editTitle_ || !editContent_) return false;
    std::wstring title = editTitle_->GetText().GetData();
    if (title.find_first_not_of(L" \t") == std::wstring::npos) return true;

    NoteItem it;
    it.id = DataStore::GetInstance().NewId();
    it.title = title;
    it.color = color_;
    it.content = editContent_->GetText().GetData();
    it.date = DataStore::GetInstance().TodayStr();
    items_.push_back(it);
    SaveAndNotify();
    Refresh();
    editTitle_->SetText(_T(""));
    editContent_->SetText(_T(""));
    return true;
}
