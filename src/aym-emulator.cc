/*
 * aym-emulator.cc - Copyright (c) 2023-2024 - Olivier Poncet
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
#include "aym-emulator.h"

// ---------------------------------------------------------------------------
// <anonymous>::BasicTraits
// ---------------------------------------------------------------------------

namespace {

struct BasicTraits
{
    using ChipType = aym::ChipType;
    using State    = aym::State;
    using Sound    = aym::Sound;
    using Noise    = aym::Noise;
    using Envelope = aym::Envelope;
    using Output   = aym::Output;
};

}

// ---------------------------------------------------------------------------
// <anonymous>::Traits
// ---------------------------------------------------------------------------

namespace {

struct Traits final
    : public BasicTraits
{
    static const float   ay_dac[32];
    static const float   ym_dac[32];
    static const uint8_t cycles[16][2];

    static constexpr uint8_t CHANNEL_A_FINE_TUNE   = 0x00;
    static constexpr uint8_t CHANNEL_A_COARSE_TUNE = 0x01;
    static constexpr uint8_t CHANNEL_B_FINE_TUNE   = 0x02;
    static constexpr uint8_t CHANNEL_B_COARSE_TUNE = 0x03;
    static constexpr uint8_t CHANNEL_C_FINE_TUNE   = 0x04;
    static constexpr uint8_t CHANNEL_C_COARSE_TUNE = 0x05;
    static constexpr uint8_t NOISE_PERIOD          = 0x06;
    static constexpr uint8_t MIXER_AND_IO_CONTROL  = 0x07;
    static constexpr uint8_t CHANNEL_A_AMPLITUDE   = 0x08;
    static constexpr uint8_t CHANNEL_B_AMPLITUDE   = 0x09;
    static constexpr uint8_t CHANNEL_C_AMPLITUDE   = 0x0a;
    static constexpr uint8_t ENVELOPE_FINE_TUNE    = 0x0b;
    static constexpr uint8_t ENVELOPE_COARSE_TUNE  = 0x0c;
    static constexpr uint8_t ENVELOPE_SHAPE        = 0x0d;
    static constexpr uint8_t IO_PORT_A             = 0x0e;
    static constexpr uint8_t IO_PORT_B             = 0x0f;
    static constexpr uint8_t RAMP_UP               = 0x00;
    static constexpr uint8_t RAMP_DOWN             = 0x01;
    static constexpr uint8_t HOLD_UP               = 0x02;
    static constexpr uint8_t HOLD_DOWN             = 0x03;
    static constexpr uint8_t SOUND0                = 0;
    static constexpr uint8_t SOUND1                = 1;
    static constexpr uint8_t SOUND2                = 2;
    static constexpr uint8_t NOISE0                = 0;
    static constexpr uint8_t PORT0                 = 0;
    static constexpr uint8_t PORT1                 = 1;
};

const float Traits::ay_dac[32] = {
    0.0000000, 0.0000000, 0.0099947, 0.0099947,
    0.0144503, 0.0144503, 0.0210575, 0.0210575,
    0.0307012, 0.0307012, 0.0455482, 0.0455482,
    0.0644999, 0.0644999, 0.1073625, 0.1073625,
    0.1265888, 0.1265888, 0.2049897, 0.2049897,
    0.2922103, 0.2922103, 0.3728389, 0.3728389,
    0.4925307, 0.4925307, 0.6353246, 0.6353246,
    0.8055848, 0.8055848, 1.0000000, 1.0000000
};

const float Traits::ym_dac[32] = {
    0.0000000, 0.0000000, 0.0046540, 0.0077211,
    0.0109560, 0.0139620, 0.0169986, 0.0200198,
    0.0243687, 0.0296941, 0.0350652, 0.0403906,
    0.0485389, 0.0583352, 0.0680552, 0.0777752,
    0.0925154, 0.1110857, 0.1297475, 0.1484855,
    0.1766690, 0.2115511, 0.2463874, 0.2811017,
    0.3337301, 0.4004273, 0.4673838, 0.5344320,
    0.6351720, 0.7580072, 0.8799268, 1.0000000
};

const uint8_t Traits::cycles[16][2] = {
    { RAMP_DOWN, HOLD_DOWN },
    { RAMP_DOWN, HOLD_DOWN },
    { RAMP_DOWN, HOLD_DOWN },
    { RAMP_DOWN, HOLD_DOWN },
    { RAMP_UP  , HOLD_DOWN },
    { RAMP_UP  , HOLD_DOWN },
    { RAMP_UP  , HOLD_DOWN },
    { RAMP_UP  , HOLD_DOWN },
    { RAMP_DOWN, RAMP_DOWN },
    { RAMP_DOWN, HOLD_DOWN },
    { RAMP_DOWN, RAMP_UP   },
    { RAMP_DOWN, HOLD_UP   },
    { RAMP_UP  , RAMP_UP   },
    { RAMP_UP  , HOLD_UP   },
    { RAMP_UP  , RAMP_DOWN },
    { RAMP_UP  , HOLD_DOWN },
};

}

// ---------------------------------------------------------------------------
// <anonymous>::StateTraits
// ---------------------------------------------------------------------------

namespace {

struct StateTraits final
    : public BasicTraits
{
    static inline auto reset(State& state) -> void
    {
        state.ticks &= 0;
        state.index &= 0;
        for(auto& value : state.array) {
            value &= 0;
        }
        for(auto& value : state.has_sound) {
            value &= 0;
        }
        for(auto& value : state.has_noise) {
            value &= 0;
        }
        for(auto& value : state.dir_port) {
            value &= 0;
        }
    }

    static inline auto set_type(State& state, const ChipType type) -> void
    {
        switch(state.type = type) {
            case ChipType::CHIP_AY8910:
            case ChipType::CHIP_AY8912:
            case ChipType::CHIP_AY8913:
                static_cast<void>(::memcpy(state.dac, Traits::ay_dac, sizeof(state.dac)));
                break;
            case ChipType::CHIP_YM2149:
                static_cast<void>(::memcpy(state.dac, Traits::ym_dac, sizeof(state.dac)));
                break;
            default:
                static_cast<void>(::memcpy(state.dac, Traits::ay_dac, sizeof(state.dac)));
                break;
        }
    }

    static inline auto get_mixer_and_io_control(State& state, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_mixer_and_io_control(State& state, const uint8_t value) -> uint8_t
    {
        state.has_sound[Traits::SOUND0] = ((value & 0x01) == 0);
        state.has_sound[Traits::SOUND1] = ((value & 0x02) == 0);
        state.has_sound[Traits::SOUND2] = ((value & 0x04) == 0);
        state.has_noise[Traits::SOUND0] = ((value & 0x08) == 0);
        state.has_noise[Traits::SOUND1] = ((value & 0x10) == 0);
        state.has_noise[Traits::SOUND2] = ((value & 0x20) == 0);
        state.dir_port[Traits::PORT0]   = ((value & 0x40) != 0);
        state.dir_port[Traits::PORT1]   = ((value & 0x80) != 0);

        return value;
    }

    static inline auto get_port0(State& state, const uint8_t value) -> uint8_t
    {
        if(state.dir_port[Traits::PORT0] == 0) {
            /* call port0 input */
        }
        return value;
    }

    static inline auto set_port0(State& state, const uint8_t value) -> uint8_t
    {
        if(state.dir_port[Traits::PORT0] != 0) {
            /* call port0 output */
        }
        return value;
    }

    static inline auto get_port1(State& state, const uint8_t value) -> uint8_t
    {
        if(state.dir_port[Traits::PORT1] == 0) {
            /* call port1 input */
        }
        return value;
    }

    static inline auto set_port1(State& state, const uint8_t value) -> uint8_t
    {
        if(state.dir_port[Traits::PORT1] != 0) {
            /* call port1 output */
        }
        return value;
    }
};

}

