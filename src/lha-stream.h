/*
 * stream.h - Copyright (c) 2023-2025 - Olivier Poncet
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
#ifndef __LHA_Stream_h__
#define __LHA_Stream_h__

#include <lhasa.h>

// ---------------------------------------------------------------------------
// aliases declarations
// ---------------------------------------------------------------------------

namespace lha {

using StreamImpl = LHAInputStream;
using ReaderImpl = LHAReader;
using HeaderImpl = LHAFileHeader;

}

// ---------------------------------------------------------------------------
// lha::Stream
// ---------------------------------------------------------------------------

namespace lha {

class Stream
{
public: // public interface
    Stream(const std::string& filename);

    Stream(const Stream&) = delete;

    Stream& operator=(const Stream&) = delete;

    virtual ~Stream();

    auto get() -> auto
    {
        return _lha_stream;
    }

private: // private data
    StreamImpl* _lha_stream;
};

}

// ---------------------------------------------------------------------------
// lha::Reader
// ---------------------------------------------------------------------------

namespace lha {

class Reader
{
public: // public interface
    Reader(Stream&);

    Reader(const Reader&) = delete;

    Reader& operator=(const Reader&) = delete;

    virtual ~Reader();

    bool next();

    void extract(const std::string& filename);

    auto get() -> auto
    {
        return _lha_reader;
    }

private: // private data
    ReaderImpl* _lha_reader;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __LHA_Stream_h__ */
