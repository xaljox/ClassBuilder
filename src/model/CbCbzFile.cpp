// model/CbCbzFile.cpp -- see CbCbzFile.h.

#include "StdAfx.h"
#include "CbCbzFile.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <iterator>
#include <sstream>

#include "ClassBuilderInclude.h"   // DataModelDoc, DataModelDocObject
#include "CbSerialize.h"
#include "CbZstdStream.h"

namespace {

// First int of a header file. Never a project version (those are >= 0), and
// an older ClassBuilder reading it as _version skips every gated field and
// then fails loudly on the misaligned stream ("file is corrupt").
const int kCbzHeaderMarker = (int)0xCB2F0000;

// The format every header-less file is read as at most: its gates were the
// project version, which at this point never covered more than format 3.
const int kLegacyFormat = 3;

// The read gates use the protected static DataModelDocObject::_objectVersion
// (NOT DataModelDoc::_objectVersion, a separate static no gate reads). A
// derived type -- never instantiated -- may set it.
struct GateVersion : DataModelDocObject
{
    static void Set(int version) { _objectVersion = version; }
};

int IntAt(const std::string& data, size_t offset)
{
    int value = 0;
    std::memcpy(&value, data.data() + offset, sizeof(value));
    return value;
}

} // namespace

long long Cb_WriteCbz(DataModelDoc& doc, std::ostream& os)
{
    CbZstdOutBuf zbuf(os);
    std::ostream zos(&zbuf);
    CbArchive ar(zos);

    const int projectVersion = doc.GetVersion();
    ar << kCbzHeaderMarker;
    ar << projectVersion;

    // The _version slot carries the format; put the project version back
    // whatever happens.
    doc.SetVersion(kCbzFormatVersion);
    try
    {
        doc.Serialize(ar);
    }
    catch (...)
    {
        doc.SetVersion(projectVersion);
        throw;
    }
    doc.SetVersion(projectVersion);
    return ar.GetTotalLength();
    // ar / zos / zbuf destruct in reverse order; zbuf's destructor ends the
    // zstd frame and writes the compressed tail.
}

void Cb_ReadCbz(DataModelDoc& doc, std::istream& is, CbCbzReadInfo& info)
{
    // Decompress the whole stream first: a legacy file's first int has to be
    // rewritten before CbArchive sees it, and the zstd stream cannot rewind.
    std::string data;
    {
        CbZstdInBuf zbuf(is);
        std::istream zis(&zbuf);
        data.assign(std::istreambuf_iterator<char>(zis),
                    std::istreambuf_iterator<char>());
        if (zbuf.HasError())
            throw (int)CB_ARCHIVE_BAD_STREAM;
    }
    if (data.size() < sizeof(int))
        throw (int)CB_ARCHIVE_BAD_STREAM;

    size_t offset = 0;
    const int first = IntAt(data, 0);
    if (first == kCbzHeaderMarker)
    {
        if (data.size() < 3 * sizeof(int))
            throw (int)CB_ARCHIVE_BAD_STREAM;
        info.projectVersion = IntAt(data, sizeof(int));
        offset = 2 * sizeof(int);
        info.format = IntAt(data, offset);   // the stream's _version slot
        if (info.format > kCbzFormatVersion)
            throw (int)CB_CBZ_NEWER_FORMAT;
    }
    else
    {
        info.projectVersion = first;
        info.format = std::min(first, kLegacyFormat);
        std::memcpy(&data[0], &info.format, sizeof(int));
    }

    std::istringstream ss(data.substr(offset),
                          std::ios::in | std::ios::binary);
    CbArchive ar(ss);
    try
    {
        doc.Serialize(ar);
    }
    catch (...)
    {
        info.bytes     = ar.GetTotalLength();
        info.objects   = ar._objectsLoaded;
        info.lastClass = ar._lastLoadedClass.c_str();
        // The read set the SHARED gate static to this file's format; every
        // other open document reads its undo snapshots with it.
        GateVersion::Set(INT_MAX);
        throw;
    }
    info.bytes     = ar.GetTotalLength();
    info.objects   = ar._objectsLoaded;
    info.lastClass = ar._lastLoadedClass.c_str();
    doc.SetVersion(info.projectVersion);
    // Undo / redo snapshots are always current: read every field.
    GateVersion::Set(INT_MAX);
}
