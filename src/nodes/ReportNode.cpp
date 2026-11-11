#include "../StdAfx.h"
#include "ReportNode.h"
#include "../NodeMessages.h"

using namespace DuiLib;

ReportNodeUI::ReportNodeUI()
    : views_(NULL), list_(NULL), dbLabel_(NULL), weekLabel_(NULL),
      offsetWeeks_(0), queryByWeek_(false), queryYear_(0), queryWeek_(0),
      editorLabel_(NULL), editDone_(NULL), editPlan_(NULL),
      editRisks_(NULL), editingOffset_(0)
{
    SetNodeName(_T("ReportNode"));
    BuildUI();
    RefreshList();
}

LPCTSTR ReportNodeUI::GetClass() const { return _T("ReportNodeUI"); }

LPVOID ReportNodeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("ReportNodeUI")) == 0) return static_cast<ReportNodeUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

void ReportNodeUI::OnNodeMessage(const CNodeMessage& msg)
{
}

void ReportNodeUI::BuildUI()
{
    CVerticalLayoutUI* root = nodeui::V();
    root->SetChildPadding(10);

    // 标题行：周报 + 周切换 + 新建
    CHorizontalLayoutUI* header = nodeui::H(34);
    header->SetChildPadding(8);
    CLabelUI* title = nodeui::Label(_T("周报"), COL_TEXT, 34, 1);
    title->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(title);

    CButtonUI* prev = nodeui::Button(_T("week_prev"), _T("◀"), COL_BRAND, 0xFFFFFFFF, 34, 34);
    prev->SetAttribute(_T("hotbkcolor"), _T("#FF12A279"));
    prev->SetAttribute(_T("pushedbkcolor"), _T("#FF0A5A45"));
    prev->SetAttribute(_T("bordersize"), _T("1"));
    prev->SetAttribute(_T("bordercolor"), _T("#FF0A5A45"));
    prev->OnNotify += MakeDelegate(this, &ReportNodeUI::OnPrevWeek);
    header->Add(prev);
    weekLabel_ = nodeui::Label(_T(""), COL_TEXT, 30, 2);
    weekLabel_->SetFixedWidth(250);
    weekLabel_->SetAttribute(_T("align"), _T("center"));
    weekLabel_->SetAttribute(_T("valign"), _T("vcenter"));
    header->Add(weekLabel_);
    CButtonUI* next = nodeui::Button(_T("week_next"), _T("▶"), COL_BRAND, 0xFFFFFFFF, 34, 34);
    next->SetAttribute(_T("hotbkcolor"), _T("#FF12A279"));
    next->SetAttribute(_T("pushedbkcolor"), _T("#FF0A5A45"));
    next->SetAttribute(_T("bordersize"), _T("1"));
    next->SetAttribute(_T("bordercolor"), _T("#FF0A5A45"));
    next->OnNotify += MakeDelegate(this, &ReportNodeUI::OnNextWeek);
    header->Add(next);

    // 按选中周查询历史周报 / 显示全部
    CButtonUI* btnQuery = nodeui::Button(_T("btn_query"), _T("查询"), COL_BRAND, 0xFFFFFFFF, 64, 34);
    btnQuery->SetAttribute(_T("hotbkcolor"), _T("#FF12A279"));
    btnQuery->SetAttribute(_T("pushedbkcolor"), _T("#FF0A5A45"));
    btnQuery->OnNotify += MakeDelegate(this, &ReportNodeUI::OnQuery);
    header->Add(btnQuery);
    CButtonUI* btnAll = nodeui::Button(_T("btn_query_all"), _T("全部"), COL_SURFACE, COL_TEXT, 56, 34);
    btnAll->SetAttribute(_T("bordersize"), _T("1"));
    btnAll->SetAttribute(_T("bordercolor"), _T("#FFD3D1C7"));
    btnAll->SetFont(2);
    btnAll->OnNotify += MakeDelegate(this, &ReportNodeUI::OnQueryAll);
    header->Add(btnAll);

    CControlUI* spacer = new CControlUI;
    header->Add(spacer);
    CButtonUI* btnNew = nodeui::Button(_T("btn_new_report"), _T("编写本周周报"), COL_BRAND, 0xFFFFFFFF, 120, 30);
    btnNew->OnNotify += MakeDelegate(this, &ReportNodeUI::OnNewReport);
    header->Add(btnNew);
    root->Add(header);

    // 数据库状态
    CHorizontalLayoutUI* dbRow = nodeui::H(22);
    dbLabel_ = nodeui::Label(_T("数据库连接中…"), COL_TEXT_TIP, 22, 2);
    dbLabel_->SetAttribute(_T("valign"), _T("vcenter"));
    dbRow->Add(dbLabel_);
    root->Add(dbRow);

    // 视图切换：列表 / 编辑器
    views_ = new CTabLayoutUI;
    views_->SetName(_T("reportViews"));

    // 视图一：历史周报列表
    list_ = nodeui::V();
    list_->SetAttribute(_T("vscrollbar"), _T("true"));
    nodeui::StyleVScrollbar(list_);
    list_->SetChildPadding(8);
    views_->Add(list_);

    // 视图二：编辑器
    CVerticalLayoutUI* editor = nodeui::V();
    editor->SetAttribute(_T("vscrollbar"), _T("true"));
    nodeui::StyleVScrollbar(editor);
    editor->SetChildPadding(8);

    CHorizontalLayoutUI* edHeader = nodeui::H(34);
    edHeader->SetChildPadding(8);
    CButtonUI* back = nodeui::Button(_T("btn_report_back"), _T("‹ 返回"), COL_SURFACE, COL_TEXT, 80, 30);
    back->OnNotify += MakeDelegate(this, &ReportNodeUI::OnBack);
    edHeader->Add(back);
    editorLabel_ = nodeui::Label(_T("编写周报"), COL_TEXT, 34, 1);
    editorLabel_->SetAttribute(_T("valign"), _T("vcenter"));
    edHeader->Add(editorLabel_);
    CControlUI* edSpacer = new CControlUI;
    edHeader->Add(edSpacer);
    CButtonUI* import = nodeui::Button(_T("btn_import_todo"), _T("从待办导入"), COL_INFO, 0xFFFFFFFF, 110, 30);
    import->OnNotify += MakeDelegate(this, &ReportNodeUI::OnImport);
    edHeader->Add(import);
    editor->Add(edHeader);

    editor->Add(nodeui::Label(_T("本周完成"), COL_TEXT, 22, 0));
    editDone_ = nodeui::RichEdit(_T("editDone"), 110);
    editor->Add(editDone_);

    editor->Add(nodeui::Label(_T("下周计划"), COL_TEXT, 22, 0));
    editPlan_ = nodeui::RichEdit(_T("editPlan"), 90);
    editor->Add(editPlan_);

    editor->Add(nodeui::Label(_T("问题与风险"), COL_TEXT, 22, 0));
    editRisks_ = nodeui::RichEdit(_T("editRisks"), 70);
    editor->Add(editRisks_);

    CHorizontalLayoutUI* btnRow = nodeui::H(34);
    btnRow->SetChildPadding(8);
    CButtonUI* draft = nodeui::Button(_T("btn_save_draft"), _T("保存草稿"), COL_SURFACE, COL_TEXT, 100, 30);
    draft->OnNotify += MakeDelegate(this, &ReportNodeUI::OnSaveDraft);
    btnRow->Add(draft);
    CButtonUI* submit = nodeui::Button(_T("btn_submit"), _T("提交周报"), COL_BRAND, 0xFFFFFFFF, 100, 30);
    submit->OnNotify += MakeDelegate(this, &ReportNodeUI::OnSubmit);
    btnRow->Add(submit);
    editor->Add(btnRow);
    views_->Add(editor);

    root->Add(views_);
    this->Add(root);

    UpdateWeekLabel();

    // 打开本地数据库（data\reports.db，SQLite）
    std::wstring err = LocalDB::GetInstance().Open();
    if (err.empty()) {
        { wchar_t buf[128]; swprintf_s(buf, L"本地数据库已就绪 (data\\reports.db)"); dbLabel_->SetText(buf); }
        dbLabel_->SetTextColor(COL_BRAND_MID);
        CNodeMessage msg(NODE_MSG_DB_STATUS, _T("ok"));
        CNodeUIManager::GetInstance().BroadcastMessage(msg);
    } else {
        { wchar_t buf[256]; swprintf_s(buf, L"本地数据库打开失败：%s", err.c_str()); dbLabel_->SetText(buf); }
        dbLabel_->SetTextColor(COL_DANGER);
        CNodeMessage msg(NODE_MSG_DB_STATUS, err.c_str());
        CNodeUIManager::GetInstance().BroadcastMessage(msg);
    }
}

