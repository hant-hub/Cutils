## TODO List:


### Cutils Image Library
- [X] PPM output
- [X] PAM output (for transparency)
- [X] Research BMP
- [X] Parsing
    - [X] Set up assets folder for tests
    - [X] Parse Headers
    - [X] Image size and compression
    - [X] Output bytes
- [ ] Handle Compressed BMP
    - [ ] 4 bit RLE
    - [ ] 8 bit RLE
    - [ ] 16 bit color
    - [ ] 8 bit color
    - [ ] 4 bit color
    - [ ] 1 bit color
- [ ] Saving
    - [ ] Build Headers
    - [ ] Minimal example

### Cutils Math Module
- [ ] finish current functions
- [ ] Sandwich product
    - [ ] point
    - [ ] line
    - [ ] plane
- [ ] rotors
- [ ] translators
- [ ] motors

## Cutils Sub Tasks
- [X] Convert small to big endian
- [X] convert big to small endian


### Cutils Array Slices
- [X] Array Slice

### Scratch Allocators
- [X] Reserve approach to Scratch allocators
    - [X] mmap

### Cutils File Improvements
- [X] Iter Directory
    - [X] Non Recursive
    - [X] Recursive
        - [X] filepath funcs
            - [X] Popdir
            - [X] Base name
            - [X] Get Ext
            - [X] Path Depth

### SimpleDS
- [ ] Hashmap struct equality
    - [ ] eq var
- [ ] Rewrite To Void*
    - [ ] rework api
        - [ ] stb style
        - [ ] struct with v pointer
        - [ ] macros for get and void*
    - [ ] dyn Array
- [ ] Sparse Set
- [ ] Bin Tree
- [ ] Trie?

### Cutils String Handling
- [X] Merge StrBase

### Centralization
- [X] Improve Cutils Tooling
    - [X] Grouped Test Runner
    - [X] Better Test Runner
- [X] Investigate Merging SimpleDS into Cutils
    - [X] ds.h include
    - [X] Tests
- [X] Investigate Merging StringBase into Cutils
    - [X] Fix StrBase
    - [X] Add into header?
- [ ] Merge CGen + Improve CGen code Quality
    - [ ] Rewrite!
    - [ ] Print Type
    - [ ] Parse Type
        - [ ] Base Type
        - [X] Pointers
        - [ ] Consts
        - [ ] Struct
        - [ ] Union
    - [ ] Parse Map
    - [ ] Multiline Directives
    - [ ] Error Handler


### Re-Architecting
- [X] Switch to Static Library
    - [X] cutils
    - [X] SimpleDS
    - [X] StringBase
