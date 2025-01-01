/*
 * aym-player.cc - Copyright (c) 2023-2025 - Olivier Poncet
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
#include <unistd.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include "lha-stream.h"
#include "aym-player.h"

// ---------------------------------------------------------------------------
// aym::PlayerProcessor
// ---------------------------------------------------------------------------

namespace aym {

PlayerProcessor::PlayerProcessor(AudioDevice& device, const Settings& settings)
    : AudioProcessor(device)
    , _archive()
    , _emulator(settings.get_chip(), *this)
    , _music()
    , _sound()
{
}

void PlayerProcessor::process(const void* input, void* output, const uint32_t count)
{
    const auto  channels   = _device->playback.channels;
    const auto  samplerate = _device->sampleRate;
    const auto& psg        = _emulator.get_output();

    auto set_register = [&](const uint8_t index, const uint8_t value) -> void
    {
        if((index == 13) && (value == 0xff)) {
            return;
        }
        else {
            _emulator.set_index(index);
            _emulator.set_value(value);
        }
    };

    auto reset = [&]() -> void
    {
        _emulator.reset();
    };

    auto clock_music = [&]() -> void
    {
        if(_music.index >= _music.count) {
            return;
        }
        if(++_music.index < _music.count) {
            const auto& frame = _archive.frames[_music.index];
            for(int index = 0; index < 14; ++index) {
                const auto value = frame.data[index];
                set_register(index, value);
            }
        }
        else {
            reset();
        }
    };

    auto process_music = [&]() -> void
    {
        if((_music.ticks += _music.clock) >= samplerate) {
            do {
                clock_music();
            } while((_music.ticks -= samplerate) >= samplerate);
        }
    };

    auto clock_sound = [&]() -> void
    {
        _emulator.clock();
    };

    auto process_sound = [&]() -> void
    {
        if((_sound.ticks += _sound.clock) >= samplerate) {
            do {
                clock_sound();
            } while((_sound.ticks -= samplerate) >= samplerate);
        }
    };

    auto mix_mono = [&](MonoFrameFlt32& audio_frame) -> void
    {
        const float mono = (psg.channel0 * 1.00f)
                         + (psg.channel1 * 1.00f)
                         + (psg.channel2 * 1.00f)
                         ;
        audio_frame.mono = (mono / 3.0f);
    };

    auto mix_stereo = [&](StereoFrameFlt32& audio_frame) -> void
    {
        const float left  = (psg.channel0 * 0.75f)
                          + (psg.channel1 * 0.50f)
                          + (psg.channel2 * 0.25f)
                          ;
        const float right = (psg.channel0 * 0.25f)
                          + (psg.channel1 * 0.50f)
                          + (psg.channel2 * 0.75f)
                          ;
        audio_frame.left  = (left  / 1.5f);
        audio_frame.right = (right / 1.5f);
    };

    auto mix = [&](const int index) -> void
    {
        switch(channels) {
            case 1:
                mix_mono(reinterpret_cast<MonoFrameFlt32*>(output)[index]);
                break;
            case 2:
                mix_stereo(reinterpret_cast<StereoFrameFlt32*>(output)[index]);
                break;
            default:
                break;
        }
    };

    auto render = [&]() -> void
    {
        const MutexLock lock(_mutex);

        for(uint32_t index = 0; index < count; ++index) {
            process_music();
            process_sound();
            mix(index);
        }
    };

    return render();
}

bool PlayerProcessor::playing()
{
    const MutexLock lock(_mutex);

    if(_music.index < _music.count) {
        return true;
    }
    return false;
}

void PlayerProcessor::load(const std::string& filename)
{
    const MutexLock lock(_mutex);

    auto ym_create = [](char* filename) -> void
    {
        const int rc = ::mkstemp(filename);

        if(rc >= 0) {
            static_cast<void>(::close(rc));
        }
        else {
            throw std::runtime_error("mkstemp() has failed");
        }
    };

    auto ym_extract = [](const std::string& archive, const std::string& output) -> void
    {
        lha::Stream stream(archive);
        lha::Reader reader(stream);

        if(reader.next() != false) {
            reader.extract(output);
        }
    };

    auto ym_import = [](const std::string& filename, ym::Archive& archive) -> void
    {
        ym::Reader reader(filename, archive);

        reader.read();
    };

    auto ym_remove = [](const std::string& filename) -> void
    {
        const int rc = ::unlink(filename.c_str());

        if((rc != 0) && (errno != ENOENT)) {
            throw std::runtime_error("unlink() has failed");
        }
    };

    auto ym_finalize = [&]() -> void
    {
        _music.ticks = 0;
        _music.clock = _archive.header.framerate;
        _music.index = 0;
        _music.count = _archive.header.frames;
        _sound.ticks = 0;
        _sound.clock = _archive.header.frequency;
    };

    auto ym_load_uncompressed = [&]() -> bool
    {
        ym::Reader reader(filename, _archive);

        if(reader.probe()) {
            reader.read();
            ym_finalize();
            return true;
        }
        return false;
    };

    auto ym_load_compressed = [&]() -> void
    {
        char extracted[] = "/tmp/aym-player-XXXXXX";
        try {
            ym_create(extracted);
            ym_extract(filename, extracted);
            ym_import(extracted, _archive);
            ym_remove(extracted);
            ym_finalize();
        }
        catch(...) {
            ym_remove(extracted);
            throw;
        }
    };

    auto try_load = [&]() -> void
    {
        if(ym_load_uncompressed() == false) {
            ym_load_compressed();
        }
    };

    return try_load();
}

uint8_t PlayerProcessor::aym_port_a_rd(Emulator& emulator, uint8_t data)
{
    return data;
}

uint8_t PlayerProcessor::aym_port_a_wr(Emulator& emulator, uint8_t data)
{
    return data;
}

uint8_t PlayerProcessor::aym_port_b_rd(Emulator& emulator, uint8_t data)
{
    return data;
}

uint8_t PlayerProcessor::aym_port_b_wr(Emulator& emulator, uint8_t data)
{
    return data;
}

}

// ---------------------------------------------------------------------------
// aym::Player
// ---------------------------------------------------------------------------

namespace aym {

Player::Player(Settings& settings, Playlist& playlist)
    : _settings(settings)
    , _playlist(playlist)
    , _device(_settings.get_config())
    , _processor(_device, _settings)
{
    _settings.set_channels(_device->playback.channels);
    _settings.set_samplerate(_device->sampleRate);
}

void Player::play()
{
    auto setup = [&]() -> void
    {
        std::string filename;

        if(_playlist.get(filename) != false) {
            _processor.load(filename);
        }
    };

    auto start = [&]() -> void
    {
        _device.start();
    };

    auto stop = [&]() -> void
    {
        _device.stop();
    };

    auto playing = [&]() -> bool
    {
        bool result = _processor.playing();

        if(result == false) {
            std::string filename;
            if((result = _playlist.next(filename)) != false) {
                _processor.load(filename);
            }
        }
        return result;
    };

    auto process = []() -> void
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    };

    auto mainloop = [&]() -> void
    {
        setup();
        start();
        while(playing()) {
            process();
        }
        stop();
    };

    return mainloop();
}

void Player::dump()
{
    constexpr uint32_t length = 16384;
    float              buffer[length * 2];

    auto write_mono = [&](MonoFrameFlt32& audio_frame) -> void
    {
        const int rc = ::write(STDOUT_FILENO, &audio_frame, sizeof(audio_frame));
        if(rc < 0) {
            throw std::runtime_error("write() has failed");
        }
    };

    auto write_stereo = [&](StereoFrameFlt32& audio_frame) -> void
    {
        const int rc = ::write(STDOUT_FILENO, &audio_frame, sizeof(audio_frame));
        if(rc < 0) {
            throw std::runtime_error("write() has failed");
        }
    };

    auto process = [&]() -> void
    {
        _processor.process(nullptr, buffer, length);
        const auto channels = _device->playback.channels;
        for(uint32_t index = 0; index < length; ++index) {
            switch(channels) {
                case 1:
                    write_mono(reinterpret_cast<MonoFrameFlt32*>(buffer)[index]);
                    break;
                case 2:
                    write_stereo(reinterpret_cast<StereoFrameFlt32*>(buffer)[index]);
                    break;
                default:
                    break;
            }
        }
    };

    auto setup = [&]() -> void
    {
        std::string filename;

        if(_playlist.get(filename) != false) {
            _processor.load(filename);
        }
    };

    auto start = [&]() -> void
    {
    };

    auto stop = [&]() -> void
    {
    };

    auto playing = [&]() -> bool
    {
        bool result = _processor.playing();

        if(result == false) {
            std::string filename;
            if((result = _playlist.next(filename)) != false) {
                _processor.load(filename);
            }
        }
        return result;
    };

    auto mainloop = [&]() -> void
    {
        setup();
        start();
        while(playing()) {
            process();
        }
        stop();
    };

    return mainloop();
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
