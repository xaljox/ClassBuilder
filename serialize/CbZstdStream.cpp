#include "stdafx.h"
#include "CbZstdStream.h"

#include <zstd.h>
#include <ostream>
#include <istream>

#undef ZSTD_CLEVEL_DEFAULT
#define ZSTD_CLEVEL_DEFAULT 4

// ---------------------------------------------------------------------------
// CbZstdOutBuf — streaming compressor
// ---------------------------------------------------------------------------

CbZstdOutBuf::CbZstdOutBuf(std::ostream& sink, int compressionLevel)
    : _sink(sink),
      _cctx(ZSTD_createCCtx()),
      _inBuf(ZSTD_CStreamInSize()),
      _outBuf(ZSTD_CStreamOutSize()),
      _error(false)
{
    if (_cctx)
    {
        int level = (compressionLevel == 0) ? ZSTD_CLEVEL_DEFAULT
                                            : compressionLevel;
        ZSTD_CCtx_setParameter((ZSTD_CCtx*)_cctx,
                               ZSTD_c_compressionLevel, level);
    }
    setp(_inBuf.data(), _inBuf.data() + _inBuf.size());
}

CbZstdOutBuf::~CbZstdOutBuf()
{
    if (_cctx && !_error)
    {
        flushBuffer(true);
    }
    if (_cctx)
    {
        ZSTD_freeCCtx((ZSTD_CCtx*)_cctx);
    }
}

CbZstdOutBuf::int_type CbZstdOutBuf::overflow(int_type ch)
{
    if (sync() != 0) return traits_type::eof();
    if (!traits_type::eq_int_type(ch, traits_type::eof()))
    {
        *pptr() = traits_type::to_char_type(ch);
        pbump(1);
    }
    return traits_type::not_eof(ch);
}

int CbZstdOutBuf::sync()
{
    return flushBuffer(false) ? 0 : -1;
}

bool CbZstdOutBuf::flushBuffer(bool finish)
{
    if (_error || !_cctx) return false;

    const size_t pending = (size_t)(pptr() - pbase());

    ZSTD_inBuffer in = { pbase(), pending, 0 };
    ZSTD_EndDirective mode = finish ? ZSTD_e_end : ZSTD_e_continue;

    // Loop until both input is consumed AND (when finishing) the encoder
    // signals it has flushed everything.
    size_t remaining;
    do
    {
        ZSTD_outBuffer out = { _outBuf.data(), _outBuf.size(), 0 };
        remaining = ZSTD_compressStream2((ZSTD_CCtx*)_cctx, &out, &in, mode);
        if (ZSTD_isError(remaining)) { _error = true; return false; }

        if (out.pos > 0)
        {
            _sink.write(_outBuf.data(), (std::streamsize)out.pos);
            if (!_sink) { _error = true; return false; }
        }
    }
    while (in.pos < in.size || (finish && remaining != 0));

    setp(_inBuf.data(), _inBuf.data() + _inBuf.size());
    return true;
}


// ---------------------------------------------------------------------------
// CbZstdInBuf — streaming decompressor
// ---------------------------------------------------------------------------

CbZstdInBuf::CbZstdInBuf(std::istream& src)
    : _src(src),
      _dctx(ZSTD_createDCtx()),
      _inBuf(ZSTD_DStreamInSize()),
      _outBuf(ZSTD_DStreamOutSize()),
      _inPos(0),
      _inSize(0),
      _eof(false),
      _error(false)
{
    // Empty get-area triggers underflow on first read.
    setg(_outBuf.data(), _outBuf.data(), _outBuf.data());
}

CbZstdInBuf::~CbZstdInBuf()
{
    if (_dctx) ZSTD_freeDCtx((ZSTD_DCtx*)_dctx);
}

CbZstdInBuf::int_type CbZstdInBuf::underflow()
{
    if (_error || !_dctx) return traits_type::eof();

    while (true)
    {
        // Refill compressed input if exhausted.
        if (_inPos >= _inSize)
        {
            if (_eof) return traits_type::eof();
            _src.read(_inBuf.data(), (std::streamsize)_inBuf.size());
            _inSize = (size_t)_src.gcount();
            _inPos = 0;
            if (_inSize == 0) { _eof = true; return traits_type::eof(); }
        }

        ZSTD_inBuffer  in  = { _inBuf.data(),  _inSize,        _inPos };
        ZSTD_outBuffer out = { _outBuf.data(), _outBuf.size(), 0       };
        size_t r = ZSTD_decompressStream((ZSTD_DCtx*)_dctx, &out, &in);
        _inPos = in.pos;
        if (ZSTD_isError(r)) { _error = true; return traits_type::eof(); }

        if (out.pos > 0)
        {
            setg(_outBuf.data(),
                 _outBuf.data(),
                 _outBuf.data() + out.pos);
            return traits_type::to_int_type(*gptr());
        }

        // out.pos == 0 — decoder needs more input. Loop and refill.
    }
}
