#
# Makefile - Copyright (c) 2023-2024 - Olivier Poncet
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# ----------------------------------------------------------------------------
# global environment
# ----------------------------------------------------------------------------

OPTLEVEL = -O2 -g
WARNINGS = -Wall
EXTRAS   = -pthread -fstack-protector-strong
CC       = gcc
CFLAGS   = -std=c99 $(OPTLEVEL) $(WARNINGS) $(EXTRAS)
CXX      = g++
CXXFLAGS = -std=c++14 $(OPTLEVEL) $(WARNINGS) $(EXTRAS)
CPPFLAGS = -I. -I/usr/include/liblhasa-1.0 -D_DEFAULT_SOURCE -D_FORTIFY_SOURCE=2
LD       = g++
LDFLAGS  = -L.
CP       = cp
CPFLAGS  = -f
RM       = rm
RMFLAGS  = -f

# ----------------------------------------------------------------------------
# default rules
# ----------------------------------------------------------------------------

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $<

# ----------------------------------------------------------------------------
# global targets
# ----------------------------------------------------------------------------

all: build

build: build_aym_player
	@echo "=== $@ ok ==="

clean: clean_aym_player
	@echo "=== $@ ok ==="

# ----------------------------------------------------------------------------
# aym_player files
# ----------------------------------------------------------------------------

aym_player_PROGRAM = aym-player.bin

aym_player_SOURCES = \
	miniaudio.c \
	aym-audio.cc \
	aym-emulator.cc \
	aym-player.cc \
	lha-stream.cc \
	ym-archive.cc \
	console.cc \
	program.cc \
	$(NULL)

aym_player_HEADERS = \
	miniaudio.h \
	aym-audio.h \
	aym-emulator.h \
	aym-player.h \
	lha-stream.h \
	ym-archive.h \
	console.h \
	program.h \
	$(NULL)

aym_player_OBJECTS = \
	miniaudio.o \
	aym-audio.o \
	aym-emulator.o \
	aym-player.o \
	lha-stream.o \
	ym-archive.o \
	console.o \
	program.o \
	$(NULL)

aym_player_LDFLAGS = \
	$(NULL)

aym_player_LDADD = \
	-llhasa -ldl -lpthread -lm \
	$(NULL)

aym_player_CLEANFILES = \
	aym-player.bin \
	$(NULL)

# ----------------------------------------------------------------------------
# build aym_player
# ----------------------------------------------------------------------------

build_aym_player: $(aym_player_PROGRAM)

$(aym_player_PROGRAM): $(aym_player_OBJECTS)
	$(LD) $(LDFLAGS) $(aym_player_LDFLAGS) -o $(aym_player_PROGRAM) $(aym_player_OBJECTS) $(aym_player_LDADD)

# ----------------------------------------------------------------------------
# clean aym_player
# ----------------------------------------------------------------------------

clean_aym_player:
	$(RM) $(RMFLAGS) $(aym_player_OBJECTS) $(aym_player_CLEANFILES)

# ----------------------------------------------------------------------------
# End-Of-File
# ----------------------------------------------------------------------------
