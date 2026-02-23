## TODO List:

### Scratch Allocators
- [X] Reserve approach to Scratch allocators
    - [X] mmap
    - [ ] perf testing
    - [ ] string building
        - [ ] Arena usage

### Cutils String Handling
- [X] Merge StrBase
- [ ] Scratch Allocator Based funcs
- [ ] More print options
    - [ ] debuglog -> no line nums
    - [ ] strarray format -> array of formats

### Centralization
- [ ] Improve Cutils Tooling
    - [X] Grouped Test Runner
    - [ ] Better Test Runner
- [X] Investigate Merging SimpleDS into Cutils
    - [X] ds.h include
    - [X] Tests
- [X] Investigate Merging StringBase into Cutils
    - [X] Fix StrBase
    - [X] Add into header?

### Re-Architecting
- [ ] Consider Switching to Static Library
    - [ ] Make lightweight version for scripting?
    - [ ] Split into scripts and utilities
    - [ ] merge simple engine rendering code?
