#ifndef __REPORTTYPES_H__
#define __REPORTTYPES_H__

#include "StdAfx.h"

// 周报公共类型与周工具（LocalDB 与 MySQL 版 ReportDB 共用，不依赖任何数据库头文件）

struct ReportItem {
    int          year = 0;
    int          week = 0;
    std::wstring dateStart;    // yyyy-mm-dd
    std::wstring dateEnd;
    std::wstring done;         // 本周完成
    std::wstring plan;         // 下周计划
    std::wstring risks;        // 问题与风险
    int          status = 0;   // 0=草稿 1=已提交
    std::wstring updatedAt;
};

// ISO 8601 周计算（周一为一周开始）
struct WeekInfo {
    int year;           // ISO 年
    int week;           // ISO 周
    std::wstring start; // 周一 yyyy-mm-dd
    std::wstring end;   // 周日 yyyy-mm-dd
};
WeekInfo IsoWeekOf(int yearOffsetWeeks);  // 相对当前周偏移（0=本周，-1=上周）
std::wstring DateStr(struct tm* day);

#endif
