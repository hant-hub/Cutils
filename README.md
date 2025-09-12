CUtils is a library which attempts
to provide general utilities and updates to the
core C standard library.

It also comes with many convenience functions
with the goal of improving development alongside some
replacements for the C standard library.

Dependencies for testing include valgrind and clang-format

## Features:
- Width types for fundamental types (u8, u16, u32, etc.)
- Sized string support
    - Sstrcmp
    - Sstrtok
    - Sstrdup
- Logging functions
    - line numbers + file
    - Colorized
    - Sized string support
    - Multi channel support (split info, warn, and errors)
- Custom file handling
    - Set working directory to executable location
    - Control file buffering 
    - Read and write
    - Full file load and save
    - delete files
- Custom allocators
    - Pass around custom allocators
    - Global allocator (malloc and free)
    - Stack allocator arena
