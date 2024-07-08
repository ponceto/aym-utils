/*
 * ym-archive.cc - Copyright (c) 2023-2024 - Olivier Poncet
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
#include "ym-archive.h"

// ---------------------------------------------------------------------------
// Some useful macros
// ---------------------------------------------------------------------------

#ifndef countof
#define countof(array) (sizeof(array) / sizeof(array[0]))
#endif

// ---------------------------------------------------------------------------
// ym::Archive
// ---------------------------------------------------------------------------

namespace ym {

}

// ---------------------------------------------------------------------------
// ym::Stream
// ---------------------------------------------------------------------------

namespace ym {

Stream::Stream(const std::string& filename, const std::string& filemode)
    : _filename(filename)
    , _filemode(filemode)
    , _stream(nullptr)
{
    if((_stream = ::fopen(_filename.c_str(), _filemode.c_str())) == nullptr) {
        const std::string reason(::strerror(errno));
        throw std::runtime_error(filename + ':' + ' ' + reason);
    }
}

Stream::~Stream()
{
    if(_stream != nullptr) {
        _stream = (::fclose(_stream), nullptr);
    }
}

void Stream::rewind()
{
    const int rc = ::fseek(_stream, 0, SEEK_SET);

    if(rc != 0) {
        throw std::runtime_error("fseek() has failed");
    }
}

auto Stream::read_byte() -> uint8_t
{
    uint8_t      byte = 0;
    const size_t size = ::fread(&byte, sizeof(byte), 1, _stream);
    if(size != sizeof(byte)) {
        throw std::runtime_error("read_byte() has failed");
    }
    return byte;
}

auto Stream::read_uint08be(uint8_t& value) -> void
{
    value = 0;
    try {
        value = ((value << 8) | read_byte());
    }
    catch(...) {
        throw std::runtime_error("read_uint08be() has failed");
    }
}

auto Stream::read_uint16be(uint16_t& value) -> void
{
    value = 0;
    try {
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
    }
    catch(...) {
        throw std::runtime_error("read_uint16be() has failed");
    }
}

auto Stream::read_uint32be(uint32_t& value) -> void
{
    value = 0;
    try {
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
    }
    catch(...) {
        throw std::runtime_error("read_uint32be() has failed");
    }
}

auto Stream::read_uint64be(uint64_t& value) -> void
{
    value = 0;
    try {
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
        value = ((value << 8) | read_byte());
    }
    catch(...) {
        throw std::runtime_error("read_uint64be() has failed");
    }
}

auto Stream::read_string(std::string& value) -> void
{
    value.clear();
    try {
        do {
            const char character = read_byte();
            if(character != '\0') {
                value += character;
            }
            else {
                break;
            }
        } while(true);
    }
    catch(...) {
        throw std::runtime_error("read_string() has failed");
    }
}

}

// ---------------------------------------------------------------------------
// ym::Reader
// ---------------------------------------------------------------------------

namespace ym {

Reader::Reader(const std::string& filename, Archive& archive)
    : Stream(filename, "rb")
    , _archive(archive)
{
}

void Reader::read()
{
    auto read_magic = [&]() -> uint32_t
    {
        uint32_t magic = 0;

        rewind();
        read_uint32be(magic);

        return magic;
    };

    return ym_read(read_magic());
}

bool Reader::probe()
{
    auto read_magic = [&]() -> uint32_t
    {
        uint32_t magic = 0;

        rewind();
        read_uint32be(magic);

        return magic;
    };

    auto is_ym = [&](const uint32_t magic) -> bool
    {
        if(magic == TAG_YM1) {
            return true;
        }
        if(magic == TAG_YM2) {
            return true;
        }
        if(magic == TAG_YM3) {
            return true;
        }
        if(magic == TAG_YM4) {
            return true;
        }
        if(magic == TAG_YM5) {
            return true;
        }
        if(magic == TAG_YM6) {
            return true;
        }
        return false;
    };

    return is_ym(read_magic());
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym_read(const uint32_t magic)
{
    if(magic == TAG_YM1) {
        return ym1_read();
    }
    if(magic == TAG_YM2) {
        return ym2_read();
    }
    if(magic == TAG_YM3) {
        return ym3_read();
    }
    if(magic == TAG_YM4) {
        return ym4_read();
    }
    if(magic == TAG_YM5) {
        return ym5_read();
    }
    if(magic == TAG_YM6) {
        return ym6_read();
    }
    throw std::runtime_error("unsupported file format");
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM1 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym1_read()
{
    rewind();
    ym1_read_begin();
    ym1_read_end();
}

void Reader::ym1_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM1) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym1_read_end()
{
    auto read_end = [&]() -> void
    {
        throw std::runtime_error("YM1! format is not supported");
    };

    return read_end();
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM2 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym2_read()
{
    rewind();
    ym2_read_begin();
    ym2_read_end();
}

void Reader::ym2_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM2) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym2_read_end()
{
    auto read_end = [&]() -> void
    {
        throw std::runtime_error("YM2! format is not supported");
    };

    return read_end();
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM3 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym3_read()
{
    rewind();
    ym3_read_begin();
    ym3_read_end();
}

void Reader::ym3_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM3) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym3_read_end()
{
    auto read_end = [&]() -> void
    {
        throw std::runtime_error("YM3! format is not supported");
    };

    return read_end();
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM4 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym4_read()
{
    rewind();
    ym4_read_begin();
    ym4_read_end();
}

void Reader::ym4_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM4) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym4_read_end()
{
    auto read_end = [&]() -> void
    {
        throw std::runtime_error("YM4! format is not supported");
    };

    return read_end();
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM5 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym5_read()
{
    rewind();
    ym5_read_begin();
    ym5_read_header();
    ym5_read_samples();
    ym5_read_metadata();
    ym5_read_frames();
    ym5_read_footer();
    ym5_read_end();
}

void Reader::ym5_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM5) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym5_read_header()
{
    auto read_signature = [&]() -> void
    {
        read_uint64be(_archive.header.signature);

        if(_archive.header.signature != TAG_LEONARD) {
            throw std::runtime_error("bad header signature");
        }
    };

    auto read_frames = [&]() -> void
    {
        read_uint32be(_archive.header.frames);

        if(_archive.header.frames > countof(_archive.frames)) {
            throw std::runtime_error("bad num frames");
        }
    };

    auto read_attributes = [&]() -> void
    {
        read_uint32be(_archive.header.attributes);
    };

    auto read_samples = [&]() -> void
    {
        read_uint16be(_archive.header.samples);
    };

    auto read_frequency = [&]() -> void
    {
        read_uint32be(_archive.header.frequency);
    };

    auto read_framerate = [&]() -> void
    {
        read_uint16be(_archive.header.framerate);
    };

    auto read_frameloop = [&]() -> void
    {
        read_uint32be(_archive.header.frameloop);
    };

    auto read_extrabytes = [&]() -> void
    {
        read_uint16be(_archive.header.extrabytes);

        if(_archive.header.extrabytes != 0) {
            throw std::runtime_error("bad extrabytes");
        }
    };

    auto read_header = [&]() -> void
    {
        read_signature();
        read_frames();
        read_attributes();
        read_samples();
        read_frequency();
        read_framerate();
        read_frameloop();
        read_extrabytes();
    };

    return read_header();
}

void Reader::ym5_read_samples()
{
    auto read_size = [&](Sample& sample) -> void
    {
        read_uint32be(sample.size);
    };

    auto read_data = [&](Sample& sample) -> void
    {
        const uint32_t count = sample.size;
        if(count <= countof(sample.data)) {
            for(uint32_t index = 0; index < count; ++index) {
                 read_uint08be(sample.data[index]);
            }
        }
        else {
            throw std::runtime_error("bad sample size");
        }
    };

    auto read_samples = [&]() -> void
    {
        const uint32_t count = _archive.header.samples;
        if(count <= countof(_archive.samples)) {
            for(uint32_t index = 0; index < count; ++index) {
                auto& sample(_archive.samples[index]);
                read_size(sample);
                read_data(sample);
            }
        }
        else {
            throw std::runtime_error("bad samples count");
        }
    };

    return read_samples();
}

void Reader::ym5_read_metadata()
{
    auto read_name = [&]() -> void
    {
        read_string(_archive.infos.title);
    };

    auto read_author = [&]() -> void
    {
        read_string(_archive.infos.author);
    };

    auto read_comments = [&]() -> void
    {
        read_string(_archive.infos.comments);
    };

    auto read_metadata = [&]() -> void
    {
        read_name();
        read_author();
        read_comments();
    };

    return read_metadata();
}

void Reader::ym5_read_frames()
{
    auto read_progressive = [&]() -> void
    {
        const uint32_t count = _archive.header.frames;
        for(uint32_t index = 0; index < count; ++index) {
            auto& frame(_archive.frames[index]);
            read_uint08be(frame.data[0x0]);
            read_uint08be(frame.data[0x1]);
            read_uint08be(frame.data[0x2]);
            read_uint08be(frame.data[0x3]);
            read_uint08be(frame.data[0x4]);
            read_uint08be(frame.data[0x5]);
            read_uint08be(frame.data[0x6]);
            read_uint08be(frame.data[0x7]);
            read_uint08be(frame.data[0x8]);
            read_uint08be(frame.data[0x9]);
            read_uint08be(frame.data[0xa]);
            read_uint08be(frame.data[0xb]);
            read_uint08be(frame.data[0xc]);
            read_uint08be(frame.data[0xd]);
            read_uint08be(frame.data[0xe]);
            read_uint08be(frame.data[0xf]);
        }
    };

    auto read_interleaved = [&]() -> void
    {
        const uint32_t count = _archive.header.frames;
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x0]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x1]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x2]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x3]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x4]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x5]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x6]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x7]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x8]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0x9]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xa]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xb]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xc]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xd]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xe]); }
        for(uint32_t index = 0; index < count; ++index) { read_uint08be(_archive.frames[index].data[0xf]); }
    };

    auto read_frames = [&]() -> void
    {
        if((_archive.header.attributes & 0x01) != 0) {
            read_interleaved();
        }
        else {
            read_progressive();
        }
    };

    return read_frames();
}

void Reader::ym5_read_footer()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.footer.magic);

        if(_archive.footer.magic != TAG_END) {
            throw std::runtime_error("bad footer magic");
        }
    };

    auto read_footer = [&]() -> void
    {
        read_magic();
    };

    return read_footer();
}

void Reader::ym5_read_end()
{
}

}

// ---------------------------------------------------------------------------
// ym::Reader - YM6 file format
// ---------------------------------------------------------------------------

namespace ym {

void Reader::ym6_read()
{
    rewind();
    ym6_read_begin();
    ym6_read_header();
    ym6_read_samples();
    ym6_read_metadata();
    ym6_read_frames();
    ym6_read_footer();
    ym6_read_end();
}

void Reader::ym6_read_begin()
{
    auto read_magic = [&]() -> void
    {
        read_uint32be(_archive.header.magic);

        if(_archive.header.magic != TAG_YM6) {
            throw std::runtime_error("bad header magic");
        }
    };

    auto read_begin = [&]() -> void
    {
        read_magic();
    };

    return read_begin();
}

void Reader::ym6_read_header()
{
    ym5_read_header();
}

void Reader::ym6_read_samples()
{
    ym5_read_samples();
}

void Reader::ym6_read_metadata()
{
    ym5_read_metadata();
}

void Reader::ym6_read_frames()
{
    ym5_read_frames();
}

void Reader::ym6_read_footer()
{
    ym5_read_footer();
}

void Reader::ym6_read_end()
{
    ym5_read_end();
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
