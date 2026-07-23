#ifndef _CBGEOMETRY_H
#define _CBGEOMETRY_H
//
// CbGeometry.h -- MFC-free, windows.h-free CbPoint / CbSize / CbRect.
//
// Replacements for MFC's CPoint / CSize / CRect in the model / serialization.
// These are now STANDALONE plain-int structs (they used to derive from the
// Win32 tagPOINT/tagSIZE/tagRECT, which dragged <windows.h> into every header
// that includes this one). The whole Win32 interop (SIZE/POINT/RECT operators,
// operator LPRECT/LPCRECT) was vestigial -- nothing in the codebase passed a
// Win32 geometry struct to these or used the RECT* conversion -- so it's gone.
//
// Standard-layout structs of ints: the on-disk serialize (CbSerialize.h writes
// the fields individually) is unchanged, and CbRect::TopLeft/BottomRight can
// still reinterpret the (left,top)/(right,bottom) int pairs as CbPoints.
//
// The API mirrors the subset of CPoint/CSize/CRect the codebase uses.
//

// ---------------------------------------------------------------------------
// CbSize
// ---------------------------------------------------------------------------
struct CbSize
{
    int cx;
    int cy;

    CbSize()                       { cx = 0;  cy = 0;  }
    CbSize(int initCX, int initCY) { cx = initCX; cy = initCY; }

    bool operator ==(const CbSize& s) const { return cx == s.cx && cy == s.cy; }
    bool operator !=(const CbSize& s) const { return cx != s.cx || cy != s.cy; }

    CbSize operator +(const CbSize& s) const { return CbSize(cx + s.cx, cy + s.cy); }
    CbSize operator -(const CbSize& s) const { return CbSize(cx - s.cx, cy - s.cy); }
    CbSize operator -()              const { return CbSize(-cx, -cy); }

    void operator +=(const CbSize& s) { cx += s.cx; cy += s.cy; }
    void operator -=(const CbSize& s) { cx -= s.cx; cy -= s.cy; }
};

// ---------------------------------------------------------------------------
// CbPoint
// ---------------------------------------------------------------------------
struct CbPoint
{
    int x;
    int y;

    CbPoint()                     { x = 0;  y = 0;  }
    CbPoint(int initX, int initY) { x = initX; y = initY; }

    void Offset(int dx, int dy)     { x += dx;   y += dy;   }
    void Offset(const CbPoint& p)   { x += p.x;  y += p.y;  }
    void Offset(const CbSize& s)    { x += s.cx; y += s.cy; }

    bool operator ==(const CbPoint& p) const { return x == p.x && y == p.y; }
    bool operator !=(const CbPoint& p) const { return x != p.x || y != p.y; }

    void operator +=(const CbSize& s)  { x += s.cx; y += s.cy; }
    void operator -=(const CbSize& s)  { x -= s.cx; y -= s.cy; }
    void operator +=(const CbPoint& p) { x += p.x;  y += p.y;  }
    void operator -=(const CbPoint& p) { x -= p.x;  y -= p.y;  }

    CbPoint operator +(const CbSize& s)  const { return CbPoint(x + s.cx, y + s.cy); }
    CbPoint operator +(const CbPoint& p) const { return CbPoint(x + p.x,  y + p.y);  }
    CbPoint operator -(const CbSize& s)  const { return CbPoint(x - s.cx, y - s.cy); }
    CbSize  operator -(const CbPoint& p) const { return CbSize (x - p.x,  y - p.y);  }
    CbPoint operator -()                 const { return CbPoint(-x, -y); }
};

// ---------------------------------------------------------------------------
// CbRect
// ---------------------------------------------------------------------------
struct CbRect
{
    int left;
    int top;
    int right;
    int bottom;

    CbRect()                                  { left = top = right = bottom = 0; }
    CbRect(int l, int t, int r, int b)        { left = l; top = t; right = r; bottom = b; }
    CbRect(const CbPoint& topLeft, const CbPoint& botRight)
        { left = topLeft.x; top = topLeft.y; right = botRight.x; bottom = botRight.y; }
    CbRect(const CbPoint& p, const CbSize& s)
        { left = p.x; top = p.y; right = p.x + s.cx; bottom = p.y + s.cy; }

    int    Width()  const { return right - left; }
    int    Height() const { return bottom - top; }
    CbSize Size()   const { return CbSize(right - left, bottom - top); }

    // TopLeft()/BottomRight() reinterpret the (left,top) and (right,bottom)
    // int pairs as points -- same trick MFC's CRect used; the standard-layout
    // int field order guarantees it.
    CbPoint&       TopLeft()           { return *reinterpret_cast<CbPoint*>(&left);  }
    CbPoint&       BottomRight()       { return *reinterpret_cast<CbPoint*>(&right); }
    const CbPoint& TopLeft()     const { return *reinterpret_cast<const CbPoint*>(&left);  }
    const CbPoint& BottomRight() const { return *reinterpret_cast<const CbPoint*>(&right); }
    CbPoint        CenterPoint() const { return CbPoint((left + right) / 2, (top + bottom) / 2); }

