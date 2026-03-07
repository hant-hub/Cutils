## TODO List:

### Cutils Array Slices
- [X] Array Slice

### Scratch Allocators
- [X] Reserve approach to Scratch allocators
    - [X] mmap
    - [ ] perf testing
    - [ ] string building
        - [ ] Arena usage

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
- [ ] Sparse Set
- [ ] Bin Tree
- [ ] Trie?

### Cutils String Handling
- [X] Merge StrBase
- [ ] Scratch Allocator Based funcs
- [ ] More print options
    - [ ] debuglog -> no line nums
    - [ ] strarray format -> array of formats

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
    - [ ] Direct Merge
    - [ ] Better universal Helpers
    - [ ] Better Error handling

### Re-Architecting
- [ ] Consider Switching to Static Library
    - [ ] Make lightweight version for scripting?
    - [ ] Split into scripts and utilities
    - [ ] merge simple engine rendering code?
