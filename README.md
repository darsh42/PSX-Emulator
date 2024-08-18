# PSX-Emulator

PSX-Emulator is emulator for the Playstation 1 developed in C. The project was built with the aim of furthering my knowledge surrounding programming fundamentals
and computer architecture.

1.  [Quick Start](#Quick-Start)
2.  [Design](#Design)

## Quick Start
Dependencies
```
    SDL2, gnu-readline (for debugger), OpenGL version 4.6
```

Clone the repository
```
    git clone https://github.com/darsh42/PSX-Emulator.git
```

Compile the Source code
```
     make
```

Launch the Emulator
```
    make run
```

## Design

Explanations of the source code and architecture can be found in the "docs.md" file

### Project Structure
    include/      - contains all of the header files
    src/          - contains all of the source files
        core/     - contains the main device files of the PSX
        debug/    - contains the commandline debugger for the project
        renderer/ - contains the renderer of the project
    shaders/      - contains the shaders used in the renderer
    
    Makefile      - make rules for compilation and running
    docs          - further details of the PSX architecture
    README        - this document

#### include/
All header files are contained here, they contain the structures and enumerations used in the implementation of the components. They also include any public functions that can be accessed by other components or modules.

#### core/
Device implementations. The C files emulate each device on the PSX.
Each file is designed using a basic pattern
    
    Structure definitions      (static variables)
    Function declarations      (static declarations)
    External/Public functions  (extern functions)
    Internal/Private functions (static functions)

The files are structured like this to make the important function implementations, e.g reset and stepping, are at the top of the file, which will hopefully help in understanding the source code.
