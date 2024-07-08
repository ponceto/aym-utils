# AYM·UTILS

A collection of utilities around the `AY-3-8910` and the `YM2149` sound chips.

## DESCRIPTION

This project provides some utilities and an emulator to deal with the [YM file format](http://leonard.oxg.free.fr/ymformat.html).

It currently supports:

  - the AY-3-8910 PSG (Programmable Sound Generator)
  - the YM2149 SSG (Software-Controlled Sound Generator)

## COMPILE AND RUN

### Install the dependencies

Under Debian and derivatives (Ubuntu, Mint, ...):

```
apt-get install build-essential liblhasa-dev
```

### Build the project

Move to the `src` directory:

```
cd src
```

Build the sources:

```
make
```

### Run the project

AYM·Player usage:

```
Usage: aym-player.bin <command> [OPTION...] [FILE...]

Command:

    help                display this help
    play                play audio
    dump                dump audio to stdout

Chip-Type:

    ay8910              AY-3-8910
    ay8912              AY-3-8912
    ay8913              AY-3-8913
    ym2149              YM2149

Channels:

    mono                mono output
    stereo              stereo output

Sample-Rate:

    8000                phone quality
    16000               cassette quality
    32000               broadcast quality
    11025               AM quality
    22050               FM quality
    44100               CD quality
    48000               DVD quality
    96000               BRD quality

```

Play the file `commando.ay` with all parameters to default:

```
aym-player.bin play commando.ay
```

Play the file `commando.ay` as a `AY-3-8910`, mono channel, and a sample rate of 11025Hz:

```
aym-player.bin play ay8910 mono 11025 commando.ay
```

Play the file `commando.ay` then `gryzor.ay` as a `YM2149`, stereo channels, and a sample rate of 44100Hz:

```
aym-player.bin play ym2149 stereo 44100 commando.ay gryzor.ay
```

## LICENSES

### AYM·UTILS

aym-utils, a collection of utilities around the `AY-3-8910` and the `YM2149` sound chips.

This project is released under the `GPLv2` license.

```
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
```

### MINIAUDIO

miniaudio, a single file audio playback and capture library written in C.

This library is embedded into this projet in split form.

  - https://miniaud.io/

This project is released under the `MIT No Attribution` license.

```
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

