#ifndef __LOCALDB_H__
#define __LOCALDB_H__

#include "StdAfx.h"
#include "ReportTypes.h"   // ReportItem / WeekInfo

// 本地数据库（SQLite 单文件 data\reports.db）：免服务、免连接配置，
// 提供通用 SQL 执行/查询与周报表的增删改查。
// MySQL 版实现保留在 ReportDB，可通过宏 USE_MYSQL_DB 切换。

#include <sqlite3.h>

class LocalDB {
public:
    static LocalDB& GetInstance();

    // 打开（不存在则自动创建）data\reports.db 并建表；返回错误信息（空=成功）
    std::wstring Open();
    bool         IsOpen() const { return db_ != nullptr; }
    std::wstring LastError() const { return lastError_; }
    void         Close();

    // ---- 通用 SQL（供扩展其他表使用）----
    // 执行无结果集语句（INSERT/UPDATE/DELETE/CREATE ...），返回是否成功
    bool Exec(const std::wstring& sql);
    // 查询：按列名取值，每行一个 map（列名 -> UTF-16 值）
    std::vector<std::map<std::wstring, std::wstring>>
        Query(const std::wstring& sql);
    // 取最近一次 Exec/Query 的错误信息
    long long LastInsertRowId() const { return lastRowId_; }
    int  Changes() const { return changes_; }

    // ---- 周报 CRUD ----
    bool UpsertReport(const ReportItem& r);              // 增/改（按年+周主键）
    std::vector<ReportItem> LoadReports();               // 查（按年周倒序）
    bool DeleteReport(int year, int week);               // 删

private:
    LocalDB();
    ~LocalDB();
    LocalDB(const LocalDB&) = delete;
    LocalDB& operator=(const LocalDB&) = delete;

    std::wstring DBPath() const;                         // exe 同目录 data\reports.db
    bool EnsureTable();
    std::string W2U8(const std::wstring& s) const;
    std::wstring U82W(const std::string& s) const;
    void SetError(const char* what);

    sqlite3*     db_ = nullptr;
    std::wstring lastError_;
    long long    lastRowId_ = 0;
    int          changes_ = 0;
};

#endif
