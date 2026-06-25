#ifndef _CBSTRING_H
#define _CBSTRING_H
//
// CbString -- an MFC-free string with a CString-compatible API.
//
// This is the replacement for MFC's CString in the model / codegen /
// serialization layers, so those layers stop depending on MFC and the (Qt)
// GUI can use the model directly. It is deliberately:
//
//  * MFC-free       -- no afx*, no <windows.h>; includable from Qt code.
//  * pointer-sized  -- the object is EXACTLY one char* (8 bytes), like MFC's
//                      CString. Length and capacity live in a header in the
//                      heap block, *before* the chars. This is essential:
//                      the codebase passes strings straight into printf-style
//                      varargs (`Format("%s", aString)`), which only works if
//                      the object is one pointer wide. A fatter object would
//                      misalign every following vararg.  <-- learned the hard
//                      way: an earlier {char*,int,int} CbString broke this.
//  * capacity-managed -- geometric growth on append, amortised O(1) build;
//                      absorbs the role of the old MyCString perf subclass.
//
// API mirrors the subset of CString the codebase actually uses.
//
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdint>   // intptr_t / int32_t etc. -- portable, windows.h-free integer types

#ifndef _WIN32
#include <strings.h>           // strcasecmp / strncasecmp (POSIX)
#define _stricmp  strcasecmp   // MSVC case-insensitive compare -> POSIX spelling
#define _strnicmp strncasecmp
#endif

class CbArchive;

class CbString
{
    friend class CbArchive;

public:
    CbString();
    CbString(const char* str);
    CbString(const char* str, int length);
    CbString(char c, int repeat = 1);
    CbString(const CbString& str);
#ifdef __AFX_H__
    // MFC bridge -- compiled only in translation units that already pulled in
    // MFC (afx.h). Lets surviving MFC code (the dialogs, until the Qt port
    // deletes them) pass a CString straight to the CbString-based model with
    // no explicit conversion. For Qt / non-MFC code this whole bridge is
    // skipped, so CbString.h stays strictly MFC-free.
    CbString(const CString& s) : _data(EmptyData())
    {
        Assign((const char*)s, s.GetLength());
    }
    // ...and the reverse: lets a CbString be passed to MFC APIs and other
    // CString-typed code. Most CString-overloaded MFC calls (ExtTextOut,
    // GetTextExtent, ...) become viable again through this.
    operator CString() const { return CString(_data); }
#endif
    ~CbString();

    const CbString& operator =(const CbString& str);
    const CbString& operator =(const char* str);
    const CbString& operator =(char c);

    const CbString& operator +=(const CbString& str);
    const CbString& operator +=(const char* str);
    const CbString& operator +=(char c);

    operator const char*() const { return _data; }
    const char* c_str()    const { return _data; }

    int  GetLength() const { return Hdr()->length; }
    bool IsEmpty()   const { return Hdr()->length == 0; }
    void Empty();

    char operator [](int index) const { return GetAt(index); }
    char GetAt(int index) const;
    void SetAt(int index, char c);

    int  Compare(const char* str)       const;
    int  CompareNoCase(const char* str) const;

    int  Find(char c, int start = 0)          const;
    int  Find(const char* sub, int start = 0) const;
    int  ReverseFind(char c)                  const;
    int  FindOneOf(const char* set)           const;

    CbString Left(int count)             const;
    CbString Right(int count)            const;
    CbString Mid(int first)              const;
    CbString Mid(int first, int count)   const;

    void MakeUpper();
    void MakeLower();
    void MakeReverse();

    CbString& TrimLeft();
    CbString& TrimRight();
    CbString& Trim();

    int  Delete(int index, int count = 1);
    int  Remove(char c);
    int  Replace(char chOld, char chNew);
    int  Replace(const char* strOld, const char* strNew);

    void Format(const char* format, ...);
    void FormatV(const char* format, va_list args);

    // CString-compatible raw buffer access, plus Preallocate -- the perf hook
    // that replaces MyCString (reserve a big buffer up front for hot loops).
    char* GetBuffer(int minLength);
    char* GetBufferSetLength(int newLength);
    void  ReleaseBuffer(int newLength = -1);
    void  Preallocate(int capacityChars);

private:
    // Heap block layout: [Header][chars ...][NUL]. _data points at the chars,
    // so the Header sits at _data - sizeof(Header). capacity counts char
    // slots, excluding the NUL.
    struct Header { int length; int capacity; };

    char* _data;   // never null -- empty strings share EmptyData()

