## TODO List:



### Cutils Math Module
- [ ] finish current functions
- [ ] Sandwich product
    - [ ] point
    - [ ] line
    - [ ] plane
- [ ] rotors
- [ ] translators
- [ ] motors



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
    - [ ] Parse Type
        - [ ] Base Type
        - [ ] Pointers
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