void ReportNodeUI::UpdateWeekLabel()
{
    WeekInfo wi = IsoWeekOf(offsetWeeks_);
    CDuiString text;
    text.Format(_T("第 %d 周 (%s ~ %s)"), wi.week, wi.start.c_str(), wi.end.c_str());
    weekLabel_->SetText(text);
}

int ReportNodeUI::CountLinesOf(const std::wstring& s) const
{
    int n = 0;
    std::wstring line;
    for (size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == L'\n') {
            if (!line.empty()) ++n;
            line.clear();
        } else {
            line += s[i];
        }
    }
    return n;
}

void ReportNodeUI::BuildReportCard(const ReportItem& r)
{
    CVerticalLayoutUI* card = nodeui::Card();
    card->SetInset(CDuiRect(12, 8, 12, 8));
    card->SetChildPadding(6);

    CHorizontalLayoutUI* row1 = nodeui::H(26);
    row1->SetChildPadding(8);
    CDuiString title;
    title.Format(_T("第 %d 周周报"), r.week);
    CLabelUI* lblTitle = nodeui::Label(title, COL_TEXT, 26, 0);
    lblTitle->SetAttribute(_T("valign"), _T("vcenter"));
    row1->Add(lblTitle);
    CDuiString range;
    range.Format(_T("%s ~ %s"), r.dateStart.c_str(), r.dateEnd.c_str());
    CLabelUI* lblRange = nodeui::Label(range, COL_TEXT_TIP, 26, 2);
    lblRange->SetAttribute(_T("valign"), _T("vcenter"));
    row1->Add(lblRange);
    row1->Add(nodeui::Badge(r.status == 1 ? _T("已提交") : _T("草稿"),
        r.status == 1 ? COL_BRAND_MID : COL_WARN,
        r.status == 1 ? COL_BRAND_BG : COL_WARN_BG, 60));
    CControlUI* spacer = new CControlUI;
    row1->Add(spacer);
    CLabelUI* upd = nodeui::Label(TStrFromW(r.updatedAt).GetData(), COL_TEXT_TIP, 26, 2);
    upd->SetAttribute(_T("valign"), _T("vcenter"));
    row1->Add(upd);

    TCHAR yearbuf[16];
    _stprintf_s(yearbuf, _T("%d"), r.year);
    CDuiString base = CDuiString(yearbuf) + _T("_");
    _stprintf_s(yearbuf, _T("%d"), r.week);
    base += yearbuf;

    CButtonUI* edit = nodeui::Button((base + _T("_edit")).GetData(), _T("编辑"),
                                     COL_SURFACE, COL_BRAND, 56, 26);
    edit->SetFont(2);
    edit->OnNotify += MakeDelegate(this, &ReportNodeUI::OnRowEdit);
    row1->Add(edit);
    CButtonUI* del = nodeui::Button((base + _T("_del")).GetData(), _T("✕"),
                                    COL_CARD_BG, COL_DANGER, 26, 26);
    del->OnNotify += MakeDelegate(this, &ReportNodeUI::OnRowDelete);
    row1->Add(del);
    card->Add(row1);

    // 统计 + 摘要
    CHorizontalLayoutUI* row2 = nodeui::H(30);
    row2->SetChildPadding(10);
    CDuiString stat;
    stat.Format(_T("完成 %d 项 · 计划 %d 项 · 风险 %d 项"),
        CountLinesOf(r.done), CountLinesOf(r.plan), CountLinesOf(r.risks));
    CLabelUI* lblStat = nodeui::Label(stat, COL_TEXT_SUB, 30, 2);
    lblStat->SetAttribute(_T("valign"), _T("vcenter"));
    row2->Add(lblStat);
    card->Add(row2);

    std::wstring preview = r.done.substr(0, 60);
    if (r.done.size() > 60) preview += L"...";
    CTextUI* txt = new CTextUI;
    txt->SetText(TStrFromW(preview).GetData());
    txt->SetTextColor(COL_TEXT_TIP);
    txt->SetFont(2);
    txt->SetAttribute(_T("multiline"), _T("true"));
    card->Add(txt);

    list_->Add(card);
}

