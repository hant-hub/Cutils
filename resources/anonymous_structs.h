
struct nested {
    struct inner {
        int one;
        int two;
    } a;
    int three;
};

struct nested_anonymous {
    struct {
        int one;
        int two;
    } a;
    struct {
        int w;
    } three;
};
