#ifndef _CBSERIALIZE_H
#define _CBSERIALIZE_H

#include <assert.h>
#include <string.h>
#include <iostream>

#include "CbString.h"
#include "CbGeometry.h"
#include "CbTime.h"

// Only the stream types are needed here. A blanket `using namespace std;` in a
// widely-#included header leaks std::byte into the global namespace, which makes
// every `byte` in windows.h (rpcndr.h) ambiguous (C2872) for any TU that pulls
// windows.h after this header — keep the using-declarations narrow.
using std::istream;
using std::ostream;

// Compression is layered above CbArchive via CbZstdStream — wrap the
// raw ifstream/ofstream with CbZstdInBuf / CbZstdOutBuf and pass the
// resulting istream/ostream to CbArchive. The archive itself stays
// stream-agnostic.

// Thrown (as an int) by CbArchive::operator>>(CbObject*&) when deserialization
// hits an unknown class tag -- a misaligned or corrupt stream, e.g. a .cbz whose
// serialize layout no longer matches this build. Callers that open documents
// wrap the load in try/catch and surface it as a message box.
#define CB_ARCHIVE_BAD_STREAM 0x0BAD57EA   // "BAD STrEAm"

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
class CbArchive;
class CbClassRegistration;

// ---------------------------------------------------------------------------
// CbObject — polymorphic root for the CbArchive serialization runtime.
// Bare class (no MFC dependency). Subclasses declare their own polymorphic
// behavior via CB_DECLARE_SERIAL / CB_IMPLEMENT_SERIAL.
// ---------------------------------------------------------------------------
class CbObject
{
public:
    virtual void Serialize(CbArchive& archive) = 0;
    virtual CbClassRegistration& GetClassRegistration() const = 0;
};

typedef CbObject* (*CreateObjectFuncPtr)();

// ---------------------------------------------------------------------------
// CB_DECLARE_SERIAL / CB_IMPLEMENT_SERIAL — kept as macros for user subclasses
// ---------------------------------------------------------------------------
#define CB_DECLARE_SERIAL(className)                                    \
    private:                                                            \
        static CbClassRegistration _register##className;                \
        static CbObject* CreateCbObject();                              \
    public:                                                             \
        virtual CbClassRegistration& GetClassRegistration() const;