    // Shared, read-only empty string: a Header{0,0} followed by one NUL.
    static char* EmptyData()
    {
        static struct Empty { Header h; char nul; } e = { { 0, 0 }, '\0' };
        return &e.nul;
    }
    Header* Hdr()       const { return reinterpret_cast<Header*>(_data) - 1; }
    const char* Ptr()   const { return _data; }
    bool IsShared()     const { return _data == EmptyData(); }
    void Free()         { if (_data != EmptyData()) delete[] reinterpret_cast<char*>(Hdr()); }
    void SetLen(int n)  { Hdr()->length = n; _data[n] = '\0'; }
    void Reserve(int minChars);                 // ensure a writable block
    void Assign(const char* str, int length);
    CbString(const CbString& a, const CbString& b);   // concatenating ctor

    friend CbString operator +(const CbString& a, const CbString& b);
    friend CbString operator +(const CbString& a, const char* b);
    friend CbString operator +(const char* a, const CbString& b);
    friend CbString operator +(const CbString& a, char b);
    friend CbString operator +(char a, const CbString& b);
};

// ===========================================================================
// CbString -- inline implementation
// ===========================================================================

// Ensure a uniquely-owned, writable block with room for at least minChars
// characters (plus the NUL). Preserves existing content.
inline void CbString::Reserve(int minChars)
{
    if (!IsShared() && Hdr()->capacity >= minChars)
        return;
    const int oldLen = Hdr()->length;             // 0 on the shared empty
    int cap = IsShared() ? 0 : Hdr()->capacity;
    if (cap < 16)
        cap = 16;
    while (cap < minChars)
        cap *= 2;
    char* block = new char[sizeof(Header) + cap + 1];
    Header* h   = reinterpret_cast<Header*>(block);
    h->capacity = cap;
    h->length   = oldLen;
    char* nd    = block + sizeof(Header);
    memcpy(nd, _data, oldLen);                     // copy existing chars
    nd[oldLen]  = '\0';
    Free();
    _data = nd;
}

inline void CbString::Assign(const char* str, int length)
{
    Reserve(length);
    if (length)
        memcpy(_data, str, length);
    SetLen(length);
}

inline CbString::CbString() : _data(EmptyData())
{
}

inline CbString::CbString(const char* str) : _data(EmptyData())
{
    Assign(str, str ? int(strlen(str)) : 0);
}

inline CbString::CbString(const char* str, int length) : _data(EmptyData())
{
    Assign(str ? str : "", str ? length : 0);
}

inline CbString::CbString(char c, int repeat) : _data(EmptyData())
{
    if (repeat > 0)
    {
        Reserve(repeat);
        for (int i = 0; i < repeat; i++)
            _data[i] = c;
        SetLen(repeat);
    }
}

inline CbString::CbString(const CbString& str) : _data(EmptyData())
{
    Assign(str._data, str.Hdr()->length);
}

inline CbString::CbString(const CbString& a, const CbString& b) : _data(EmptyData())
{
    const int la = a.Hdr()->length, lb = b.Hdr()->length;
    Reserve(la + lb);
    if (la) memcpy(_data,      a._data, la);
    if (lb) memcpy(_data + la, b._data, lb);
    SetLen(la + lb);
}

inline CbString::~CbString()
{
    Free();
}

inline const CbString& CbString::operator =(const CbString& str)
{
    if (this != &str)
        Assign(str._data, str.Hdr()->length);
    return *this;
}

inline const CbString& CbString::operator =(const char* str)
{
    Assign(str ? str : "", str ? int(strlen(str)) : 0);
    return *this;
}

inline const CbString& CbString::operator =(char c)
{
    Reserve(1);
    _data[0] = c;
    SetLen(1);
    return *this;
}

inline const CbString& CbString::operator +=(const CbString& str)
{
    return operator +=(str._data);
}

inline const CbString& CbString::operator +=(const char* str)
{
    const int add = str ? int(strlen(str)) : 0;
    if (add)
    {
        const int n = Hdr()->length;
        Reserve(n + add);
        memcpy(_data + n, str, add);
        SetLen(n + add);
    }
    return *this;
}

inline const CbString& CbString::operator +=(char c)
{
    const int n = Hdr()->length;
    Reserve(n + 1);
    _data[n] = c;
    SetLen(n + 1);
    return *this;
}

inline void CbString::Empty()
{
    if (!IsShared())
        SetLen(0);
}

inline char CbString::GetAt(int index) const
{
    return _data[index];
}

