#ifndef __NOTENODE_H__
#define __NOTENODE_H__

#include "../StdAfx.h"
#include "../NodeHelper.h"
#include "../data/DataStore.h"

// 便签组件：多彩便签墙 + 添加 + 删除
class NoteNodeUI : public DuiLib::CNodeUIBase
{
public:
    NoteNodeUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);
    bool    WantNodeMessage() const { return true; }
    void    OnNodeMessage(const DuiLib::CNodeMessage& msg);

private:
    void BuildUI();
    void Refresh();
    void SaveAndNotify();

    bool OnAddClick(void* param);
    bool OnColorClick(void* param);
    bool OnRowDelete(void* param);

    DuiLib::CTileLayoutUI*      wall_;
    DuiLib::CEditUI*            editTitle_;
    DuiLib::CRichEditUI*        editContent_;
    std::vector<DuiLib::COptionUI*> colorBtns_;
    int                         color_;     // 当前选中颜色 0-4
    std::vector<NoteItem>       items_;

    static DWORD NoteBg(int c);
};

#endif