void ReportNodeUI::RefreshList()
{
    if (!list_) return;
    list_->RemoveAll();
    reports_ = LocalDB::GetInstance().LoadReports();

    if (!LocalDB::GetInstance().IsOpen()) {
        CVerticalLayoutUI* tip = nodeui::Card();
        tip->SetInset(CDuiRect(14, 12, 14, 12));
        CTextUI* txt = new CTextUI;
        txt->SetText(_T("本地数据库未打开，无法查询历史周报。\r\n请检查程序 data 目录是否可写后重启程序。"));
        txt->SetTextColor(COL_WARN);
        txt->SetFont(0);
        txt->SetAttribute(_T("multiline"), _T("true"));
        tip->Add(txt);
        list_->Add(tip);
        return;
    }

    if (queryByWeek_) {
        // 仅显示选中周（年+周匹配）的周报
        int shown = 0;
        for (size_t i = 0; i < reports_.size(); ++i) {
            ReportItem& r = reports_[i];
            if (r.year != queryYear_ || r.week != queryWeek_) continue;
            BuildReportCard(r);
            ++shown;
        }
        if (shown == 0) {
            CVerticalLayoutUI* tip = nodeui::Card();
            tip->SetInset(CDuiRect(14, 12, 14, 12));
            CDuiString txt;
            txt.Format(_T("第 %d 周 (%d 年) 暂无周报记录。\r\n可点击右上角「编写本周周报」为该周补写，或点「全部」查看所有历史周报。"),
                       queryWeek_, queryYear_);
            CTextUI* t = new CTextUI;
            t->SetText(txt);
            t->SetTextColor(COL_TEXT_TIP);
            t->SetFont(0);
            t->SetAttribute(_T("multiline"), _T("true"));
            tip->Add(t);
            list_->Add(tip);
        }
        return;
    }

    for (size_t i = 0; i < reports_.size(); ++i) {
        ReportItem& r = reports_[i];
        BuildReportCard(r);
    }

    if (reports_.empty()) {
        CVerticalLayoutUI* tip = nodeui::Card();
        tip->SetInset(CDuiRect(14, 12, 14, 12));
        CTextUI* txt = new CTextUI;
        txt->SetText(_T("暂无周报记录，点击右上角「编写本周周报」创建第一份周报。"));
        txt->SetTextColor(COL_TEXT_TIP);
        txt->SetFont(0);
        txt->SetAttribute(_T("multiline"), _T("true"));
        tip->Add(txt);
        list_->Add(tip);
    }
}

