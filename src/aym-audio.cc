/*
 * aym-audio.cc - Copyright (c) 2023-2024 - Olivier Poncet
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
#include "aym-audio.h"

// ---------------------------------------------------------------------------
// <anonymous>::BasicTraits
// ---------------------------------------------------------------------------

namespace {

struct BasicTraits
{
    using AudioDeviceType = aym::AudioDeviceType;
    using MiniAudioConfig = aym::MiniAudioConfig;
    using MiniAudioDevice = aym::MiniAudioDevice;
};

}

// ---------------------------------------------------------------------------
// <anonymous>::MiniAudioConfigTraits
// ---------------------------------------------------------------------------

namespace {

struct MiniAudioConfigTraits final
    : public BasicTraits
{
    static void init(MiniAudioConfig& config, const AudioDeviceType type)
    {
        config = ::ma_device_config_init(type);
    }
};

}

// ---------------------------------------------------------------------------
// <anonymous>::MiniAudioDeviceTraits
// ---------------------------------------------------------------------------

namespace {

struct MiniAudioDeviceTraits final
    : public BasicTraits
{
    static void init(MiniAudioDevice& device, MiniAudioConfig* config)
    {
        if(::ma_device_init(nullptr, config, &device) != MA_SUCCESS) {
            throw std::runtime_error("ma_device_init() has failed");
        }
    }

    static void uninit(MiniAudioDevice& device)
    {
        ::ma_device_uninit(&device);
    }

    static void start(MiniAudioDevice& device)
    {
        if(::ma_device_start(&device) != MA_SUCCESS) {
            throw std::runtime_error("ma_device_start() has failed");
        }
    }

    static void stop(MiniAudioDevice& device)
    {
        if(::ma_device_stop(&device) != MA_SUCCESS) {
            throw std::runtime_error("ma_device_stop() has failed");
        }
    }
};

}

// ---------------------------------------------------------------------------
// aym::AudioConfig
// ---------------------------------------------------------------------------

namespace aym {

AudioConfig::AudioConfig(const AudioDeviceType type)
    : _impl()
{
    MiniAudioConfigTraits::init(_impl, type);
}

}

// ---------------------------------------------------------------------------
// aym::AudioDevice
// ---------------------------------------------------------------------------

namespace aym {

AudioDevice::AudioDevice(const AudioConfig& config)
    : _impl()
    , _processor(nullptr)
{
    AudioConfig settings(config);

    auto callback = [](MiniAudioDevice* device_impl, void* output, const void* input, ma_uint32 count) -> void
    {
        AudioDevice* device = reinterpret_cast<AudioDevice*>(device_impl->pUserData);

        if(device != nullptr) {
            AudioProcessor* processor = device->_processor;
            if(processor != nullptr) {
                processor->process(input, output, count);
            }
        }
    };

    auto get_config = [&]() -> MiniAudioConfig*
    {
        settings->pUserData       = this;
        settings->dataCallback    = callback;
        settings->playback.format = ma_format_f32;
        return settings.get();
    };

    MiniAudioDeviceTraits::init(_impl, get_config());
}

AudioDevice::~AudioDevice()
{
    MiniAudioDeviceTraits::uninit(_impl);
}

void AudioDevice::start()
{
    MiniAudioDeviceTraits::start(_impl);
}

void AudioDevice::stop()
{
    MiniAudioDeviceTraits::stop(_impl);
}

void AudioDevice::attach(AudioProcessor& processor)
{
    if(_processor == nullptr) {
        _processor = &processor;
    }
}

void AudioDevice::detach(AudioProcessor& processor)
{
    if(_processor == &processor) {
        _processor = nullptr;
    }
}

}

// ---------------------------------------------------------------------------
// aym::AudioProcessor
// ---------------------------------------------------------------------------

namespace aym {

AudioProcessor::AudioProcessor(AudioDevice& device)
    : _device(device)
    , _mutex()
{
    _device.attach(*this);
}

AudioProcessor::~AudioProcessor()
{
    _device.detach(*this);
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
