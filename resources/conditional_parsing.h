
//test directives
#define test
#define multiline a \
    b \
    c

#define start_parse(...)
#define end_parse(...)


struct a {
    int content;
};

start_parse(1, 2);

struct b {
    int secret;
};

end_parse();
