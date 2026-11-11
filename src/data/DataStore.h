#ifndef __DATASTORE_H__
#define __DATASTORE_H__

#include "StdAfx.h"

// 本地文件数据层（待办 / 备忘录 / 便签），TSV 转义存储于 exe 同目录 data\ 下

struct TodoItem {
    uint64_t    id = 0;
    std::wstring title;
    int         status = 0;     // 0=未开始 1=进行中 2=已完成
    int         progress = 0;   // 0-100
    std::wstring due;           // 截止日期 yyyy-mm-dd
    std::wstring updated;
};

struct MemoItem {
    uint64_t    id = 0;
    std::wstring category;      // 账户密码 / 工作 / 生活
    std::wstring title;
    std::wstring content;
    std::wstring updated;
};

struct NoteItem {
    uint64_t    id = 0;
    int         color = 0;      // 0琥珀 1青绿 2蓝 3粉 4绿
    std::wstring title;
    std::wstring content;
    std::wstring date;
};

class DataStore {
public:
    static DataStore& GetInstance();

    std::vector<TodoItem>  LoadTodos();
    void                   SaveTodos(const std::vector<TodoItem>& items);
    std::vector<MemoItem>  LoadMemos();
    void                   SaveMemos(const std::vector<MemoItem>& items);
    std::vector<NoteItem>  LoadNotes();
    void                   SaveNotes(const std::vector<NoteItem>& items);

    uint64_t NewId();
    std::wstring TodayStr();
    std::wstring DataDir() const;   // exe 同目录 data\（自动创建）

private:
    DataStore();
    std::wstring DataFile(LPCTSTR name) const;
    std::vector<std::wstring> ReadLines(LPCTSTR file) const;
    void WriteLines(LPCTSTR file, const std::vector<std::wstring>& lines);

    uint64_t idSeed_;
};

// TSV 转义工具（\n -> \x01, \t -> \x02）
std::wstring TsvEscape(const std::wstring& s);
std::wstring TsvUnescape(const std::wstring& s);
std::vector<std::wstring> TsvSplit(const std::wstring& line);
std::wstring WStrFromT(LPCTSTR s);
CDuiString    TStrFromW(const std::wstring& s);

#endif
