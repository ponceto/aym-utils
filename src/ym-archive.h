/*
 * ym-archive.h - Copyright (c) 2023-2025 - Olivier Poncet
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
#ifndef __YM_Archive_h__
#define __YM_Archive_h__

// ---------------------------------------------------------------------------
// ym::Header
// ---------------------------------------------------------------------------

namespace ym {

struct Header
{
    uint32_t magic      = 0u;
    uint64_t signature  = 0u;
    uint32_t frames     = 0u;
    uint32_t attributes = 0u;
    uint16_t samples    = 0u;
    uint32_t frequency  = 0u;
    uint16_t framerate  = 0u;
    uint32_t frameloop  = 0u;
    uint16_t extrabytes = 0u;
};

}

// ---------------------------------------------------------------------------
// ym::Sample
// ---------------------------------------------------------------------------

namespace ym {

struct Sample
{
    uint32_t size       = 0u;
    uint8_t  data[1024] = {};
};

}

// ---------------------------------------------------------------------------
// ym::Infos
// ---------------------------------------------------------------------------

namespace ym {

struct Infos
{
    std::string title    = "";
    std::string author   = "";
    std::string comments = "";
};

}

// ---------------------------------------------------------------------------
// ym::Frame
// ---------------------------------------------------------------------------

namespace ym {

struct Frame
{
    uint8_t data[16] = {};
};

}

// ---------------------------------------------------------------------------
// ym::Footer
// ---------------------------------------------------------------------------

namespace ym {

struct Footer
{
    uint32_t magic = 0u;
};

}

// ---------------------------------------------------------------------------
// ym::Archive
// ---------------------------------------------------------------------------

namespace ym {

struct Archive
{
    Header header;
    Sample samples[128];
    Infos  infos;
    Frame  frames[65536];
    Footer footer;
};

}

// ---------------------------------------------------------------------------
// ym::Stream
// ---------------------------------------------------------------------------

namespace ym {

class Stream
{
public: // public interface
    Stream(const std::string& filename, const std::string& filemode);

    Stream(const Stream&) = delete;

    Stream& operator=(const Stream&) = delete;

    virtual ~Stream();

    void rewind();

    auto read_byte() -> uint8_t;

    auto read_uint08be(uint8_t& value) -> void;

    auto read_uint16be(uint16_t& value) -> void;

    auto read_uint32be(uint32_t& value) -> void;

    auto read_uint64be(uint64_t& value) -> void;

    auto read_string(std::string& value) -> void;

private: // private data
    const std::string _filename;
    const std::string _filemode;
    FILE*             _stream;
};

}

// ---------------------------------------------------------------------------
// ym::Reader
// ---------------------------------------------------------------------------

namespace ym {

class Reader
    : public Stream
{
public: // public interface
    Reader(const std::string& filename, Archive& archive);

    Reader(const Reader&) = delete;

    Reader& operator=(const Reader&) = delete;

    virtual ~Reader() = default;

    void read();

    bool probe();

private: // YM private interface
    void ym_read(const uint32_t magic);

private: // YM1 private interface
    void ym1_read();
    void ym1_read_begin();
    void ym1_read_end();

private: // YM2 private interface
    void ym2_read();
    void ym2_read_begin();
    void ym2_read_end();

private: // YM3 private interface
    void ym3_read();
    void ym3_read_begin();
    void ym3_read_end();

private: // YM4 private interface
    void ym4_read();
    void ym4_read_begin();
    void ym4_read_end();

private: // YM5 private interface
    void ym5_read();
    void ym5_read_begin();
    void ym5_read_header();
    void ym5_read_samples();
    void ym5_read_metadata();
    void ym5_read_frames();
    void ym5_read_footer();
    void ym5_read_end();

private: // YM6 private interface
    void ym6_read();
    void ym6_read_begin();
    void ym6_read_header();
    void ym6_read_samples();
    void ym6_read_metadata();
    void ym6_read_frames();
    void ym6_read_footer();
    void ym6_read_end();

private: // private static data
    static constexpr uint32_t TAG_YM1     = 0x594d3121;
    static constexpr uint32_t TAG_YM2     = 0x594d3221;
    static constexpr uint32_t TAG_YM3     = 0x594d3321;
    static constexpr uint32_t TAG_YM4     = 0x594d3421;
    static constexpr uint32_t TAG_YM5     = 0x594d3521;
    static constexpr uint32_t TAG_YM6     = 0x594d3621;
    static constexpr uint64_t TAG_LEONARD = 0x4c654f6e41724421;
    static constexpr uint64_t TAG_END     = 0x456e6421;

private: // private data
    Archive& _archive;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __YM_Archive_h__ */
