#include <core/cutils.h>
#include <cgen/cgen.h>


int main() {

    ScratchArena sc = ScratchArenaGet(NULL);

    file testfile = fileopen(sstring("../resources/type_parsing.h"), FILE_READ); 

    SString buffer = {
        .data = ArenaAlloc(&sc.arena, testfile.stats.size),
        .len = testfile.stats.size,
    };
    fileread(buffer, testfile);
    fileclose(testfile);

    StrBase b = {
        .mem = GlobalAllocator,
    };

    //debuglog("Loaded Data");
    //printlog("%s\n", buffer);

    TokenBuffer tokens = Tokenize(buffer, &b, GlobalAllocator);

    printlog("Tokens:\n");
    u64 pos = ArenaGetPos(sc.arena);
    for (int i = 0; i < tokens.size; i++) {
        printlog("\t%s\n", TokenName(tokens.data[i], &b, &sc.arena));
        ArenaSetPos(&sc.arena, pos);
    }

    ParsingState parser = BeginParser(GlobalAllocator, &tokens, &b);
    u32 t = ParseType(&parser); 

    Type type = parser.type.types.data[t];
    Type inner = parser.type.types.data[type.inner];
    debuglog("Type: %s", TypeName(sc.arena, &parser, t));
    debuglog("inner: %s", TypeName(sc.arena, &parser, type.inner));
    debuglog("Base type? %d", inner.flags & TYPE_BASE);


    ScratchArenaEnd(sc);

    TokenBufferFree(&tokens);
    StrBaseFree(&b);

    TypeInfoFree(&parser.type);


    return 0;
}
