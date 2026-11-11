#ifndef __MEMONODE_H__
#define __MEMONODE_H__

#include "../StdAfx.h"
#include "../NodeHelper.h"
#include "../data/DataStore.h"

// 备忘录组件：分类备忘 + 账户密码记录 + 复制 + 删除
class MemoNodeUI : public DuiLib::CNodeUIBase
{
public:
    MemoNodeUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);
    bool    WantNodeMessage() const { return true; }
    void    OnNodeMessage(const DuiLib::CNodeMessage& msg);

private:
    void BuildUI();
    void Refresh();
    void SaveAndNotify();

    bool OnAddClick(void* param);
    bool OnCategoryClick(void* param);
    bool OnFilterClick(void* param);
    bool OnRowCopy(void* param);
    bool OnRowDelete(void* param);

    DuiLib::CVerticalLayoutUI*  list_;
    DuiLib::CEditUI*            editTitle_;
    DuiLib::CRichEditUI*        editContent_;
    std::vector<DuiLib::COptionUI*> catBtns_;
    std::vector<DuiLib::COptionUI*> filterBtns_;
    int                         filter_;            // -1 全部，0-2 分类
    std::vector<MemoItem>       items_;

    static LPCTSTR Categories(int i) { static LPCTSTR c[3] = { _T("账户密码"), _T("工作"), _T("生活") }; return c[i]; }
};

#endif
