#include "../StdAfx.h"
#include "ReportDB.h"

// 周工具（DateStr / IsoWeekOf）已抽到 WeekUtil.cpp 公共实现

// ---------------- ReportDB ----------------

ReportDB& ReportDB::GetInstance()
{
    static ReportDB inst;
    return inst;
}

ReportDB::ReportDB()
{
    ReadConfig();
}

ReportDB::~ReportDB()
{
    Close();
}

std::string ReportDB::W2U8(const std::wstring& s) const
{
    if (s.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0, NULL, NULL);
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], len, NULL, NULL);
    return out;
}

std::wstring ReportDB::U82W(const std::string& s) const
{
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    std::wstring out(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], len);
    return out;
}

std::string ReportDB::Escape(const std::string& s) const
{
    if (!mysql_) return s;
    std::string out(s.size() * 2 + 1, 0);
    unsigned long n = mysql_real_escape_string(mysql_, &out[0], s.c_str(), (unsigned long)s.size());
    out.resize(n);
    return out;
}

void ReportDB::ReadConfig()
{
    TCHAR path[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L'\\');
    if (pos != std::wstring::npos) dir = dir.substr(0, pos + 1);
    dir += L"mysql.ini";

    if (GetFileAttributesW(dir.c_str()) == INVALID_FILE_ATTRIBUTES) {
        // 生成默认配置文件
        FILE* fp = NULL;
        if (_wfopen_s(&fp, dir.c_str(), L"wb") == 0 && fp) {
            fprintf(fp, "[mysql]\r\nhost=127.0.0.1\r\nport=3306\r\nuser=root\r\npassword=\r\ndatabase=memo_app\r\n");
            fclose(fp);
        }
        return;
    }
    TCHAR buf[256] = { 0 };
    const wchar_t* p = dir.c_str();
    GetPrivateProfileStringW(L"mysql", L"host", L"127.0.0.1", buf, 256, p); host_ = W2U8(buf);
    port_ = GetPrivateProfileIntW(L"mysql", L"port", 3306, p);
    GetPrivateProfileStringW(L"mysql", L"user", L"root", buf, 256, p); user_ = W2U8(buf);
    GetPrivateProfileStringW(L"mysql", L"password", L"", buf, 256, p); password_ = W2U8(buf);
    GetPrivateProfileStringW(L"mysql", L"database", L"memo_app", buf, 256, p); database_ = W2U8(buf);
}

std::wstring ReportDB::Connect()
{
    if (connected_) return L"";
    Close();

    // 调试日志
    {
        wchar_t exepath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, exepath, MAX_PATH);
        std::wstring logpath = exepath;
        size_t pos = logpath.find_last_of(L'\\');
        if (pos != std::wstring::npos) logpath = logpath.substr(0, pos + 1);
        logpath += L"db.log";
        FILE* fp = NULL;
        _wfopen_s(&fp, logpath.c_str(), L"a");
        if (fp) {
            fprintf(fp, "[connect] host=%s port=%d user=%s\n", host_.c_str(), port_, user_.c_str());
            fclose(fp);
        }
    }

    mysql_ = mysql_init(NULL);
    if (!mysql_) { lastError_ = L"MySQL 初始化失败"; return lastError_; }
    const char* charset = "utf8mb4";
    mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, charset);

    if (!mysql_real_connect(mysql_, host_.c_str(), user_.c_str(), password_.c_str(),
                            NULL, port_, NULL, 0)) {
        wchar_t msg[512];
        swprintf_s(msg, L"连接失败(%s:%d): %S", host_.c_str(), port_, mysql_error(mysql_));
        lastError_ = msg;
        {
            wchar_t exepath[MAX_PATH] = { 0 };
            GetModuleFileNameW(NULL, exepath, MAX_PATH);
            std::wstring logpath = exepath;
            size_t pos = logpath.find_last_of(L'\\');
            if (pos != std::wstring::npos) logpath = logpath.substr(0, pos + 1);
            logpath += L"db.log";
            FILE* fp = NULL;
            _wfopen_s(&fp, logpath.c_str(), L"a");
            if (fp) {
                fprintf(fp, "[error] %s\n", mysql_error(mysql_));
                fclose(fp);
            }
        }
        mysql_close(mysql_);
        mysql_ = NULL;
        return lastError_;
    }

    // 建库
    std::string sql = "CREATE DATABASE IF NOT EXISTS `" + database_ +
                      "` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci";
    if (mysql_query(mysql_, sql.c_str())) {
        U82W(mysql_error(mysql_));
        wchar_t msg[512];
        swprintf_s(msg, L"建库失败: %S", mysql_error(mysql_));
        lastError_ = msg;
        mysql_close(mysql_);
        mysql_ = NULL;
        return lastError_;
    }
    mysql_select_db(mysql_, database_.c_str());

    // 建表
    const char* createTable =
        "CREATE TABLE IF NOT EXISTS weekly_reports ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  report_year INT NOT NULL,"
        "  report_week INT NOT NULL,"
        "  date_start VARCHAR(16) DEFAULT '',"
        "  date_end VARCHAR(16) DEFAULT '',"
        "  done MEDIUMTEXT,"
        "  plan MEDIUMTEXT,"
        "  risks MEDIUMTEXT,"
        "  status TINYINT DEFAULT 0,"
        "  updated_at VARCHAR(32) DEFAULT '',"
        "  UNIQUE KEY uq_week (report_year, report_week)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";
    if (mysql_query(mysql_, createTable)) {
        wchar_t msg[512];
        swprintf_s(msg, L"建表失败: %S", mysql_error(mysql_));
        lastError_ = msg;
        mysql_close(mysql_);
        mysql_ = NULL;
        return lastError_;
    }

    connected_ = true;
    lastError_.clear();
    return L"";
}