    bool IsRectEmpty() const { return left >= right || top >= bottom; }
    void SetRect(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    void SetRectEmpty()      { left = top = right = bottom = 0; }
    bool PtInRect(const CbPoint& p) const
        { return p.x >= left && p.x < right && p.y >= top && p.y < bottom; }

    void NormalizeRect()
    {
        if (left > right)  { int t = left; left = right;  right  = t; }
        if (top  > bottom) { int t = top;  top  = bottom; bottom = t; }
    }

    void OffsetRect(int dx, int dy)   { left += dx; right += dx; top += dy; bottom += dy; }
    void OffsetRect(const CbPoint& p) { OffsetRect(p.x, p.y); }
    void OffsetRect(const CbSize& s)  { OffsetRect(s.cx, s.cy); }

    void InflateRect(int x, int y)  { left -= x; right += x; top -= y; bottom += y; }
    void InflateRect(int l, int t, int r, int b) { left -= l; top -= t; right += r; bottom += b; }
    void InflateRect(const CbSize& s)  { InflateRect(s.cx, s.cy); }
    void InflateRect(const CbRect& r)  { InflateRect(r.left, r.top, r.right, r.bottom); }
    void DeflateRect(const CbRect& r)  { DeflateRect(r.left, r.top, r.right, r.bottom); }
    void DeflateRect(int x, int y)  { InflateRect(-x, -y); }
    void DeflateRect(int l, int t, int r, int b) { left += l; top += t; right -= r; bottom -= b; }

    bool IntersectRect(const CbRect& r1, const CbRect& r2)
    {
        left   = r1.left   > r2.left   ? r1.left   : r2.left;
        top    = r1.top    > r2.top    ? r1.top    : r2.top;
        right  = r1.right  < r2.right  ? r1.right  : r2.right;
        bottom = r1.bottom < r2.bottom ? r1.bottom : r2.bottom;
        if (IsRectEmpty()) { SetRectEmpty(); return false; }
        return true;
    }

    // Offset operators -- mirror MFC's CRect: +/- a CbPoint or CbSize shifts the
    // whole rect (OffsetRect); the in-place forms do the same.
    CbRect operator +(const CbPoint& p) const { CbRect r(*this); r.OffsetRect(p); return r; }
    CbRect operator +(const CbSize&  s) const { CbRect r(*this); r.OffsetRect(s); return r; }
    CbRect operator -(const CbPoint& p) const { CbRect r(*this); r.OffsetRect(-p.x, -p.y); return r; }
    CbRect operator -(const CbSize&  s) const { CbRect r(*this); r.OffsetRect(-s.cx, -s.cy); return r; }

    void operator +=(const CbPoint& p) { OffsetRect(p); }
    void operator +=(const CbSize&  s) { OffsetRect(s); }
    void operator -=(const CbPoint& p) { OffsetRect(-p.x, -p.y); }
    void operator -=(const CbSize&  s) { OffsetRect(-s.cx, -s.cy); }

    // +/- a CbRect inflates/deflates by that rect's edges (MFC CRect semantics).
    CbRect operator +(const CbRect& r) const { CbRect x(*this); x.InflateRect(r); return x; }
    CbRect operator -(const CbRect& r) const { CbRect x(*this); x.DeflateRect(r); return x; }
    void operator +=(const CbRect& r) { InflateRect(r); }
    void operator -=(const CbRect& r) { DeflateRect(r); }

    // *= grows this rect to the bounding UNION with r (accumulating an extent,
    // e.g. a diagram's). An initial point-rect (BottomRight == TopLeft) is
    // REPLACED by r rather than united with it, so a fresh accumulator starts
    // at the first rect. Moved here from the model's master-include user section
    // to sit with the other CbRect operators (JV 2026-07-23).
    void operator *=(const CbRect& r)
    {
        if (BottomRight() == TopLeft())      // a point rect -> just take r
        {
            *this = r;
            return;
        }
        if (top    > r.top)    top    = r.top;
        if (bottom < r.bottom) bottom = r.bottom;
        if (left   > r.left)   left   = r.left;
        if (right  < r.right)  right  = r.right;
    }

    bool operator ==(const CbRect& r) const
        { return left == r.left && top == r.top && right == r.right && bottom == r.bottom; }
    bool operator !=(const CbRect& r) const
        { return !(*this == r); }
};

#endif /* _CBGEOMETRY_H */
