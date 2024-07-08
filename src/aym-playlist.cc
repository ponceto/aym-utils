/*
 * aym-playlist.cc - Copyright (c) 2023-2024 - Olivier Poncet
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
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include "aym-playlist.h"

// ---------------------------------------------------------------------------
// aym::Playlist
// ---------------------------------------------------------------------------

namespace aym {

Playlist::Playlist()
    : _files()
    , _index()
{
}

void Playlist::add(const std::string& filename)
{
    _files.push_back(filename);
}

bool Playlist::get(std::string& filename)
{
    const size_t zero = 0;
    const size_t size = _files.size();
    const size_t curr = _index;

    if((curr >= zero) && (curr < size)) {
        filename = _files[curr];
        return true;
    }
    return false;
}

bool Playlist::prev(std::string& filename)
{
    const size_t zero = 0;
    const size_t size = _files.size();
    const size_t curr = (_index - 1);

    if((curr >= zero) && (curr < size)) {
        filename = _files[_index = curr];
        return true;
    }
    return false;
}

bool Playlist::next(std::string& filename)
{
    const size_t zero = 0;
    const size_t size = _files.size();
    const size_t curr = (_index + 1);

    if((curr >= zero) && (curr < size)) {
        filename = _files[_index = curr];
        return true;
    }
    return false;
}

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
