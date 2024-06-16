/*
 * stream.cc - Copyright (c) 2023-2024 - Olivier Poncet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "lha-stream.h"

// ---------------------------------------------------------------------------
// <anonymous>::BasicTraits
// ---------------------------------------------------------------------------

namespace {

struct BasicTraits
{
    using StreamImpl = lha::StreamImpl;
    using ReaderImpl = lha::ReaderImpl;
    using HeaderImpl = lha::HeaderImpl;
};

}

// ---------------------------------------------------------------------------
// aym::StreamImplTraits
// ---------------------------------------------------------------------------

namespace {

struct StreamImplTraits final
    : public BasicTraits
{
    static auto create(StreamImpl* stream, const std::string& filename) -> StreamImpl*
    {
        if(stream == nullptr) {
            stream = ::lha_input_stream_from(const_cast<char*>(filename.c_str()));
        }
        if(stream == nullptr) {
            throw std::runtime_error("lha_input_stream_from() has failed");
        }
        return stream;
    }

    static auto destroy(StreamImpl* stream) -> StreamImpl*
    {
        if(stream != nullptr) {
            stream = (::lha_input_stream_free(stream), nullptr);
        }
        return stream;
    }
};

}

// ---------------------------------------------------------------------------
// aym::ReaderImplTraits
// ---------------------------------------------------------------------------

namespace {

struct ReaderImplTraits final
    : public BasicTraits
{
    static auto create(ReaderImpl* reader, StreamImpl* stream) -> ReaderImpl*
    {
        if(reader == nullptr) {
            reader = ::lha_reader_new(stream);
        }
        if(reader == nullptr) {
            throw std::runtime_error("lha_reader_new() has failed");
        }
        return reader;
    }

    static auto destroy(ReaderImpl* reader) -> ReaderImpl*
    {
        if(reader != nullptr) {
            reader = (::lha_reader_free(reader), nullptr);
        }
        return reader;
    }

    static auto next(ReaderImpl* reader) -> bool
    {
        if(::lha_reader_next_file(reader) != nullptr) {
            return true;
        }
        return false;
    }

    static auto extract(ReaderImpl* reader, const std::string& filename) -> void
    {
        const int rc = ::lha_reader_extract(reader, const_cast<char*>(filename.c_str()), nullptr, nullptr);

        if(rc == 0) {
            throw std::runtime_error("lha_reader_extract() has failed");
        }
    }
};

}

// ---------------------------------------------------------------------------
// lha::Stream
// ---------------------------------------------------------------------------

namespace lha {

Stream::Stream(const std::string& filename)
    : _lha_stream(nullptr)
{
    _lha_stream = StreamImplTraits::create(_lha_stream, filename);
}

Stream::~Stream()
{
    _lha_stream = StreamImplTraits::destroy(_lha_stream);
}

}

// ---------------------------------------------------------------------------
// lha::Reader
// ---------------------------------------------------------------------------

namespace lha {

Reader::Reader(Stream& stream)
    : _lha_reader(nullptr)
{
    _lha_reader = ReaderImplTraits::create(_lha_reader, stream.get());
}

Reader::~Reader()
{
    _lha_reader = ReaderImplTraits::destroy(_lha_reader);
}

bool Reader::next()
{
    return ReaderImplTraits::next(_lha_reader);
}

void Reader::extract(const std::string& filename)
{
    ReaderImplTraits::extract(_lha_reader, filename);
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
