/**
 * vm.c
 * Noah Trupin
 *
 * the bytecode virtual machine
 */

#include <stdio.h>
#include "vm.h"
#include "gen.h"

#define r(x) fread(&x, sizeof(x), 1, f)
int vm(char *in) {
    FILE *f;
    f = fopen(in, "rb");
    if (!f) return 0;

    int stack[1000], *sp = stack;
    int16_t instr;
    int a;
loop:
    fread(&instr, sizeof(instr), 1, f);
    switch (instr) {
        case IPUSH: r(a); *sp++ = a; goto loop;
        case IPOP: --sp; goto loop;
        case IADD: sp[-2] = sp[-2] + sp[-1]; --sp; goto loop;
        case HALT: break;
    }

    printf("RESULT: %d\n", *sp);

    fclose(f);

    return 1;
}
