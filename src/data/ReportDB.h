#ifndef __REPORTDB_H__
#define __REPORTDB_H__

#include "StdAfx.h"
#include "ReportTypes.h"   // ReportItem / WeekInfo / IsoWeekOf / DateStr
#include <mysql.h>

// 周报数据层（MySQL）：配置读取 mysql.ini，自动建库建表，历史周报查询

class ReportDB {
public:
    static ReportDB& GetInstance();

    // 连接并初始化库表；返回错误信息（空表示成功）
    std::wstring Connect();
    bool         IsConnected() const { return connected_; }
    std::wstring LastError() const { return lastError_; }
    void         Close();

    // 保存/更新（草稿 status=0，提交 status=1）
    bool UpsertReport(const ReportItem& r);
    // 查询历史周报（按年周倒序）
    std::vector<ReportItem> LoadReports();
    // 删除
    bool DeleteReport(int year, int week);

private:
    ReportDB();
    ~ReportDB();
    void ReadConfig();
    std::string W2U8(const std::wstring& s) const;
    std::wstring U82W(const std::string& s) const;
    std::string Escape(const std::string& s) const;

    MYSQL*   mysql_ = nullptr;
    bool     connected_ = false;
    std::wstring lastError_;

    // mysql.ini 配置
    std::string host_ = "127.0.0.1";
    int         port_ = 3306;
    std::string user_ = "root";
    std::string password_;
    std::string database_ = "memo_app";
};

#endif
