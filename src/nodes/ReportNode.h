#ifndef __REPORTNODE_H__
#define __REPORTNODE_H__

#include "../StdAfx.h"
#include "../NodeHelper.h"
#include "../data/DataStore.h"
#include "../data/LocalDB.h"

// 周报组件：编写周报（可从待办自动汇总）+ MySQL 历史周报查询
class ReportNodeUI : public DuiLib::CNodeUIBase
{
public:
    ReportNodeUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);
    bool    WantNodeMessage() const { return true; }
    void    OnNodeMessage(const DuiLib::CNodeMessage& msg);

private:
    void BuildUI();
    void RefreshList();
    void BuildReportCard(const ReportItem& r);
    void UpdateWeekLabel();
    void OpenEditor(int offsetWeeks, bool loadExisting);
    int  CountLinesOf(const std::wstring& s) const;
    int  CountLines(const std::wstring& s) const;

    bool OnPrevWeek(void* param);
    bool OnNextWeek(void* param);
    bool OnQuery(void* param);
    bool OnQueryAll(void* param);
    bool OnNewReport(void* param);
    bool OnImport(void* param);
    bool OnSaveDraft(void* param);
    bool OnSubmit(void* param);
    bool OnBack(void* param);
    bool OnRowEdit(void* param);
    bool OnRowDelete(void* param);

    // 视图切换
    DuiLib::CTabLayoutUI*       views_;
    DuiLib::CVerticalLayoutUI*  list_;
    DuiLib::CLabelUI*           dbLabel_;
    DuiLib::CLabelUI*           weekLabel_;
    int                         offsetWeeks_;   // 0=本周 -1=上周 ...
    bool                        queryByWeek_;   // 是否按选中周过滤历史列表
    int                         queryYear_;     // 查询的年份
    int                         queryWeek_;     // 查询的周数

    // 编辑器
    DuiLib::CLabelUI*           editorLabel_;
    DuiLib::CRichEditUI*        editDone_;
    DuiLib::CRichEditUI*        editPlan_;
    DuiLib::CRichEditUI*        editRisks_;
    int                         editingOffset_; // 当前编辑的周

    std::vector<ReportItem>     reports_;
};

#endif
