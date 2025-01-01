/*
 * console.cc - Copyright (c) 2023-2025 - Olivier Poncet
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
#include "console.h"

// ---------------------------------------------------------------------------
// Console
// ---------------------------------------------------------------------------

void Console::println(const char* format, ...)
{
    FILE* stream = stdout;

    va_list arguments;
    va_start(arguments, format);
    static_cast<void>(::vfprintf(stream, format, arguments));
    static_cast<void>(::fputc('\n', stream));
    static_cast<void>(::fflush(stream));
    va_end(arguments);
}

void Console::alertln(const char* format, ...)
{
    FILE* stream = stderr;

    va_list arguments;
    va_start(arguments, format);
    static_cast<void>(::vfprintf(stream, format, arguments));
    static_cast<void>(::fputc('\n', stream));
    static_cast<void>(::fflush(stream));
    va_end(arguments);
}

void Console::errorln(const char* format, ...)
{
    FILE* stream = stderr;

    va_list arguments;
    va_start(arguments, format);
    static_cast<void>(::vfprintf(stream, format, arguments));
    static_cast<void>(::fputc('\n', stream));
    static_cast<void>(::fflush(stream));
    va_end(arguments);
}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