inline void CbString::SetAt(int index, char c)
{
    if (0 <= index && index < Hdr()->length)
        _data[index] = c;
}

inline int CbString::Compare(const char* str) const
{
    return strcmp(_data, str ? str : "");
}

inline int CbString::CompareNoCase(const char* str) const
{
    return _stricmp(_data, str ? str : "");
}

inline int CbString::Find(char c, int start) const
{
    if (start < 0 || start > Hdr()->length)
        return -1;
    const char* p = strchr(_data + start, c);
    return p ? int(p - _data) : -1;
}

inline int CbString::Find(const char* sub, int start) const
{
    if (!sub || start < 0 || start > Hdr()->length)
        return -1;
    const char* p = strstr(_data + start, sub);
    return p ? int(p - _data) : -1;
}

inline int CbString::ReverseFind(char c) const
{
    const char* p = strrchr(_data, c);
    return p ? int(p - _data) : -1;
}

inline int CbString::FindOneOf(const char* set) const
{
    if (!set)
        return -1;
    const char* p = strpbrk(_data, set);
    return p ? int(p - _data) : -1;
}

inline CbString CbString::Left(int count) const
{
    const int len = Hdr()->length;
    if (count < 0)     count = 0;
    if (count > len)   count = len;
    return CbString(_data, count);
}

inline CbString CbString::Right(int count) const
{
    const int len = Hdr()->length;
    if (count < 0)     count = 0;
    if (count > len)   count = len;
    return CbString(_data + len - count, count);
}

inline CbString CbString::Mid(int first) const
{
    const int len = Hdr()->length;
    if (first < 0)     first = 0;
    if (first > len)   first = len;
    return CbString(_data + first, len - first);
}

inline CbString CbString::Mid(int first, int count) const
{
    const int len = Hdr()->length;
    if (first < 0)     first = 0;
    if (first > len)   first = len;
    if (count < 0)     count = 0;
    if (count > len - first)
        count = len - first;
    return CbString(_data + first, count);
}

inline void CbString::MakeUpper()
{
    const int len = Hdr()->length;
    for (int i = 0; i < len; i++)
        _data[i] = char(toupper((unsigned char)_data[i]));
}

inline void CbString::MakeLower()
{
    const int len = Hdr()->length;
    for (int i = 0; i < len; i++)
        _data[i] = char(tolower((unsigned char)_data[i]));
}

inline void CbString::MakeReverse()
{
    const int len = Hdr()->length;
    for (int i = 0; i < len / 2; i++)
    {
        char t = _data[i];
        _data[i] = _data[len - 1 - i];
        _data[len - 1 - i] = t;
    }
}

inline CbString& CbString::TrimLeft()
{
    int i = 0;
    const int len = Hdr()->length;
    while (i < len && isspace((unsigned char)_data[i]))
        i++;
    if (i)
        Delete(0, i);
    return *this;
}

inline CbString& CbString::TrimRight()
{
    if (IsShared())
        return *this;
    int n = Hdr()->length;
    while (n && isspace((unsigned char)_data[n - 1]))
        n--;
    SetLen(n);
    return *this;
}

inline CbString& CbString::Trim()
{
    TrimRight();
    TrimLeft();
    return *this;
}

inline int CbString::Delete(int index, int count)
{
    const int len = Hdr()->length;
    if (index < 0 || index >= len || count <= 0)
        return len;
    if (count > len - index)
        count = len - index;
    memmove(_data + index, _data + index + count, len - index - count);
    SetLen(len - count);
    return len - count;
}

inline int CbString::Remove(char c)
{
    if (IsShared())
        return 0;
    const int len = Hdr()->length;
    int w = 0;
    for (int r = 0; r < len; r++)
        if (_data[r] != c)
            _data[w++] = _data[r];
    SetLen(w);
    return len - w;
}

inline int CbString::Replace(char chOld, char chNew)
{
    int n = 0;
    const int len = Hdr()->length;
    for (int i = 0; i < len; i++)
        if (_data[i] == chOld)
        {
            _data[i] = chNew;
            n++;
        }
    return n;
}

inline int CbString::Replace(const char* strOld, const char* strNew)
{
    if (!strOld || !*strOld)
        return 0;
    const int oldLen = int(strlen(strOld));

    int n = 0;
    CbString result;
    int pos = 0;
    for (;;)
    {
        const int hit = Find(strOld, pos);
        if (hit < 0)
            break;
        result += CbString(_data + pos, hit - pos);
        if (strNew && *strNew)
            result += strNew;
        pos = hit + oldLen;
        n++;
    }
    if (n)
    {
        result += (_data + pos);
        *this = result;
    }
    return n;
}

