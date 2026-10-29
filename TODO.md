## TODO List:

### Fast HashTable
- [X] Version with combined key + value struct

### Cutils Improvements
- [X] Update build Script
- [X] Custom locks
    - [X] Bakery Algorithm
    - [X] Compare against pure CAS
    - [X] Approximately Match Pthread
- [X] thread-safe printing
    - [X] locks
    - [X] locks on files
    - [X] enable/disable locks
- [X] File handling
    - [X] file local name
- [X] recursive mkdir

    

### Sbuild
- [X] Graph Data Structure
- [X] Working Commands + Fences
- [X] Auto Rebuild
- [X] Output compile_commands.json
    - [X] Logical Tasks
    - [X] parallel task graph
    - [X] Formatting
    - [X] integration
    - [X] Append Flags
- [ ] additional compilation funcs
    - [ ] git
    - [ ] curl

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
- [ ] projection
    - [ ] calc proper formula (inner product followed by geometric product)
    - [ ] revist rejection just in case
- [ ] Sandwich product
    - [ ] point
    - [ ] line
    - [ ] plane
- [ ] rotors
- [ ] translators
- [ ] motors
- [ ] matricies

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
- [X] Hashmap struct equality
- [X] fix String Map
    - [X] string storage
    - [X] rewrite
    - [X] test swiss/swar map
- [ ] Queue
    - [X] Fixed Size (small)
    - [X] Dynamic Queue
    - [ ] Large Size (using pages)
    - [ ] Dynamic (idk pick a data structure)
- [ ] Circular Buffer
    - [ ] Fixed Size (small)
    - [ ] Fixed Size (large)
- [ ] Sparse Set
- [ ] Bin Tree
- [ ] Trie?

### Cutils String Handling
- [X] Merge StrBase
- [ ] Improve StrBase
    - [ ] Chunked Lists for storage O(n) allocations -> O(log n)
    - [ ] Static StrBase (equivalent functionality, but all strings not already in the cache get the same id)
    - [ ] StrBase Clear (removes all strings and resets strbase)

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
    - [X] Print Type
    - [ ] Parse Type
        - [X] Base Type
        - [X] Pointers
        - [X] Consts
        - [ ] Struct
        - [ ] Union
    - [ ] Parse Map
    - [X] Multiline Directives
    - [X] Error Handler


### Re-Architecting
- [X] Switch to Static Library
    - [X] cutils
    - [X] SimpleDS
    - [X] StringBase
