typedef struct Example {
    int a;
    int b;
    float c;
} Example;

typedef struct Nested {
    int a;
    int b;

    struct {
        int c;
        int d;
    } fancy;
} Nested;

typedef struct Comment {
    //cpp style
    int cpp;
    /* c style */
    float c;
} Comment;
