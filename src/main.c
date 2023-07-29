#include <stdio.h>
#include "parser.h"
#include "gen.h"
#include "vm.h"

int main() {
    node *n = program();
    print_node(n, 0);
    gen("a.out", n);
    vm("a.out");
    free_node(n);
}
