# psx emulator
the project is an in-progress emulator for sony's original playstation.

## Pre-requisites
Download the required build tools and libraries
```bash
apt install cmake make libsdl2-dev
```

## Building
Clone and compile the executable
```bash
git clone https://github.com/darsh42/PSX-Emulator && cd PSX-Emulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
make -C build
```

## Usage
Currently the emulator is ran on the command line. It requires a psx bios, the
recommended bios is the SCPH1001.bin as it was used for development.

After installing the bios you can either use make to run the program or manually
pass the required parameters

#### Using make(Recommended):
Copy the bios into the build directory, make sure the name of the file has been
changed to BIOS.bin (the make rule specifically looks for this file).
```bash
make run -C build
```

#### Manual
Locate the executable in the build directory, then pass the correct files to
each flag. You can learn more about the available flags by just running the
executable without any parameters.
```bash
./build/emulator -b <BIOS.bin> -g <GAME.bin> -e <SIDELOADEXE.bin>
```

## Make Commands
Some custom make commands to make running the emulator easier.

```bash
make -C build
make run -C build
make run-debug -C build
make perf-record-general -C build
make perf-record-cache -C build
```

###### Assumptions:
- bios is in build directory with name BIOS.bin
- game is in build directory with name GAME.bin

## Goals
The project was designed to help me understand computer architecture and low-
level systems.

The motivations for choosing this project over others are as follows:

- Interest. Computer architecture, Systems programming, low-level programming,
  Graphics, Sound processing and Operating Systems are all areas that I find
  are very interesting. Within this project I touch all of these areas, most
  importantly I touch all of the areas with sufficient depth allowing me to
  study the topic and gain some practical level of understanding.

- Scale. The project is pretty large in scope. Designing each of the hardware
  systems in software is a challenge, both in a techincal sense and in a planning
  and system design sense.

- Diversity. The project lends itself well to trying many different disciplines
  within Computer Science. Some of the less apparent areas that I experimented
  with were parrallel progamming, disassemblers and decompilers, jit compilers,
  GUI design, creating debuggers, etc.  This further increased the learning
  footprint of the project.

- Tools. Since the project was large, it allowed me to test and learn new tools.
  It allowed for comparing and contrasting things liked debuggers, editors, profilers,
  build systems, etc. Having a central point of view allows for a better conclusion
  as to why certain tool preferences are justified.

## Contact
Darsh Chanduke - dchanduka@hotmail.com
