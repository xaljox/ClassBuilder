// CbZstdStream.h — std::streambuf wrappers that transparently Zstd-compress
// on write and decompress on read.
//
// Use them by attaching to an existing binary istream/ostream:
//
//     std::ofstream raw(path, std::ios::binary);
//     {
//         CbZstdOutBuf zbuf(raw);
//         std::ostream zos(&zbuf);
//         CbArchive ar(zos);
//         doc.Serialize(ar);
//     }   // zbuf destructor finalises the compressed stream
//
//     std::ifstream raw(path, std::ios::binary);
//     CbZstdInBuf  zbuf(raw);
//     std::istream zis(&zbuf);
//     CbArchive ar(zis);
//     doc.Serialize(ar);
//
// CbArchive itself stays oblivious — it just sees a std::istream/std::ostream.

#pragma once

#include <streambuf>
#include <iosfwd>
#include <vector>

class CbZstdOutBuf : public std::streambuf
{
public:
    explicit CbZstdOutBuf(std::ostream& sink, int compressionLevel = 0);
    ~CbZstdOutBuf() override;

    // Returns true if any error occurred during compression / sink write.
    bool HasError() const { return _error; }

protected:
    int_type overflow(int_type ch) override;
    int      sync() override;

private:
    bool flushBuffer(bool finish);

    std::ostream&     _sink;
    void*             _cctx;        // ZSTD_CCtx*
    std::vector<char> _inBuf;       // streambuf put-area
    std::vector<char> _outBuf;      // compressed output staging
    bool              _error;
};


class CbZstdInBuf : public std::streambuf
{
public:
    explicit CbZstdInBuf(std::istream& src);
    ~CbZstdInBuf() override;

    bool HasError() const { return _error; }

protected:
    int_type underflow() override;

private:
    std::istream&     _src;
    void*             _dctx;        // ZSTD_DCtx*
    std::vector<char> _inBuf;       // raw compressed bytes from src
    std::vector<char> _outBuf;      // decompressed bytes for streambuf get-area
    size_t            _inPos;
    size_t            _inSize;
    bool              _eof;
    bool              _error;
};
