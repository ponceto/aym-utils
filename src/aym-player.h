/*
 * aym-player.h - Copyright (c) 2023-2025 - Olivier Poncet
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
#ifndef __AYM_Player_h__
#define __AYM_Player_h__

#include "aym-audio.h"
#include "aym-emulator.h"
#include "aym-playlist.h"
#include "aym-settings.h"
#include "ym-archive.h"

// ---------------------------------------------------------------------------
// aym::PlayerProcessor
// ---------------------------------------------------------------------------

namespace aym {

class PlayerProcessor final
    : public AudioProcessor
    , public Interface
{
public: // public interface
    PlayerProcessor(AudioDevice& device, const Settings& settings);

    PlayerProcessor(const PlayerProcessor&) = delete;

    PlayerProcessor& operator=(const PlayerProcessor&) = delete;

    virtual ~PlayerProcessor() = default;

    virtual void process(const void* input, void* output, const uint32_t count) override final;

    bool playing();

    void load(const std::string& filename);

    virtual uint8_t aym_port_a_rd(Emulator& emulator, uint8_t data) override final;

    virtual uint8_t aym_port_a_wr(Emulator& emulator, uint8_t data) override final;

    virtual uint8_t aym_port_b_rd(Emulator& emulator, uint8_t data) override final;

    virtual uint8_t aym_port_b_wr(Emulator& emulator, uint8_t data) override final;

private: // private types
    struct Music
    {
        uint32_t ticks;
        uint32_t clock;
        uint32_t index;
        uint32_t count;
    };

    struct Sound
    {
        uint32_t ticks;
        uint32_t clock;
    };

private: // private data
    ym::Archive _archive;
    Emulator    _emulator;
    Music       _music;
    Sound       _sound;
};

}

// ---------------------------------------------------------------------------
// aym::Player
// ---------------------------------------------------------------------------

namespace aym {

class Player
{
public: // public interface
    Player(Settings& settings, Playlist& playlist);

    Player(const Player&) = delete;

    Player& operator=(const Player&) = delete;

    virtual ~Player() = default;

    void play();

    void dump();

private: // private data
    Settings&       _settings;
    Playlist&       _playlist;
    AudioDevice     _device;
    PlayerProcessor _processor;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __AYM_Player_h__ */
