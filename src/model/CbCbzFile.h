// model/CbCbzFile.h -- read / write a whole .cbz document: the zstd frame, the
// format header, and DataModelDoc::Serialize. Every .cbz reader and writer
// (open, save, crash recovery, the load_cb / save_cb commands) goes through
// these two, so they all agree on the layout.
//
// Why a header: the generated read gates (`if (N <= _objectVersion)`) compare
// against the version stored in DataModelDoc::_version -- but that is the
// PROJECT version, which grows with every source write (Dammen64 is at 138).
// A field added to ClassBuilder's own model at N would then be "expected" in
// every older file whose project already passed N. So the file now carries
// the project version in a header, and the _version slot of the stream holds
// the FORMAT version this build wrote; the gates see that, and the project
// version is put back after the read.
//
//   header file:   int kCbzHeaderMarker, int projectVersion, <stream with
//                  _version slot = format version>
//   legacy file:   <stream with _version slot = project version>  (no header;
//                  every file before format 4) -- read as format
//                  min(projectVersion, 3), exactly as before.
#pragma once

#include <iosfwd>
#include <string>

class DataModelDoc;

// The .cbz format level this build writes. A field of ClassBuilder's own model
// with InitialVersion N needs N <= this to be read. 4 = constexpr on
// Method / Member.
const int kCbzFormatVersion = 4;

// Thrown (as an int, like CB_ARCHIVE_BAD_STREAM) when a file carries a format
// newer than kCbzFormatVersion: written by a newer ClassBuilder.
#define CB_CBZ_NEWER_FORMAT 0x0CB2F0F4

// Diagnostics of a read, filled also when it throws.
struct CbCbzReadInfo
{
    int         format         = 0;    // format the stream was read as
    int         projectVersion = 0;
    long long   bytes          = 0;    // logical (decompressed) bytes consumed
    int         objects        = 0;
    std::string lastClass;
};

// Write `doc` to `os` (an open binary file stream) as a zstd-framed .cbz with
// the format header. Throws what CbArchive throws. Returns the logical byte
// count.
long long Cb_WriteCbz(DataModelDoc& doc, std::ostream& os);

// Read a .cbz from `is` into `doc` (the caller emptied it first). Restores the
// doc's project version, reports the format the file was read as in
// info.format (for format-dependent fix-ups), and -- success or not -- leaves
// the shared gate static DataModelDocObject::_objectVersion at INT_MAX, so
// undo / redo snapshots read every field. Throws (int) CB_ARCHIVE_BAD_STREAM /
// CB_CBZ_NEWER_FORMAT or what CbArchive throws.
void Cb_ReadCbz(DataModelDoc& doc, std::istream& is, CbCbzReadInfo& info);