void ReportDB::Close()
{
    if (mysql_) {
        mysql_close(mysql_);
        mysql_ = NULL;
    }
    connected_ = false;
}

bool ReportDB::UpsertReport(const ReportItem& r)
{
    if (!connected_) return false;
    char sql[8192];
    std::string done = Escape(W2U8(r.done));
    std::string plan = Escape(W2U8(r.plan));
    std::string risks = Escape(W2U8(r.risks));
    std::string start = Escape(W2U8(r.dateStart));
    std::string end = Escape(W2U8(r.dateEnd));
    std::string upd = Escape(W2U8(r.updatedAt));
    _snprintf_s(sql, sizeof(sql), _TRUNCATE,
        "INSERT INTO weekly_reports (report_year, report_week, date_start, date_end, done, plan, risks, status, updated_at)"
        " VALUES (%d, %d, '%s', '%s', '%s', '%s', '%s', %d, '%s')"
        " ON DUPLICATE KEY UPDATE date_start='%s', date_end='%s', done='%s', plan='%s', risks='%s', status=%d, updated_at='%s'",
        r.year, r.week, start.c_str(), end.c_str(), done.c_str(), plan.c_str(), risks.c_str(),
        r.status, upd.c_str(),
        start.c_str(), end.c_str(), done.c_str(), plan.c_str(), risks.c_str(), r.status, upd.c_str());
    return mysql_query(mysql_, sql) == 0;
}

bool ReportDB::DeleteReport(int year, int week)
{
    if (!connected_) return false;
    char sql[256];
    _snprintf_s(sql, sizeof(sql), _TRUNCATE,
        "DELETE FROM weekly_reports WHERE report_year=%d AND report_week=%d", year, week);
    return mysql_query(mysql_, sql) == 0;
}

std::vector<ReportItem> ReportDB::LoadReports()
{
    std::vector<ReportItem> out;
    if (!connected_) return out;
    if (mysql_query(mysql_, "SELECT report_year, report_week, date_start, date_end, done, plan, risks, status, updated_at"
                            " FROM weekly_reports ORDER BY report_year DESC, report_week DESC")) {
        return out;
    }
    MYSQL_RES* res = mysql_store_result(mysql_);
    if (!res) return out;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        ReportItem r;
        r.year = row[0] ? atoi(row[0]) : 0;
        r.week = row[1] ? atoi(row[1]) : 0;
        r.dateStart = row[2] ? U82W(row[2]) : L"";
        r.dateEnd = row[3] ? U82W(row[3]) : L"";
        r.done = row[4] ? U82W(row[4]) : L"";
        r.plan = row[5] ? U82W(row[5]) : L"";
        r.risks = row[6] ? U82W(row[6]) : L"";
        r.status = row[7] ? atoi(row[7]) : 0;
        r.updatedAt = row[8] ? U82W(row[8]) : L"";
        out.push_back(r);
    }
    mysql_free_result(res);
    return out;
}
