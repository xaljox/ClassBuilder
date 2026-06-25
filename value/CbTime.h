#ifndef _CBTIME_H
#define _CBTIME_H
//
// CbTime.h -- MFC-free CbTime.
//
// Replacement for MFC's CTime in the model / serialization, so those layers
// stop depending on MFC. CbTime wraps a __time64_t (absolute time, seconds
// since the 1970 epoch) -- exactly what MFC's CTime stores -- and mirrors the
// subset of the CTime API the codebase uses.
//
// Like CTime, the component accessors (GetYear/GetMonth/...) and Format()
// interpret the stored time in the LOCAL time zone, and the (year,month,day,
// ...) constructor takes local-time components.
//
#include <time.h>
#include "CbString.h"

class CbTime
{
public:
    CbTime()                 : _time(0)     {}
    CbTime(__time64_t t)     : _time(t)     {}

    CbTime(int year, int month, int day, int hour, int minute, int second)
    {
        struct tm t = {};
        t.tm_year  = year - 1900;
        t.tm_mon   = month - 1;
        t.tm_mday  = day;
        t.tm_hour  = hour;
        t.tm_min   = minute;
        t.tm_sec   = second;
        t.tm_isdst = -1;
        _time = _mktime64(&t);
    }

    static CbTime GetCurrentTime() { return CbTime(_time64(0)); }

    __time64_t GetTime() const { return _time; }

    int GetYear()   const { struct tm t; Local(t); return t.tm_year + 1900; }
    int GetMonth()  const { struct tm t; Local(t); return t.tm_mon + 1;     }
    int GetDay()    const { struct tm t; Local(t); return t.tm_mday;        }
    int GetHour()   const { struct tm t; Local(t); return t.tm_hour;        }
    int GetMinute() const { struct tm t; Local(t); return t.tm_min;         }
    int GetSecond() const { struct tm t; Local(t); return t.tm_sec;         }

    CbString Format(const char* format) const
    {
        struct tm t;
        Local(t);
        char buf[256];
        size_t n = strftime(buf, sizeof(buf), format, &t);
        buf[n] = '\0';
        return CbString(buf);
    }

    bool operator ==(const CbTime& r) const { return _time == r._time; }
    bool operator !=(const CbTime& r) const { return _time != r._time; }
    bool operator < (const CbTime& r) const { return _time <  r._time; }
    bool operator > (const CbTime& r) const { return _time >  r._time; }
    bool operator <=(const CbTime& r) const { return _time <= r._time; }
    bool operator >=(const CbTime& r) const { return _time >= r._time; }

private:
    void Local(struct tm& t) const { _localtime64_s(&t, &_time); }

    __time64_t _time;
};

#endif /* _CBTIME_H */
