/*
    Re-used so no header guard
*/

#include <core/cutils.h>

#ifndef NAME
#error Must Define NAME as macro
#define NAME map
#endif

#ifndef KEY
#error Must Define KEY as macro
#define KEY u32
#endif

#ifndef VAL
#error Must Define VAL as macro
#define VAL u32
#endif

#define ENTRY_NAME CAT(NAME, _entry)
typedef struct ENTRY_NAME {
    KEY k;
    VAL v;
} ENTRY_NAME;