// ---------------------------------------------------------------------------
// <anonymous>::SoundTraits
// ---------------------------------------------------------------------------

namespace {

struct SoundTraits final
    : public BasicTraits
{
    static inline auto reset(Sound& sound) -> void
    {
        sound.counter   &= 0;
        sound.period    &= 0;
        sound.phase     &= 0;
        sound.amplitude &= 0;
    }

    static inline auto clock(Sound& sound) -> void
    {
        if(++sound.counter >= sound.period) {
            sound.counter &= 0;
            sound.phase   ^= 1;
        }
    }

    static inline auto get_fine_tune(Sound& sound, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_fine_tune(Sound& sound, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0xff00;
        const     uint16_t data = static_cast<uint16_t>(value) << 0;

        sound.period = ((sound.period & mask) | data);

        return value;
    }

    static inline auto get_coarse_tune(Sound& sound, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_coarse_tune(Sound& sound, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0x00ff;
        const     uint16_t data = static_cast<uint16_t>(value) << 8;

        sound.period = ((sound.period & mask) | data);

        return value;
    }

    static inline auto get_amplitude(Sound& sound, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_amplitude(Sound& sound, const uint8_t value) -> uint8_t
    {
        const uint8_t msb = ((value << 1) & 0b00111110);
        const uint8_t lsb = ((value >> 3) & 0b00000001);

        sound.amplitude = (msb | lsb);

        return value;
    }
};

}

// ---------------------------------------------------------------------------
// <anonymous>::NoiseTraits
// ---------------------------------------------------------------------------

namespace {

struct NoiseTraits final
    : public BasicTraits
{
    static inline auto reset(Noise& noise) -> void
    {
        noise.counter &= 0;
        noise.period  &= 0;
        noise.shift   &= 0;
        noise.phase   &= 0;
    }

    static inline auto clock(Noise& noise) -> void
    {
        if(++noise.counter >= noise.period) {
            noise.counter &= 0;
            const uint32_t lfsr = noise.shift;
            const uint32_t bit0 = (lfsr << 16);
            const uint32_t bit3 = (lfsr << 13);
            const uint32_t msw  = (~(bit0 ^ bit3) & 0x10000);
            const uint32_t lsw  = ((lfsr >> 1) & 0x0ffff);
            noise.shift = (msw | lsw);
            noise.phase = (lfsr & 1);
        }
    }

    static inline auto get_fine_tune(Noise& sound, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_fine_tune(Noise& sound, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0xff00;
        const     uint16_t data = static_cast<uint16_t>(value) << 0;

        sound.period = ((sound.period & mask) | data);

        return value;
    }

    static inline auto get_coarse_tune(Noise& sound, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_coarse_tune(Noise& sound, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0x00ff;
        const     uint16_t data = static_cast<uint16_t>(value) << 8;

        sound.period = ((sound.period & mask) | data);

        return value;
    }
};

}

// ---------------------------------------------------------------------------
// <anonymous>::EnvelopeTraits
// ---------------------------------------------------------------------------

namespace {

struct EnvelopeTraits final
    : public BasicTraits
{
    static inline auto reset(Envelope& envelope) -> void
    {
        envelope.counter   &= 0;
        envelope.period    &= 0;
        envelope.shape     &= 0;
        envelope.phase     &= 0;
        envelope.amplitude &= 0;
    }

    static inline auto clock(Envelope& envelope) -> void
    {
        if(++envelope.counter >= envelope.period) {
            envelope.counter &= 0;
            switch(Traits::cycles[envelope.shape][envelope.phase]) {
                case Traits::RAMP_UP:
                    envelope.amplitude = ((envelope.amplitude + 1) & 0x1f);
                    if(envelope.amplitude == 0x1f) {
                        envelope.phase ^= 1;
                    }
                    break;
                case Traits::RAMP_DOWN:
                    envelope.amplitude = ((envelope.amplitude - 1) & 0x1f);
                    if(envelope.amplitude == 0x00) {
                        envelope.phase ^= 1;
                    }
                    break;
                case Traits::HOLD_UP:
                    envelope.amplitude = 0x1f;
                    break;
                case Traits::HOLD_DOWN:
                    envelope.amplitude = 0x00;
                    break;
                default:
                    break;
            }
        }
    }

    static inline auto get_fine_tune(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_fine_tune(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0xff00;
        const     uint16_t data = static_cast<uint16_t>(value) << 0;

        envelope.period = ((envelope.period & mask) | data);

        return value;
    }

    static inline auto get_coarse_tune(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_coarse_tune(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        constexpr uint16_t mask = 0x00ff;
        const     uint16_t data = static_cast<uint16_t>(value) << 8;

        envelope.period = ((envelope.period & mask) | data);

        return value;
    }

    static inline auto get_shape(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        return value;
    }

    static inline auto set_shape(Envelope& envelope, const uint8_t value) -> uint8_t
    {
        envelope.shape     = value;
        envelope.phase     = 0;
        envelope.amplitude = ((envelope.shape & 0x04) == 0 ? 0x1f : 0x00);

        return value;
    }
};

}

// ---------------------------------------------------------------------------
// <anonymous>::OutputTraits
// ---------------------------------------------------------------------------

namespace {

struct OutputTraits final
    : public BasicTraits
{
    static inline auto reset(Output& output) -> void
    {
        output.channel0 = 0.0;
        output.channel1 = 0.0;
        output.channel2 = 0.0;
    }
};

}

// ---------------------------------------------------------------------------
// aym::Emulator
// ---------------------------------------------------------------------------

namespace aym {

Emulator::Emulator(const ChipType type)
    : _state()
    , _sound()
    , _noise()
    , _envelope()
    , _output()
{
    StateTraits::set_type(_state, type);

    reset();
}

void Emulator::reset()
{
    StateTraits::reset(_state);
    SoundTraits::reset(_sound[Traits::SOUND0]);
    SoundTraits::reset(_sound[Traits::SOUND1]);
    SoundTraits::reset(_sound[Traits::SOUND2]);
    NoiseTraits::reset(_noise[Traits::NOISE0]);
    EnvelopeTraits::reset(_envelope);
    OutputTraits::reset(_output);
}

void Emulator::clock()
{
    auto fixup = [&](Sound& lhs, Sound& rhs) -> void
    {
        if((lhs.period == rhs.period) && (lhs.counter != rhs.counter)) {
            rhs.counter = lhs.counter;
            rhs.phase   = lhs.phase;
        }
    };

    auto prepare = [&]() -> void
    {
        fixup(_sound[0], _sound[1]);
        fixup(_sound[0], _sound[2]);
        fixup(_sound[1], _sound[2]);
    };

    auto get_output = [&](const int sound_index, const int noise_index) -> float
    {
        const auto sound     = (_sound[sound_index].phase & _state.has_sound[sound_index]);
        const auto noise     = (_noise[noise_index].phase & _state.has_noise[sound_index]);
        const auto amplitude = (_sound[sound_index].amplitude & 0x20 ? (_envelope.amplitude & 0x1f) : (_sound[sound_index].amplitude & 0x1f));

        return _state.dac[(sound | noise) * amplitude];
    };

    auto output = [&]() -> void
    {
        _output.channel0 = get_output(Traits::SOUND0, Traits::NOISE0);
        _output.channel1 = get_output(Traits::SOUND1, Traits::NOISE0);
        _output.channel2 = get_output(Traits::SOUND2, Traits::NOISE0);
    };

    auto update = [&]() -> void
    {
        const auto clk_div = ((++_state.ticks) & 0x07);

        if(clk_div == 0) {
            prepare();
            SoundTraits::clock(_sound[Traits::SOUND0]);
            SoundTraits::clock(_sound[Traits::SOUND1]);
            SoundTraits::clock(_sound[Traits::SOUND2]);
            NoiseTraits::clock(_noise[Traits::NOISE0]);
            EnvelopeTraits::clock(_envelope);
            output();
        }
    };

    return update();
}

uint8_t Emulator::set_index(uint8_t index)
{
    return (_state.index = index);
}

uint8_t Emulator::get_value(uint8_t value)
{
    const auto index = _state.index;
    auto&      array = _state.array[index & 0x0f];

    switch(index) {
        case Traits::CHANNEL_A_FINE_TUNE:
            value = SoundTraits::get_fine_tune(_sound[Traits::SOUND0], (array &= 0xff));
            break;
        case Traits::CHANNEL_A_COARSE_TUNE:
            value = SoundTraits::get_coarse_tune(_sound[Traits::SOUND0], (array &= 0x0f));
            break;
        case Traits::CHANNEL_B_FINE_TUNE:
            value = SoundTraits::get_fine_tune(_sound[Traits::SOUND1], (array &= 0xff));
            break;
        case Traits::CHANNEL_B_COARSE_TUNE:
            value = SoundTraits::get_coarse_tune(_sound[Traits::SOUND1], (array &= 0x0f));
            break;
        case Traits::CHANNEL_C_FINE_TUNE:
            value = SoundTraits::get_fine_tune(_sound[Traits::SOUND2], (array &= 0xff));
            break;
        case Traits::CHANNEL_C_COARSE_TUNE:
            value = SoundTraits::get_coarse_tune(_sound[Traits::SOUND2], (array &= 0x0f));
            break;
        case Traits::NOISE_PERIOD:
            value = NoiseTraits::get_fine_tune(_noise[Traits::NOISE0], (array &= 0x1f));
            break;
        case Traits::MIXER_AND_IO_CONTROL:
            value = StateTraits::get_mixer_and_io_control(_state, (array &= 0xff));
            break;
        case Traits::CHANNEL_A_AMPLITUDE:
            value = SoundTraits::get_amplitude(_sound[Traits::SOUND0], (array &= 0x1f));
            break;
        case Traits::CHANNEL_B_AMPLITUDE:
            value = SoundTraits::get_amplitude(_sound[Traits::SOUND1], (array &= 0x1f));
            break;
        case Traits::CHANNEL_C_AMPLITUDE:
            value = SoundTraits::get_amplitude(_sound[Traits::SOUND2], (array &= 0x1f));
            break;
        case Traits::ENVELOPE_FINE_TUNE:
            value = EnvelopeTraits::get_fine_tune(_envelope, (array &= 0xff));
            break;
        case Traits::ENVELOPE_COARSE_TUNE:
            value = EnvelopeTraits::get_coarse_tune(_envelope, (array &= 0xff));
            break;
        case Traits::ENVELOPE_SHAPE:
            value = EnvelopeTraits::get_shape(_envelope, (array &= 0x0f));
            break;
        case Traits::IO_PORT_A:
            value = StateTraits::get_port0(_state, (array &= 0xff));
            break;
        case Traits::IO_PORT_B:
            value = StateTraits::get_port1(_state, (array &= 0xff));
            break;
        default:
            break;
    }
    return value;
}

uint8_t Emulator::set_value(uint8_t value)
{
    const auto index = _state.index;
    auto&      array = _state.array[index & 0x0f];

    switch(index) {
        case Traits::CHANNEL_A_FINE_TUNE:
            array = SoundTraits::set_fine_tune(_sound[Traits::SOUND0], (value &= 0xff));
            break;
        case Traits::CHANNEL_A_COARSE_TUNE:
            array = SoundTraits::set_coarse_tune(_sound[Traits::SOUND0], (value &= 0x0f));
            break;
        case Traits::CHANNEL_B_FINE_TUNE:
            array = SoundTraits::set_fine_tune(_sound[Traits::SOUND1], (value &= 0xff));
            break;
        case Traits::CHANNEL_B_COARSE_TUNE:
            array = SoundTraits::set_coarse_tune(_sound[Traits::SOUND1], (value &= 0x0f));
            break;
        case Traits::CHANNEL_C_FINE_TUNE:
            array = SoundTraits::set_fine_tune(_sound[Traits::SOUND2], (value &= 0xff));
            break;
        case Traits::CHANNEL_C_COARSE_TUNE:
            array = SoundTraits::set_coarse_tune(_sound[Traits::SOUND2], (value &= 0x0f));
            break;
        case Traits::NOISE_PERIOD:
            array = NoiseTraits::set_fine_tune(_noise[Traits::NOISE0], (value &= 0x1f));
            break;
        case Traits::MIXER_AND_IO_CONTROL:
            array = StateTraits::set_mixer_and_io_control(_state, (value &= 0xff));
            break;
        case Traits::CHANNEL_A_AMPLITUDE:
            array = SoundTraits::set_amplitude(_sound[Traits::SOUND0], (value &= 0x1f));
            break;
        case Traits::CHANNEL_B_AMPLITUDE:
            array = SoundTraits::set_amplitude(_sound[Traits::SOUND1], (value &= 0x1f));
            break;
        case Traits::CHANNEL_C_AMPLITUDE:
            array = SoundTraits::set_amplitude(_sound[Traits::SOUND2], (value &= 0x1f));
            break;
        case Traits::ENVELOPE_FINE_TUNE:
            array = EnvelopeTraits::set_fine_tune(_envelope, (value &= 0xff));
            break;
        case Traits::ENVELOPE_COARSE_TUNE:
            array = EnvelopeTraits::set_coarse_tune(_envelope, (value &= 0xff));
            break;
        case Traits::ENVELOPE_SHAPE:
            array = EnvelopeTraits::set_shape(_envelope, (value &= 0x0f));
            break;
        case Traits::IO_PORT_A:
            array = StateTraits::set_port0(_state, (value &= 0xff));
            break;
        case Traits::IO_PORT_B:
            array = StateTraits::set_port1(_state, (value &= 0xff));
            break;
        default:
            break;
    }
    return value;
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
