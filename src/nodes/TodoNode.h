#ifndef __TODONODE_H__
#define __TODONODE_H__

#include "../StdAfx.h"
#include "../NodeHelper.h"
#include "../data/DataStore.h"

// 待办事项组件：任务列表 + 进度条 + 统计 + 筛选 + 增删改
class TodoNodeUI : public DuiLib::CNodeUIBase
{
public:
    TodoNodeUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);
    bool    WantNodeMessage() const { return true; }
    void    OnNodeMessage(const DuiLib::CNodeMessage& msg);

private:
    void BuildUI();
    void Refresh();
    void UpdateStats();
    void SaveAndNotify();

    bool OnAddClick(void* param);
    bool OnFilterClick(void* param);
    bool OnRowCheck(void* param);
    bool OnRowDelete(void* param);
    bool OnSliderChanged(void* param);

    DuiLib::CVerticalLayoutUI*  list_;
    DuiLib::CLabelUI*           statToday_;
    DuiLib::CLabelUI*           statDoing_;
    DuiLib::CLabelUI*           statDone_;
    DuiLib::CRichEditUI*            editNew_;
    std::vector<DuiLib::CButtonUI*> filterBtns_;
    int                         filter_;    // -1 全部, 0/1/2 状态
    std::vector<TodoItem>       items_;
};

#endif