void ReportNodeUI::OpenEditor(int offsetWeeks, bool loadExisting)
{
    editingOffset_ = offsetWeeks;
    WeekInfo wi = IsoWeekOf(offsetWeeks);
    CDuiString text;
    text.Format(_T("编写周报 · 第 %d 周 (%s ~ %s)"), wi.week, wi.start.c_str(), wi.end.c_str());
    editorLabel_->SetText(text);

    // 预填已有内容
    editDone_->SetText(_T(""));
    editPlan_->SetText(_T(""));
    editRisks_->SetText(_T(""));
    for (size_t i = 0; i < reports_.size(); ++i) {
        if (reports_[i].year == wi.year && reports_[i].week == wi.week) {
            editDone_->SetText(TStrFromW(reports_[i].done).GetData());
            editPlan_->SetText(TStrFromW(reports_[i].plan).GetData());
            editRisks_->SetText(TStrFromW(reports_[i].risks).GetData());
            break;
        }
    }
    if (views_) views_->SelectItem(1);
}

static bool SaveEditor(CRichEditUI* done, CRichEditUI* plan, CRichEditUI* risks,
                       int offsetWeeks, int status, std::wstring& errOut)
{
    WeekInfo wi = IsoWeekOf(offsetWeeks);
    ReportItem r;
    r.year = wi.year;
    r.week = wi.week;
    r.dateStart = wi.start;
    r.dateEnd = wi.end;
    r.done = done->GetText().GetData();
    r.plan = plan->GetText().GetData();
    r.risks = risks->GetText().GetData();
    r.status = status;
    r.updatedAt = nodeui::NowStr().GetData();

    LocalDB& db = LocalDB::GetInstance();
    if (!db.IsOpen()) {
        errOut = L"本地数据库未打开，无法保存";
        return false;
    }
    if (!db.UpsertReport(r)) {
        errOut = L"保存失败";
        return false;
    }
    return true;
}

bool ReportNodeUI::OnPrevWeek(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    --offsetWeeks_;
    UpdateWeekLabel();
    return true;
}

