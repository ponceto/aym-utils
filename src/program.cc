/*
 * program.cc - Copyright (c) 2023-2024 - Olivier Poncet
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
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include "aym-player.h"
#include "console.h"
#include "program.h"

// ---------------------------------------------------------------------------
// some useful declarations
// ---------------------------------------------------------------------------

using ChipType = aym::ChipType;
using Settings = aym::Settings;
using Playlist = aym::Playlist;
using Player   = aym::Player;

enum Command
{
    COMMAND_HELP = 0,
    COMMAND_PLAY = 1,
    COMMAND_DUMP = 2,
};

// ---------------------------------------------------------------------------
// Program
// ---------------------------------------------------------------------------

void Program::main(const ArgList& args)
{
    Command  command = COMMAND_HELP;
    Settings settings;
    Playlist playlist;

    auto file_exists = [&](const std::string& filename) -> bool
    {
        const int rc = ::access(filename.c_str(), R_OK);

        if(rc == 0) {
            return true;
        }
        return false;
    };

    auto set_command = [&](const Command cmd) -> void
    {
        if(command == COMMAND_HELP) {
            command = cmd;
        }
        else {
            throw std::runtime_error("the command has already been given");
        }
    };

    auto set_chip = [&](const ChipType chip) -> void
    {
        if(settings.get_chip() == 0) {
            settings.set_chip(chip);
        }
        else {
            throw std::runtime_error("the chip type has already been given");
        }
    };

    auto set_channels = [&](const uint32_t channels) -> void
    {
        if(settings.get_channels() == 0) {
            settings.set_channels(channels);
        }
        else {
            throw std::runtime_error("the number of channels has already been given");
        }
    };

    auto set_samplerate = [&](const uint32_t samplerate) -> void
    {
        if(settings.get_samplerate() == 0) {
            settings.set_samplerate(samplerate);
        }
        else {
            throw std::runtime_error("the sample rate has already been given");
        }
    };

    auto add_to_playlist = [&](const std::string& filename) -> void
    {
        playlist.add(filename);
    };

    auto invalid_argument = [&](const std::string& argument) -> void
    {
        const std::string what("invalid argument");

        throw std::runtime_error(what + ' ' + '<' + argument + '>');
    };

    auto arg_command = [&](const int argi, const std::string& arg) -> bool
    {
        if(argi == 1) {
            if(arg == "help") {
                set_command(Command::COMMAND_HELP);
                return true;
            }
            if(arg == "play") {
                set_command(Command::COMMAND_PLAY);
                return true;
            }
            if(arg == "dump") {
                set_command(Command::COMMAND_DUMP);
                return true;
            }
        }
        return false;
    };

    auto arg_chip = [&](const int argi, const std::string& arg) -> bool
    {
        if(argi >= 2) {
            if(arg == "ay8910") {
                set_chip(ChipType::CHIP_AY8910);
                return true;
            }
            if(arg == "ay8912") {
                set_chip(ChipType::CHIP_AY8912);
                return true;
            }
            if(arg == "ay8913") {
                set_chip(ChipType::CHIP_AY8913);
                return true;
            }
            if(arg == "ym2149") {
                set_chip(ChipType::CHIP_YM2149);
                return true;
            }
        }
        return false;
    };

    auto arg_channels = [&](const int argi, const std::string& arg) -> bool
    {
        if(argi >= 2) {
            if(arg == "mono") {
                set_channels(1);
                return true;
            }
            if(arg == "stereo") {
                set_channels(2);
                return true;
            }
        }
        return false;
    };

    auto arg_samplerate = [&](const int argi, const std::string& arg) -> bool
    {
        if(argi >= 2) {
            if(arg == "8000") {
                set_samplerate(8000);
                return true;
            }
            if(arg == "11025") {
                set_samplerate(11025);
                return true;
            }
            if(arg == "16000") {
                set_samplerate(16000);
                return true;
            }
            if(arg == "22050") {
                set_samplerate(22050);
                return true;
            }
            if(arg == "32000") {
                set_samplerate(32000);
                return true;
            }
            if(arg == "44100") {
                set_samplerate(44100);
                return true;
            }
            if(arg == "48000") {
                set_samplerate(48000);
                return true;
            }
            if(arg == "96000") {
                set_samplerate(96000);
                return true;
            }
        }
        return false;
    };

    auto arg_filename = [&](const int argi, const std::string& arg) -> bool
    {
        if(argi >= 2) {
            if(file_exists(arg)) {
                add_to_playlist(arg);
                return true;
            }
        }
        return false;
    };

    auto parse = [&]() -> bool
    {
        int argi = -1;
        for(auto& arg : args) {
            if(++argi == 0) {
                continue;
            }
            else if(arg_command(argi, arg)) {
                /* do nothing */;
            }
            else if(arg_chip(argi, arg)) {
                /* do nothing */;
            }
            else if(arg_channels(argi, arg)) {
                /* do nothing */;
            }
            else if(arg_samplerate(argi, arg)) {
                /* do nothing */;
            }
            else if(arg_filename(argi, arg)) {
                /* do nothing */;
            }
            else {
                invalid_argument(arg);
            }
            if(command == COMMAND_HELP) {
                break;
            }
        }
        return true;
    };

    auto play = [&]() -> void
    {
        Player player(settings, playlist);
            
        return player.play();
    };

    auto dump = [&]() -> void
    {
        Player player(settings, playlist);
            
        return player.dump();
    };

    auto execute = [&]() -> void
    {
        switch(command) {
            case COMMAND_HELP:
                help(args);
                break;
            case COMMAND_PLAY:
                play();
                break;
            case COMMAND_DUMP:
                dump();
                break;
            default:
                throw std::runtime_error("the command is not supported");
                break;
        }
    };

    auto run = [&]() -> void
    {
        if(parse()) {
            execute();
        }
    };

    return run();
}

