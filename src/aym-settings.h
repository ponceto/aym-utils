/*
 * aym-settings.h - Copyright (c) 2023-2024 - Olivier Poncet
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
#ifndef __AYM_Settings_h__
#define __AYM_Settings_h__

#include "aym-audio.h"
#include "aym-emulator.h"

// ---------------------------------------------------------------------------
// aym::Settings
// ---------------------------------------------------------------------------

namespace aym {

class Settings
{
public: // public interface
    Settings();

    Settings(const Settings&) = default;

    Settings& operator=(const Settings&) = default;

   ~Settings() = default;

    auto get_config() -> AudioConfig;

    auto get_chip() const -> ChipType
    {
        return _chip;
    }

    auto get_channels() const -> uint32_t
    {
        return _channels;
    }

    auto get_samplerate() const -> uint32_t
    {
        return _samplerate;
    }

    auto set_chip(const ChipType chip) -> void
    {
        _chip = chip;
    }

    auto set_channels(const uint32_t channels) -> void
    {
        _channels = channels;
    }

    auto set_samplerate(const uint32_t samplerate) -> void
    {
        _samplerate = samplerate;
    }

private: // private data
    ChipType _chip;
    uint32_t _channels;
    uint32_t _samplerate;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __AYM_Settings_h__ */
