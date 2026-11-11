// 周工具（从 MySQL 版 ReportDB.cpp 抽出的公共实现，供 LocalDB 版周报使用）
#include "ReportTypes.h"

std::wstring DateStr(struct tm* day)
{
    wchar_t buf[32];
    swprintf_s(buf, L"%04d-%02d-%02d", day->tm_year + 1900, day->tm_mon + 1, day->tm_mday);
    return buf;
}

// 本地时间日期加减天数。统一用 mktime（本地时间）规范化，
// 不能用 _mkgmtime（按 UTC 解释本地时间字段，UTC+8 时区每次往返漂移 8 小时，
// 累加后日期会多出一天）。
static struct tm AddDays(struct tm base, int days)
{
    base.tm_mday += days;
    base.tm_isdst = -1;
    time_t t = mktime(&base);
    struct tm out;
    localtime_s(&out, &t);
    return out;
}

// ISO 8601: 周一为第一天，第一周是包含 1月4日（即含第一个周四）的那一周
WeekInfo IsoWeekOf(int yearOffsetWeeks)
{
    time_t now = time(NULL);
    struct tm base;
    localtime_s(&base, &now);
    // 回到本周周一（tm_wday: 0=周日）
    int backToMonday = (base.tm_wday == 0) ? -6 : (1 - base.tm_wday);
    struct tm monday = AddDays(base, backToMonday + yearOffsetWeeks * 7);

    // ISO 年：以该周四所在的年份为准
    struct tm thursday = AddDays(monday, 3);
    int isoYear = thursday.tm_year + 1900;

    // ISO 第 1 周的周四 = 「1月4日所在周的周四」。
    // 周号必须与这个周四相减（两者都是周四，差值恰为 7 天的整数倍）。
    // 若直接与 1月4日 相减，因 1月4日 weekday 不定，整除 7 天会向下取错周号。
    struct tm jan4 = { 0 };
    jan4.tm_year = isoYear - 1900;
    jan4.tm_mon = 0;
    jan4.tm_mday = 4;
    jan4.tm_isdst = -1;
    mktime(&jan4);                                     // 规范化并填充 tm_wday
    int backToMon1 = (jan4.tm_wday == 0) ? -6 : (1 - jan4.tm_wday);
    struct tm thu1 = AddDays(jan4, backToMon1 + 3);    // 第 1 周的周四

    struct tm a = thursday, b = thu1;
    a.tm_isdst = -1; b.tm_isdst = -1;
    time_t tThu = mktime(&a);
    time_t tThu1 = mktime(&b);
    int week = (int)((tThu - tThu1) / 86400 / 7) + 1;
    if (week < 1) week = 1;

    WeekInfo wi;
    wi.year = isoYear;
    wi.week = week;
    wi.start = DateStr(&monday);
    struct tm sunday = AddDays(monday, 6);
    wi.end = DateStr(&sunday);
    return wi;
}
