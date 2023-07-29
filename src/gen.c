/**
 * gen.c
 * Noah Trupin
 *
 * bytecode generator
 */

#include <stdio.h>
#include <string.h>
#include "gen.h"
#include "instr.h"
#include "parser.h"

#define g(x) if(!_gen(f, x)) goto err
#define bit16(x) t_instr=x; fwrite(&t_instr, sizeof(t_instr), 1, f)
#define bitInt(x) fwrite(&x, sizeof(x), 1, f)
#define string(x) bit16(strlen(x)); fwrite(&x, sizeof(char), strlen(x), f)
int _gen(FILE *f, node *n) {
    int16_t t_instr;

    switch (n->type) {
        // functions
        case FN: bit16(IDECL); string(n->id_val); break;
        case CALL: bit16(CALL); string(n->id_val); break;

        // constant
        case CST: bit16(IPUSH); bitInt(n->int_val); break;

        // operators
        case ADD: g(n->o1); g(n->o2); bit16(IADD); break;
        case SUB: g(n->o1); g(n->o2); bit16(ISUB); break;
        case MUL: g(n->o1); g(n->o2); bit16(IMUL); break;
        case DIV: g(n->o1); g(n->o2); bit16(IDIV); break;
        case EQ: g(n->o1); g(n->o2); bit16(IEQ); break;
        case NEQ: g(n->o1); g(n->o2); bit16(INEQ); break;

        // basic expr/halt
        case EXPR: g(n->o1); bit16(IPOP); break;
        case ROOT: g(n->o1); bit16(HALT); break; 
    }

    return 1;

err:
    return 0;
}

int gen(char *out, node *n) {
    if (!out || !n) return 0;

    FILE *f;
    f = fopen(out, "wb");

    int ret = _gen(f, n); 

    fclose(f);

    return ret;
}
