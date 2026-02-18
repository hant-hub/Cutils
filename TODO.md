## TODO List:

### Scratch Allocators
- [X] Reserve approach to Scratch allocators
    - [X] mmap
    - [ ] perf testing
    - [ ] string building
        - [ ] Arena usage

### Cutils String Handling
- [ ] Merge StrBase
- [ ] Scratch Allocator Based funcs
- [ ] More print options
    - [ ] debuglog -> no line nums
    - [ ] strarray format -> array of formats

### Centralization
- [ ] Improve Cutils Tooling
    - [ ] Grouped Test Runner
    - [ ] Better Test Runner
- [ ] Investigate Merging SimpleDS into Cutils
    - [X] ds.h include
    - [ ] Tests
- [ ] Merge SimpleBuild into Cutils
    - [ ] Include file
    - [ ] Tests
- [ ] Investigate Merging StringBase into Cutils
    - [ ] Fix StrBase
    - [ ] Add into header?

### Re-Architecting
- [ ] Consider Switching to Static Library
    - [ ] Make lightweight version for scripting?
    - [ ] Split into scripts and utilities
    - [ ] merge simple engine rendering code?
