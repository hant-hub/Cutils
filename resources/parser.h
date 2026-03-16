struct basic {
    int a;
    float b;
    int c;
};

struct pointer {
    int* t;
};

struct triple_pointer {
    int*** t;
};

struct ref {
    struct basic g;
};

typedef struct alias {
    int a;
    int b;
} typedAlias;

struct tref {
    typedAlias a;
};

typedef int u32;