#define CB_IMPLEMENT_SERIAL(className)                                  \
CbClassRegistration className::_register##className =                   \
    CbClassRegistration(#className, className::CreateCbObject);         \
CbObject* className::CreateCbObject()                                   \
{                                                                       \
    return new className;                                               \
}                                                                       \
CbClassRegistration& className::GetClassRegistration() const            \
{                                                                       \
    return _register##className;                                        \
}

// ---------------------------------------------------------------------------
// CbClassRegistration
// ---------------------------------------------------------------------------
class CbClassRegistration
{
private:
    // Static list head/tail/count
    static CbClassRegistration* _firstCbClassRegistration;
    static CbClassRegistration* _lastCbClassRegistration;
    static int _countCbClassRegistration;

    // Doubly-linked list node pointers
    CbClassRegistration* _prevCbClassRegistration;
    CbClassRegistration* _nextCbClassRegistration;


    CreateObjectFuncPtr _createObjectFuncPtr;
    char*               _className;
    unsigned short      _index;

public:
    CbClassRegistration(const char* className, CreateObjectFuncPtr createObjectFuncPtr);
    ~CbClassRegistration();
    CbObject* CreateCbObject() const;
    static CbClassRegistration* FindCbClassRegistration(const char* className);
    static CbClassRegistration* GetFirstCbClassRegistration();
    static CbClassRegistration* GetNextCbClassRegistration(CbClassRegistration* pos);
    static int     GetCbClassRegistrationCount();
    const char*    GetRegisteredClassName() const;
    unsigned short GetIndex() const;
    void           SetIndex(unsigned short index);
};

// ---------------------------------------------------------------------------
// CbArchive
// ---------------------------------------------------------------------------
class CbArchive
{
private:
    istream*                _istream;
    ostream*                _ostream;
    bool                    _loading;
    unsigned short          _indexStore;
    unsigned short          _indexLoad;
    CbClassRegistration**   _registration;
    int                     _totalLength;

    CbClassRegistration* LoadObjectType();
    void StoreObjectType(const CbObject* pCbObject);

public:
    // Diagnostic: name of the most recently instantiated class via
    // operator>>(CbObject*&), and the count of objects loaded.
    // Use them to identify where a deserialize blew up.
    CbString _lastLoadedClass;
    int      _objectsLoaded;

    CbArchive(istream& is);
    CbArchive(ostream& os);
    ~CbArchive();
    bool IsLoading() const;
    bool IsStoring() const;
    CbArchive& operator <<(bool value);
    CbArchive& operator <<(char value);
    CbArchive& operator <<(double value);
    CbArchive& operator <<(float value);
    CbArchive& operator <<(int value);
    CbArchive& operator <<(long value);
    CbArchive& operator <<(short value);
    CbArchive& operator <<(unsigned char value);
    CbArchive& operator <<(unsigned int value);
    CbArchive& operator <<(unsigned long value);
    CbArchive& operator <<(unsigned short value);
    CbArchive& operator <<(const CbString& str);
    CbArchive& operator <<(const CbPoint& point);
    CbArchive& operator <<(const CbRect& rect);
    CbArchive& operator <<(const CbSize& size);
    CbArchive& operator <<(const CbTime& time);
    CbArchive& operator <<(const CbObject* pCbObject);
    CbArchive& operator >>(bool& value);
    CbArchive& operator >>(char& value);
    CbArchive& operator >>(double& value);
    CbArchive& operator >>(float& value);
    CbArchive& operator >>(int& value);
    CbArchive& operator >>(long& value);
    CbArchive& operator >>(short& value);
    CbArchive& operator >>(unsigned char& value);
    CbArchive& operator >>(unsigned int& value);
    CbArchive& operator >>(unsigned long& value);
    CbArchive& operator >>(unsigned short& value);
    CbArchive& operator >>(CbString& str);
    CbArchive& operator >>(CbPoint& point);
    CbArchive& operator >>(CbRect& rect);
    CbArchive& operator >>(CbSize& size);
    CbArchive& operator >>(CbTime& time);
    CbArchive& operator >>(CbObject*& pCbObject);
    int  Read(void* buf, int len);
    void Write(const void* buf, int len);
    int  GetTotalLength() const;
};

// ===========================================================================
// Inline implementations — CbClassRegistration
// ===========================================================================

inline CbObject* CbClassRegistration::CreateCbObject() const
{
    return _createObjectFuncPtr();
}

inline const char* CbClassRegistration::GetRegisteredClassName() const
{
    return _className;
}

inline unsigned short CbClassRegistration::GetIndex() const
{
    return _index;
}

inline void CbClassRegistration::SetIndex(unsigned short index)
{
    _index = index;
}

inline CbClassRegistration* CbClassRegistration::GetFirstCbClassRegistration()
{
    return _firstCbClassRegistration;
}

inline CbClassRegistration* CbClassRegistration::GetNextCbClassRegistration(CbClassRegistration* pos)
{
    return pos ? pos->_nextCbClassRegistration : _firstCbClassRegistration;
}

inline int CbClassRegistration::GetCbClassRegistrationCount()
{
    return _countCbClassRegistration;
}

// ===========================================================================
// Inline implementations — CbArchive
// ===========================================================================

inline int CbArchive::GetTotalLength() const
{
    return _totalLength;
}

inline bool CbArchive::IsLoading() const
{
    return _loading;
}

inline bool CbArchive::IsStoring() const
{
    return !_loading;
}

// bool is serialized as a 4-byte int -- WIRE-COMPATIBLE with BOOL (== int).
// This lets a serialized member migrate BOOL -> bool without changing a single
// byte of the .cbz stream, so old files keep loading. (Verified safe at the
// time of the change: no serialized member was a 1-byte bool, and the runtime
// format writes no internal bool flags -- object refs use unsigned short.) The
// trivial extra 3 bytes per boolean is the price of uniform, migration-proof
// booleans. See the BOOL->bool migration notes (auto-memory).
inline CbArchive& CbArchive::operator <<(bool value)
{
    int asInt = value ? 1 : 0;
    Write(&asInt, sizeof(asInt));
    return *this;
}

inline CbArchive& CbArchive::operator <<(char value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(double value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(float value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(int value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(long value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(short value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(unsigned char value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(unsigned int value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(unsigned long value)
{
    Write(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator <<(unsigned short value)
{
    Write(&value, sizeof(value));
    return *this;
}

// Wire format: int32 byte-length + raw bytes. Single-byte today (ANSI build),
// read as UTF-8 after the Qt port — ASCII content stays one byte per char,
// so existing files load unchanged and there's no UTF-16 doubling on disk.
inline CbArchive& CbArchive::operator <<(const CbString& str)
{
    assert(IsStoring());
    int length = str.GetLength();
    Write(&length, sizeof(length));
    if (length)
        Write(str.c_str(), length);
    return *this;
}

// Field-by-field rather than memcpy of POINT/RECT/SIZE: keeps the on-disk bytes
// identical (these structs are tightly packed LONGs) but doesn't bake in the
// MFC struct layout. After the Qt port, QRect's inclusive-vs-exclusive right/
// bottom convention also has to be honoured here, not silently inherited.
inline CbArchive& CbArchive::operator <<(const CbPoint& point)
{
    int x = point.x, y = point.y;
    Write(&x, sizeof(x));
    Write(&y, sizeof(y));
    return *this;
}

inline CbArchive& CbArchive::operator <<(const CbRect& rect)
{
    int left = rect.left, top = rect.top, right = rect.right, bottom = rect.bottom;
    Write(&left,   sizeof(left));
    Write(&top,    sizeof(top));
    Write(&right,  sizeof(right));
    Write(&bottom, sizeof(bottom));
    return *this;
}

inline CbArchive& CbArchive::operator <<(const CbSize& size)
{
    int cx = size.cx, cy = size.cy;
    Write(&cx, sizeof(cx));
    Write(&cy, sizeof(cy));
    return *this;
}

inline CbArchive& CbArchive::operator <<(const CbTime& time)
{
    __time64_t value = time.GetTime();
    Write(&value, sizeof(value));
    return *this;
}

// NOTE: defined out-of-line in CbSerialize.cpp because it dispatches through
// CbObject's vtable and therefore needs the full type definition.
// CbArchive& operator <<(const CbObject* pCbObject);

// Read the 4-byte int written by operator<<(bool) (wire-compatible with BOOL).
inline CbArchive& CbArchive::operator >>(bool& value)
{
    assert(IsLoading());
    int asInt = 0;
    Read(&asInt, sizeof(asInt));
    value = (asInt != 0);
    return *this;
}

inline CbArchive& CbArchive::operator >>(char& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(double& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(float& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(int& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(long& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(short& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(unsigned char& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(unsigned int& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(unsigned long& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(unsigned short& value)
{
    assert(IsLoading());
    Read(&value, sizeof(value));
    return *this;
}

inline CbArchive& CbArchive::operator >>(CbString& str)
{
    assert(IsLoading());
    int length;
    Read(&length, sizeof(length));
    if (length)
    {
        char* buf = str.GetBuffer(length);
        Read(buf, length);
        str.ReleaseBuffer(length);
    }
    else
    {
        str.Empty();
    }
    return *this;
}

inline CbArchive& CbArchive::operator >>(CbPoint& point)
{
    assert(IsLoading());
    int x, y;
    Read(&x, sizeof(x));
    Read(&y, sizeof(y));
    point.x = x; point.y = y;
    return *this;
}

inline CbArchive& CbArchive::operator >>(CbRect& rect)
{
    assert(IsLoading());
    int left, top, right, bottom;
    Read(&left,   sizeof(left));
    Read(&top,    sizeof(top));
    Read(&right,  sizeof(right));
    Read(&bottom, sizeof(bottom));
    rect.left = left; rect.top = top; rect.right = right; rect.bottom = bottom;
    return *this;
}

inline CbArchive& CbArchive::operator >>(CbSize& size)
{
    assert(IsLoading());
    int cx, cy;
    Read(&cx, sizeof(cx));
    Read(&cy, sizeof(cy));
    size.cx = cx; size.cy = cy;
    return *this;
}

inline CbArchive& CbArchive::operator >>(CbTime& time)
{
    assert(IsLoading());
    __time64_t value = 0;
    Read(&value, sizeof(value));
    time = CbTime(value);
    return *this;
}

// NOTE: defined out-of-line in CbSerialize.cpp because it dispatches through
// CbObject's vtable and therefore needs the full type definition.
// CbArchive& operator >>(CbObject*& pCbObject);

inline int CbArchive::Read(void* buf, int len)
{
    int actualLen = 0;
    if (IsLoading() && _istream)
    {
        _istream->read((char*)buf, len);
        actualLen = (int)_istream->gcount();
        if (_istream->fail() || actualLen != len)
        {
            throw 1;
        }
    }

    _totalLength += actualLen;
    return actualLen;
}

inline void CbArchive::Write(const void* buf, int len)
{
    if (IsStoring() && _ostream)
    {
        _ostream->write((const char*)buf, len);
        _totalLength += len;
    }
}


#endif
