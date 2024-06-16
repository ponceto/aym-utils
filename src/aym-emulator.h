/*
 * aym-emulator.h - Copyright (c) 2023-2024 - Olivier Poncet
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
#ifndef __AYM_Emulator_h__
#define __AYM_Emulator_h__

// ---------------------------------------------------------------------------
// aym::ChipType
// ---------------------------------------------------------------------------

namespace aym {

enum ChipType
{
    CHIP_DEFAULT = 0,
    CHIP_AY8910  = 1,
    CHIP_AY8912  = 2,
    CHIP_AY8913  = 3,
    CHIP_YM2149  = 4,
};

}

// ---------------------------------------------------------------------------
// aym::State
// ---------------------------------------------------------------------------

namespace aym {

struct State
{
    uint8_t  type;
    uint32_t ticks;
    uint8_t  index;
    uint8_t  array[16];
    uint8_t  has_sound[3];
    uint8_t  has_noise[3];
    uint8_t  dir_port[2];
    float    dac[32];
};

}

// ---------------------------------------------------------------------------
// aym::Sound
// ---------------------------------------------------------------------------

namespace aym {

struct Sound
{
    uint16_t counter;
    uint16_t period;
    uint8_t  phase;
    uint8_t  amplitude;
};

}

// ---------------------------------------------------------------------------
// aym::Noise
// ---------------------------------------------------------------------------

namespace aym {

struct Noise
{
    uint16_t counter;
    uint16_t period;
    uint32_t shift;
    uint8_t  phase;
};

}

// ---------------------------------------------------------------------------
// aym::Envelope
// ---------------------------------------------------------------------------

namespace aym {

struct Envelope
{
    uint16_t counter;
    uint16_t period;
    uint8_t  shape;
    uint8_t  phase;
    uint8_t  amplitude;
};

}

// ---------------------------------------------------------------------------
// aym::Output
// ---------------------------------------------------------------------------

namespace aym {

struct Output
{
    float channel0;
    float channel1;
    float channel2;
};

}

// ---------------------------------------------------------------------------
// aym::Emulator
// ---------------------------------------------------------------------------

namespace aym {

class Emulator
{
public: // public interface
    Emulator(const ChipType type);

    Emulator(const Emulator&) = delete;

    Emulator& operator=(const Emulator&) = delete;

    virtual ~Emulator() = default;

    void reset();

    void clock();

    uint8_t set_index(uint8_t index);

    uint8_t get_value(uint8_t value);

    uint8_t set_value(uint8_t value);

    const Output& get_output() const
    {
        return _output;
    }

protected: // protected data
    State    _state;
    Sound    _sound[3];
    Noise    _noise[1];
    Envelope _envelope;
    Output   _output;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __AYM_Emulator_h__ */