void Program::help(const ArgList& args)
{
    auto program = [&]() -> const char*
    {
        const char* arg0  = args[0].c_str();
        const char* slash = ::strrchr(arg0, '/');
        if(slash != nullptr) {
            return slash + 1;
        }
        return arg0;
    };

    auto usage = [&]() -> void
    {
        std::cout << "Usage: " << program() << " <command> [OPTION...] [FILE...]" << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "Command:"                                                   << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "    help                display this help"                  << std::endl;
        std::cout << "    play                play audio"                         << std::endl;
        std::cout << "    dump                dump audio to stdout"               << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "Chip-Type:"                                                 << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "    ay8910              AY-3-8910"                          << std::endl;
        std::cout << "    ay8912              AY-3-8912"                          << std::endl;
        std::cout << "    ay8913              AY-3-8913"                          << std::endl;
        std::cout << "    ym2149              YM2149"                             << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "Channels:"                                                  << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "    mono                mono output"                        << std::endl;
        std::cout << "    stereo              stereo output"                      << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "Sample-Rate:"                                               << std::endl;
        std::cout << ""                                                           << std::endl;
        std::cout << "    8000                phone quality"                      << std::endl;
        std::cout << "    16000               cassette quality"                   << std::endl;
        std::cout << "    32000               broadcast quality"                  << std::endl;
        std::cout << "    11025               AM quality"                         << std::endl;
        std::cout << "    22050               FM quality"                         << std::endl;
        std::cout << "    44100               CD quality"                         << std::endl;
        std::cout << "    48000               DVD quality"                        << std::endl;
        std::cout << "    96000               BRD quality"                        << std::endl;
        std::cout << ""                                                           << std::endl;
    };

    return usage();
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try {
        Program::main(ArgList(argv, argv + argc));
    }
    catch(const std::exception& e) {
        Console::errorln("error: %s", e.what());
        return EXIT_FAILURE;
    }
    catch(...) {
        Console::errorln("error: %s", "unexpected exception");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