bool ReportNodeUI::OnNextWeek(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    ++offsetWeeks_;
    UpdateWeekLabel();
    return true;
}

bool ReportNodeUI::OnQuery(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    WeekInfo wi = IsoWeekOf(offsetWeeks_);
    queryByWeek_ = true;
    queryYear_ = wi.year;
    queryWeek_ = wi.week;
    RefreshList();

    // 在状态行反馈查询结果
    int found = 0;
    for (size_t i = 0; i < reports_.size(); ++i) {
        if (reports_[i].year == queryYear_ && reports_[i].week == queryWeek_) ++found;
    }
    CDuiString tip;
    if (found > 0)
        tip.Format(_T("查询完成：第 %d 周 (%s ~ %s)，找到 %d 条周报"), wi.week, wi.start.c_str(), wi.end.c_str(), found);
    else
        tip.Format(_T("查询完成：第 %d 周 (%s ~ %s) 暂无周报"), wi.week, wi.start.c_str(), wi.end.c_str());
    dbLabel_->SetText(tip);
    dbLabel_->SetTextColor(found > 0 ? COL_BRAND_MID : COL_WARN);
    return true;
}

bool ReportNodeUI::OnQueryAll(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    queryByWeek_ = false;
    RefreshList();
    dbLabel_->SetText(_T("显示全部历史周报"));
    dbLabel_->SetTextColor(COL_TEXT_TIP);
    return true;
}

bool ReportNodeUI::OnNewReport(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    OpenEditor(offsetWeeks_, true);
    return true;
}

bool ReportNodeUI::OnImport(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    if (!editDone_) return false;

    // 从待办导入：取该周更新过或未完成的任务
    std::vector<TodoItem> todos = DataStore::GetInstance().LoadTodos();
    std::wstring text;
    for (size_t i = 0; i < todos.size(); ++i) {
        TodoItem& t = todos[i];
        if (t.status == 2 || t.progress > 0 || t.updated == DataStore::GetInstance().TodayStr()) {
            TCHAR line[512];
            _stprintf_s(line, L"%ls (%d%%)\r\n", t.title.c_str(), t.progress);
            text += line;
        }
    }
    if (text.empty()) text = L"（本周暂无已完成的待办任务）";
    editDone_->SetText(text.c_str());
    return true;
}

bool ReportNodeUI::OnSaveDraft(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    std::wstring err;
    if (!SaveEditor(editDone_, editPlan_, editRisks_, editingOffset_, 0, err)) {
        dbLabel_->SetText(err.c_str());
        dbLabel_->SetTextColor(COL_DANGER);
        return true;
    }
    dbLabel_->SetText(_T("草稿已保存到本地数据库"));
    dbLabel_->SetTextColor(COL_BRAND_MID);
    RefreshList();
    return true;
}

bool ReportNodeUI::OnSubmit(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    std::wstring err;
    if (!SaveEditor(editDone_, editPlan_, editRisks_, editingOffset_, 1, err)) {
        dbLabel_->SetText(err.c_str());
        dbLabel_->SetTextColor(COL_DANGER);
        return true;
    }
    dbLabel_->SetText(_T("周报已提交到本地数据库"));
    dbLabel_->SetTextColor(COL_BRAND_MID);
    RefreshList();
    if (views_) views_->SelectItem(0);
    return true;
}

bool ReportNodeUI::OnBack(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    RefreshList();
    if (views_) views_->SelectItem(0);
    return true;
}

bool ReportNodeUI::OnRowEdit(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    // 名称: year_week_edit
    LPCTSTR name = msg->pSender->GetName().GetData();
    int year = _ttoi(name);
    LPCTSTR p = wcschr(name, '_');
    if (!p) return true;
    int week = _ttoi(p + 1);
    // 计算该周相对本周的偏移
    WeekInfo cur = IsoWeekOf(0);
    int delta = (year - cur.year) * 52 + (week - cur.week);
    if (delta < -52) delta = -52;
    OpenEditor(delta, true);
    return true;
}

bool ReportNodeUI::OnRowDelete(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (!msg || msg->sType != DUI_MSGTYPE_CLICK) return false;
    LPCTSTR name = msg->pSender->GetName().GetData();
    int year = _ttoi(name);
    LPCTSTR p = wcschr(name, '_');
    if (!p) return true;
    int week = _ttoi(p + 1);
    LocalDB::GetInstance().DeleteReport(year, week);
    RefreshList();
    return true;
}
