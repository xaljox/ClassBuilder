// qt/QtModelText.h -- CbString <-> QString conversion at the Qt/model boundary.
//
// The model stores multi-line text with CRLF line endings -- the CB
// convention (method bodies, notes, the list fields, and the .cbz strings).
// Qt's QString / QPlainTextEdit use bare LF. Convert at every boundary: strip
// CR coming in, restore CRLF going out. For single-line fields (no newlines)
// both are no-ops, so these are safe to use for every string field.
//
// CRLF is a Windows-ism in origin, but it is the model's *storage* convention,
// not a Qt thing: every other consumer (codegen, the method-name picker, the
// serialised .cbz) assumes CRLF, so toCb must emit it today regardless of
// platform -- emitting LF here would just move the bug. The .cbz can stay
// CRLF-internal cross-platform (identical format everywhere; this file fixes
// up the widget boundary). The genuinely platform-dependent case -- the line
// endings of *generated source files* -- belongs in the codegen / file-write
// layer, not here. If CB ever makes LF its canonical convention, THIS is the
// single file to flip.
#pragma once

#include <QString>

#include "CbString.h"

// Model -> Qt: decode the local-8-bit bytes, drop CR so the widget sees LF.
inline QString toQ(const CbString& s)
{
    QString q = QString::fromLocal8Bit((const char*)s);
    q.remove(QLatin1Char('\r'));
    return q;
}

// Qt -> model: normalise to LF, then emit CRLF as the model expects.
inline CbString toCb(const QString& q)
{
    QString text = q;
    text.remove(QLatin1Char('\r'));
    text.replace(QLatin1Char('\n'), QStringLiteral("\r\n"));
    return CbString(text.toLocal8Bit().constData());
}
