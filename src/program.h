/*
 * program.h - Copyright (c) 2023-2024 - Olivier Poncet
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
#ifndef __Program_h__
#define __Program_h__

// ---------------------------------------------------------------------------
// ArgList
// ---------------------------------------------------------------------------

using ArgList = std::vector<std::string>;

// ---------------------------------------------------------------------------
// Program
// ---------------------------------------------------------------------------

struct Program
{
    static void main(const ArgList& args);

    static void help(const ArgList& args);
};

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __Program_h__ */