inline void CbString::FormatV(const char* format, va_list args)
{
    va_list copy;
    va_copy(copy, args);
    const int n = vsnprintf(0, 0, format, copy);
    va_end(copy);
    if (n < 0)
    {
        Empty();
        return;
    }
    Reserve(n);
    vsnprintf(_data, n + 1, format, args);
    SetLen(n);
}

inline void CbString::Format(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    FormatV(format, args);
    va_end(args);
}

inline char* CbString::GetBuffer(int minLength)
{
    Reserve(minLength < 0 ? 0 : minLength);
    return _data;
}

inline char* CbString::GetBufferSetLength(int newLength)
{
    if (newLength < 0)
        newLength = 0;
    Reserve(newLength);
    SetLen(newLength);
    return _data;
}

inline void CbString::ReleaseBuffer(int newLength)
{
    if (newLength < 0)
        newLength = int(strlen(_data));
    if (!IsShared())
        SetLen(newLength);
}

inline void CbString::Preallocate(int capacityChars)
{
    if (capacityChars > 0)
        Reserve(capacityChars);
}

// CbString must stay exactly one pointer wide -- see the header comment.
// (Can't use static_assert at namespace scope pre-C++11-uniformly here in a
// header included everywhere, but the typedef trick traps a regression.)
typedef char CbString_must_be_pointer_sized
    [sizeof(CbString) == sizeof(char*) ? 1 : -1];

// ---------------------------------------------------------------------------
// CbString -- free operators
// ---------------------------------------------------------------------------

inline CbString operator +(const CbString& a, const CbString& b) { return CbString(a, b); }
inline CbString operator +(const CbString& a, const char* b)     { return CbString(a, CbString(b)); }
inline CbString operator +(const char* a, const CbString& b)     { return CbString(CbString(a), b); }
inline CbString operator +(const CbString& a, char b)            { return CbString(a, CbString(b, 1)); }
inline CbString operator +(char a, const CbString& b)            { return CbString(CbString(a, 1), b); }

inline bool operator ==(const CbString& a, const CbString& b) { return a.Compare(b) == 0; }
inline bool operator ==(const CbString& a, const char* b)     { return a.Compare(b) == 0; }
inline bool operator ==(const char* a, const CbString& b)     { return b.Compare(a) == 0; }
inline bool operator !=(const CbString& a, const CbString& b) { return a.Compare(b) != 0; }
inline bool operator !=(const CbString& a, const char* b)     { return a.Compare(b) != 0; }
inline bool operator !=(const char* a, const CbString& b)     { return b.Compare(a) != 0; }
inline bool operator < (const CbString& a, const CbString& b) { return a.Compare(b) <  0; }
inline bool operator < (const CbString& a, const char* b)     { return a.Compare(b) <  0; }
inline bool operator > (const CbString& a, const CbString& b) { return a.Compare(b) >  0; }
inline bool operator > (const CbString& a, const char* b)     { return a.Compare(b) >  0; }
inline bool operator <=(const CbString& a, const CbString& b) { return a.Compare(b) <= 0; }
inline bool operator >=(const CbString& a, const CbString& b) { return a.Compare(b) >= 0; }

#ifdef __AFX_H__
// MFC bridge (see the CbString(const CString&) note above): exact-match
// CbString<->CString comparison operators. Without these, `aCString == aCbString`
// is ambiguous -- CbString converts to const char* AND to CString -- so the
// compiler can't pick CString's operator. An exact overload removes the choice.
inline bool operator ==(const CbString& a, const CString& b) { return a.Compare((const char*)b) == 0; }
inline bool operator ==(const CString& a, const CbString& b) { return b.Compare((const char*)a) == 0; }
inline bool operator !=(const CbString& a, const CString& b) { return a.Compare((const char*)b) != 0; }
inline bool operator !=(const CString& a, const CbString& b) { return b.Compare((const char*)a) != 0; }
// Exact-match operator+ for the mixed combos -- otherwise `aCString + aCbString`
// is ambiguous (CbString converts to both const char* and CString).
inline CbString operator +(const CbString& a, const CString& b) { return a + (const char*)b; }
inline CbString operator +(const CString& a, const CbString& b) { return CbString(a) + b; }
#endif

#endif /* _CBSTRING_H */
