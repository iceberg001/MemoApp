#include "../StdAfx.h"
#include "DataStore.h"
#include <shlobj.h>

DataStore& DataStore::GetInstance()
{
    static DataStore inst;
    return inst;
}

DataStore::DataStore() : idSeed_(0)
{
    idSeed_ = (uint64_t)time(NULL) * 1000;
}

uint64_t DataStore::NewId()
{
    return ++idSeed_;
}

std::wstring WStrFromT(LPCTSTR s)
{
    return s ? std::wstring(s) : std::wstring();
}

CDuiString TStrFromW(const std::wstring& s)
{
    return CDuiString(s.c_str());
}

std::wstring TsvEscape(const std::wstring& s)
{
    std::wstring r;
    r.reserve(s.size() + 8);
    for (size_t i = 0; i < s.size(); ++i) {
        wchar_t c = s[i];
        if (c == L'\n') r += L"\x01";
        else if (c == L'\t') r += L"\x02";
        else if (c != L'\r') r += c;
    }
    return r;
}

std::wstring TsvUnescape(const std::wstring& s)
{
    std::wstring r;
    r.reserve(s.size() + 8);
    for (size_t i = 0; i < s.size(); ++i) {
        wchar_t c = s[i];
        if (c == L'\x01') r += L'\n';
        else if (c == L'\x02') r += L'\t';
        else r += c;
    }
    return r;
}

std::vector<std::wstring> TsvSplit(const std::wstring& line)
{
    std::vector<std::wstring> out;
    std::wstring cur;
    for (size_t i = 0; i < line.size(); ++i) {
        wchar_t c = line[i];
        if (c == L'\t') { out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}

std::wstring DataStore::DataDir() const
{
    TCHAR path[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L'\\');
    if (pos != std::wstring::npos) dir = dir.substr(0, pos + 1);
    dir += L"data";
    CreateDirectoryW(dir.c_str(), NULL);
    return dir;
}

std::wstring DataStore::DataFile(LPCTSTR name) const
{
    return DataDir() + L"\\" + name;
}

std::vector<std::wstring> DataStore::ReadLines(LPCTSTR file) const
{
    std::vector<std::wstring> lines;
    FILE* fp = NULL;
    if (_wfopen_s(&fp, file, L"rb") != 0 || !fp) return lines;
    std::wstring content;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, (int)n, NULL, 0);
        if (wlen > 0) {
            std::vector<wchar_t> wbuf(wlen);
            MultiByteToWideChar(CP_UTF8, 0, buf, (int)n, &wbuf[0], wlen);
            content.append(&wbuf[0], wlen);
        }
    }
    fclose(fp);
    // 跳过 UTF-8 BOM
    if (!content.empty() && content[0] == 0xFEFF) content.erase(0, 1);
    std::wstring cur;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == L'\n') { lines.push_back(cur); cur.clear(); }
        else cur += content[i];
    }
    if (!cur.empty()) lines.push_back(cur);
    return lines;
}

void DataStore::WriteLines(LPCTSTR file, const std::vector<std::wstring>& lines)
{
    FILE* fp = NULL;
    if (_wfopen_s(&fp, file, L"wb") != 0 || !fp) return;
    // UTF-8 BOM
    const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
    fwrite(bom, 1, 3, fp);
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i) {
        int len = WideCharToMultiByte(CP_UTF8, 0, lines[i].c_str(), (int)lines[i].size(), NULL, 0, NULL, NULL);
        if (len > 0) {
            std::vector<char> buf(len);
            WideCharToMultiByte(CP_UTF8, 0, lines[i].c_str(), (int)lines[i].size(), &buf[0], len, NULL, NULL);
            out.append(&buf[0], len);
        }
        out += "\n";
    }
    fwrite(out.data(), 1, out.size(), fp);
    fclose(fp);
}

