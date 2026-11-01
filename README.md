# 6502-Emulator-C++20

A modular MOS 6502 CPU emulator core built with modern C++20 modules and automated unit testing.

## Overview

A modern C++ implementation of the MOS 6502 processor, designed with clean architectural boundaries and test-driven verification.

**Status**: Work in progress.

## Current Progress

- **Addressing Modes**: Immediate, Zero Page, Zero Page X, Zero Page Y, Absolute, Absolute X, Absolute Y, Indirect Indexed, Indexed Indirect
- **Core Instructions**:
  - **Load Family**: `LDA`, `LDX`, `LDY` (verified with flag mutation logic)
  - **Store Family**: `STA`, `STX`, `STY` (verified across all supported addressing modes with flag preservation)
  - **Control Flow**: `JSR`

## Build

### Option 1: CPM / FetchContent

**Requirements**:

- A compiler supporting the latest C++ standards (C++20 / C++23)
- CMake (3.28+)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Option 2: Conan 2

**Requirements**:

- Python (3.8+)
- A compiler supporting the latest C++ standards
- CMake (3.28+)
- Make

```bash
make conan_d   # for Debug build
# or: make conan_r for Release build

cd build
cmake ..
cmake --build .
```

**Tip**: For users who prefer storing Conan profiles and packages outside the system drive:

```powershell
[System.Environment]::SetEnvironmentVariable('CONAN_HOME', '..Place You Like..', 'User')
```

## Documentation

See `Notes.md` for technical design notes and execution logic details.

