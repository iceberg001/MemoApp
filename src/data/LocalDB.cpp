#include "StdAfx.h"
#include "LocalDB.h"
#include "DataStore.h"

LocalDB& LocalDB::GetInstance()
{
    static LocalDB inst;
    return inst;
}

LocalDB::LocalDB() {}
LocalDB::~LocalDB() { Close(); }

std::string LocalDB::W2U8(const std::wstring& s) const
{
    if (s.empty()) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0, NULL, NULL);
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], len, NULL, NULL);
    return out;
}

std::wstring LocalDB::U82W(const std::string& s) const
{
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    std::wstring out(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], len);
    return out;
}

void LocalDB::SetError(const char* what)
{
    if (db_) {
        wchar_t buf[512];
        swprintf_s(buf, L"%S: %S", what, sqlite3_errmsg(db_));
        lastError_ = buf;
    } else {
        lastError_ = L"数据库未打开";
    }
}

std::wstring LocalDB::DBPath() const
{
    return DataStore::GetInstance().DataDir() + L"\\reports.db";
}

std::wstring LocalDB::Open()
{
    if (db_) return L"";
    Close();

    std::wstring path = DBPath();
    int rc = sqlite3_open_v2(W2U8(path).c_str(), &db_,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        SetError("打开本地数据库失败");
        if (db_) { sqlite3_close(db_); db_ = nullptr; }
        return lastError_;
    }
    sqlite3_busy_timeout(db_, 3000);
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

    if (!EnsureTable()) return lastError_;
    lastError_.clear();
    return L"";
}

bool LocalDB::EnsureTable()
{
    const wchar_t* sql =
        L"CREATE TABLE IF NOT EXISTS weekly_reports ("
        L"  year       INTEGER NOT NULL,"
        L"  week       INTEGER NOT NULL,"
        L"  date_start TEXT,"
        L"  date_end   TEXT,"
        L"  done       TEXT,"
        L"  plan       TEXT,"
        L"  risks      TEXT,"
        L"  status     INTEGER NOT NULL DEFAULT 0,"   // 0=草稿 1=已提交
        L"  updated_at TEXT,"
        L"  PRIMARY KEY (year, week)"
        L");";
    return Exec(sql);
}

void LocalDB::Close()
{
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool LocalDB::Exec(const std::wstring& sql)
{
    lastError_.clear();
    if (!db_) { lastError_ = L"数据库未打开"; return false; }
    char* err = nullptr;
    int rc = sqlite3_exec(db_, W2U8(sql).c_str(), nullptr, nullptr, &err);
    lastRowId_ = sqlite3_last_insert_rowid(db_);
    changes_ = sqlite3_changes(db_);
    if (rc != SQLITE_OK) {
        wchar_t buf[512];
        swprintf_s(buf, L"SQL 执行失败: %S", err ? err : sqlite3_errmsg(db_));
        lastError_ = buf;
        sqlite3_free(err);
        return false;
    }
    sqlite3_free(err);
    return true;
}

std::vector<std::map<std::wstring, std::wstring>> LocalDB::Query(const std::wstring& sql)
{
    std::vector<std::map<std::wstring, std::wstring>> rows;
    lastError_.clear();
    if (!db_) { lastError_ = L"数据库未打开"; return rows; }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, W2U8(sql).c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        SetError("SQL 预处理失败");
        return rows;
    }
    int colCount = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::wstring, std::wstring> row;
        for (int i = 0; i < colCount; ++i) {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            row[U82W(sqlite3_column_name(stmt, i))] =
                text ? U82W((const char*)text) : L"";
        }
        rows.push_back(row);
    }
    sqlite3_finalize(stmt);
    return rows;
}

// ---------- 周报 CRUD ----------

bool LocalDB::UpsertReport(const ReportItem& r)
{
    if (!db_) { lastError_ = L"数据库未打开"; return false; }
    const char* sql =
        "INSERT INTO weekly_reports (year, week, date_start, date_end, done, plan, risks, status, updated_at) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9) "
        "ON CONFLICT(year, week) DO UPDATE SET "
        "date_start=?3, date_end=?4, done=?5, plan=?6, risks=?7, status=?8, updated_at=?9";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        SetError("SQL 预处理失败");
        return false;
    }
    sqlite3_bind_int(stmt, 1, r.year);
    sqlite3_bind_int(stmt, 2, r.week);
    std::string s1 = W2U8(r.dateStart), s2 = W2U8(r.dateEnd);
    std::string s3 = W2U8(r.done), s4 = W2U8(r.plan), s5 = W2U8(r.risks), s6 = W2U8(r.updatedAt);
    sqlite3_bind_text(stmt, 3, s1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, s2.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, s3.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, s4.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, s5.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, r.status);
    sqlite3_bind_text(stmt, 9, s6.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) SetError("写入周报失败");
    lastRowId_ = sqlite3_last_insert_rowid(db_);
    changes_ = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    if (ok) lastError_.clear();
    return ok;
}

std::vector<ReportItem> LocalDB::LoadReports()
{
    std::vector<ReportItem> out;
    auto rows = Query(
        L"SELECT year, week, date_start, date_end, done, plan, risks, status, updated_at "
        L"FROM weekly_reports ORDER BY year DESC, week DESC");
    for (auto& row : rows) {
        ReportItem r;
        r.year = _wtoi(row[L"year"].c_str());
        r.week = _wtoi(row[L"week"].c_str());
        r.dateStart = row[L"date_start"];
        r.dateEnd = row[L"date_end"];
        r.done = row[L"done"];
        r.plan = row[L"plan"];
        r.risks = row[L"risks"];
        r.status = _wtoi(row[L"status"].c_str());
        r.updatedAt = row[L"updated_at"];
        out.push_back(r);
    }
    return out;
}

bool LocalDB::DeleteReport(int year, int week)
{
    wchar_t sql[128];
    swprintf_s(sql, L"DELETE FROM weekly_reports WHERE year=%d AND week=%d", year, week);
    return Exec(sql);
}