std::wstring DataStore::TodayStr()
{
    time_t t = time(NULL);
    struct tm lt;
    localtime_s(&lt, &t);
    wchar_t buf[32];
    swprintf_s(buf, L"%04d-%02d-%02d", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
    return buf;
}

std::vector<TodoItem> DataStore::LoadTodos()
{
    std::vector<TodoItem> out;
    std::vector<std::wstring> lines = ReadLines(DataFile(_T("todos.txt")).c_str());
    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::wstring> f = TsvSplit(lines[i]);
        // 字段顺序与 SaveTodos 一致: id, status, progress, due, updated, title
        if (f.size() < 6) continue;
        TodoItem it;
        it.id = _wtoi64(f[0].c_str());
        it.status = _wtoi(f[1].c_str());
        if (it.status < 0 || it.status > 2) it.status = 0;
        it.progress = _wtoi(f[2].c_str());
        if (it.progress < 0) it.progress = 0;
        if (it.progress > 100) it.progress = 100;
        it.due = f[3];
        it.updated = f[4];
        it.title = TsvUnescape(f[5]);
        out.push_back(it);
    }
    return out;
}

void DataStore::SaveTodos(const std::vector<TodoItem>& items)
{
    std::vector<std::wstring> lines;
    for (size_t i = 0; i < items.size(); ++i) {
        wchar_t head[64];
        swprintf_s(head, L"%I64u\t%d\t%d\t", items[i].id, items[i].status, items[i].progress);
        std::wstring line = head;
        line += TsvEscape(items[i].due) + L"\t" + TsvEscape(items[i].updated) + L"\t" + TsvEscape(items[i].title);
        lines.push_back(line);
    }
    WriteLines(DataFile(_T("todos.txt")).c_str(), lines);
}

std::vector<MemoItem> DataStore::LoadMemos()
{
    std::vector<MemoItem> out;
    std::vector<std::wstring> lines = ReadLines(DataFile(_T("memos.txt")).c_str());
    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::wstring> f = TsvSplit(lines[i]);
        if (f.size() < 5) continue;
        MemoItem it;
        it.id = _wtoi64(f[0].c_str());
        it.category = f[1];
        it.title = TsvUnescape(f[2]);
        it.content = TsvUnescape(f[3]);
        it.updated = f[4];
        out.push_back(it);
    }
    return out;
}

void DataStore::SaveMemos(const std::vector<MemoItem>& items)
{
    std::vector<std::wstring> lines;
    for (size_t i = 0; i < items.size(); ++i) {
        wchar_t head[64];
        swprintf_s(head, L"%I64u\t", items[i].id);
        std::wstring line = head;
        line += TsvEscape(items[i].category) + L"\t" + TsvEscape(items[i].title) + L"\t"
              + TsvEscape(items[i].content) + L"\t" + TsvEscape(items[i].updated);
        lines.push_back(line);
    }
    WriteLines(DataFile(_T("memos.txt")).c_str(), lines);
}

std::vector<NoteItem> DataStore::LoadNotes()
{
    std::vector<NoteItem> out;
    std::vector<std::wstring> lines = ReadLines(DataFile(_T("notes.txt")).c_str());
    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::wstring> f = TsvSplit(lines[i]);
        if (f.size() < 5) continue;
        NoteItem it;
        it.id = _wtoi64(f[0].c_str());
        it.color = _wtoi(f[1].c_str());
        it.title = TsvUnescape(f[2]);
        it.content = TsvUnescape(f[3]);
        it.date = f[4];
        out.push_back(it);
    }
    return out;
}

void DataStore::SaveNotes(const std::vector<NoteItem>& items)
{
    std::vector<std::wstring> lines;
    for (size_t i = 0; i < items.size(); ++i) {
        wchar_t head[64];
        swprintf_s(head, L"%I64u\t%d\t", items[i].id, items[i].color);
        std::wstring line = head;
        line += TsvEscape(items[i].title) + L"\t" + TsvEscape(items[i].content) + L"\t" + TsvEscape(items[i].date);
        lines.push_back(line);
    }
    WriteLines(DataFile(_T("notes.txt")).c_str(), lines);
}
