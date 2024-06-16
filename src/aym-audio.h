/*
 * aym-audio.h - Copyright (c) 2023-2024 - Olivier Poncet
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
#ifndef __AYM_Audio_h__
#define __AYM_Audio_h__

#include "miniaudio.h"
#include "aym-emulator.h"

// ---------------------------------------------------------------------------
// forward declarations
// ---------------------------------------------------------------------------

namespace aym {

class AudioConfig;
class AudioDevice;
class AudioProcessor;

}

// ---------------------------------------------------------------------------
// aliases declarations
// ---------------------------------------------------------------------------

namespace aym {

using AudioDeviceType = ma_device_type;
using MiniAudioConfig = ma_device_config;
using MiniAudioDevice = ma_device;
using Mutex           = std::mutex;
using MutexLock       = std::unique_lock<std::mutex>;

}

// ---------------------------------------------------------------------------
// aym::MonoFrame<T>
// ---------------------------------------------------------------------------

namespace aym {

template <typename T>
struct MonoFrame
{
    T mono;
};

using MonoFrameInt16 = MonoFrame<int16_t>;
using MonoFrameInt32 = MonoFrame<int32_t>;
using MonoFrameFlt32 = MonoFrame<float>;

static_assert(sizeof(MonoFrameInt16) == 2);
static_assert(sizeof(MonoFrameInt32) == 4);
static_assert(sizeof(MonoFrameFlt32) == 4);

}

// ---------------------------------------------------------------------------
// aym::StereoFrame<T>
// ---------------------------------------------------------------------------

namespace aym {

template <typename T>
struct StereoFrame
{
    T left;
    T right;
};

using StereoFrameInt16 = StereoFrame<int16_t>;
using StereoFrameInt32 = StereoFrame<int32_t>;
using StereoFrameFlt32 = StereoFrame<float>;

static_assert(sizeof(StereoFrameInt16) == 4);
static_assert(sizeof(StereoFrameInt32) == 8);
static_assert(sizeof(StereoFrameFlt32) == 8);

}

// ---------------------------------------------------------------------------
// aym::AudioConfig
// ---------------------------------------------------------------------------

namespace aym {

class AudioConfig
{
public: // public interface
    AudioConfig(const AudioDeviceType type);

   ~AudioConfig() = default;

    auto get() -> MiniAudioConfig*
    {
        return &_impl;
    }

    auto operator->() -> MiniAudioConfig*
    {
        return get();
    }

private: // private data
    MiniAudioConfig _impl;
};

}

// ---------------------------------------------------------------------------
// aym::AudioDevice
// ---------------------------------------------------------------------------

namespace aym {

class AudioDevice
{
public: // public interface
    AudioDevice(const AudioConfig& config);

    AudioDevice(const AudioDevice&) = delete;

    AudioDevice& operator=(const AudioDevice&) = delete;

    virtual ~AudioDevice();

    void start();

    void stop();

    void attach(AudioProcessor& processor);

    void detach(AudioProcessor& processor);

    auto get() const -> const MiniAudioDevice*
    {
        return &_impl;
    }

    auto operator->() const -> const MiniAudioDevice*
    {
        return get();
    }

private: // private data
    MiniAudioDevice _impl;
    AudioProcessor* _processor;
};

}

// ---------------------------------------------------------------------------
// aym::AudioProcessor
// ---------------------------------------------------------------------------

namespace aym {

class AudioProcessor
{
public: // public interface
    AudioProcessor(AudioDevice& device);

    AudioProcessor(const AudioProcessor&) = delete;

    AudioProcessor& operator=(const AudioProcessor&) = delete;

    virtual ~AudioProcessor();

    virtual void process(const void* input, void* output, const uint32_t count) = 0;

protected: // protected data
    AudioDevice& _device;
    Mutex        _mutex;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __AYM_Audio_h__ */
