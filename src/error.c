#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void syntax_error(int line, int col, const char *msg) {
    fprintf(stderr, "[%d:%d] error: %s\n", line, col, msg);
}
