/*
 * aym-playlist.h - Copyright (c) 2023-2024 - Olivier Poncet
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
#ifndef __AYM_Playlist_h__
#define __AYM_Playlist_h__

#include "aym-audio.h"
#include "aym-emulator.h"

// ---------------------------------------------------------------------------
// aym::Playlist
// ---------------------------------------------------------------------------

namespace aym {

class Playlist
{
public: // public interface
    Playlist();

    Playlist(const Playlist&) = default;

    Playlist& operator=(const Playlist&) = default;

   ~Playlist() = default;

    void add(const std::string& filename);

    bool get(std::string& filename);

    bool prev(std::string& filename);

    bool next(std::string& filename);

private: // private data
    std::vector<std::string> _files;
    size_t                   _index;
};

}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __AYM_Playlist_h__ */
