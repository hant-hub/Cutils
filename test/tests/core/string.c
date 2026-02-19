#include <core/cutils.h>

int main() {
    SString test = sstring("bar");
    SString test2 = sstring("foo");

    bool8 c = Sstrcmp(test, test);
    assert(c);

    c = Sstrcmp(test2, test);
    assert(!c);

    SString new = Sstrdup(GlobalAllocator, test);

    c = Sstrcmp(new, test);
    assert(c);

    Free(GlobalAllocator, new.data, new.len);

    SString sentence = sstring("This is a sentence");

    SString tok = Sstrtok(sentence, ' ');
    assert(Sstrcmp(tok, sstring("This")));

    tok = Sstrtok((SString){0}, ' ');
    assert(Sstrcmp(tok, sstring("is")));

    tok = Sstrtok((SString){0}, ' ');
    assert(Sstrcmp(tok, sstring("a")));

    tok = Sstrtok((SString){0}, ' ');
    assert(Sstrcmp(tok, sstring("sentence")));

    tok = Sstrtok((SString){0}, ' ');
    assert(!tok.data && !tok.len);

    debuglog("String Test Passed");
}
