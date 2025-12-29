#include <ctype.h>
#include <cutils.h>


int main() {

    //assert equivalent to ctype
    for (int c = 0; c < 128; c++) {
        int cu = isAlpha(c);
        int cy = !!isalpha(c);
        if (cu != cy) {
            printlog("%d, %d %d\n", c, cu, cy);
            panic();
        }
    }

    for (int c = 0; c < 128; c++) {
        int cu = isNum(c);
        int cy = !!isdigit(c);
        if (cu != cy) {
            printlog("%d, %d %d\n", c, cu, cy);
            panic();
        }
    }

    for (int c = 0; c < 128; c++) {
        int cu = isAlNum(c);
        int cy = !!isalnum(c);
        if (cu != cy) {
            printlog("%d, %d %d\n", c, cu, cy);
            panic();
        }
    }

    for (int c = 0; c < 128; c++) {
        int cu = isSpace(c);
        int cy = !!isspace(c);
        if (cu != cy) {
            printlog("%d, %d %d\n", c, cu, cy);
            panic();
        }
    }



    return 0;
}
