/**
 * parser.c
 * Noah Trupin
 *
 * a hand-written recursive-descent parser with an operator-precedence parsing
 * (pratt parsing) component for mathematical expressions.
 *
 * this file contains functions for creating, freeing, and printing ast nodes
 * alongside standard parsing functions.
 *
 * error handling is done through gotos to cleanup nodes and provide nice
 * tracing for the programmer.
 *
 * parser functions are commented in ebnf form.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "error.h"

node *new_node(int type) {
    node *n = (node *)malloc(sizeof(node));
    if (!n) return NULL;
    n->type = type;
    n->o1 = NULL;
    n->o2 = NULL;
    n->o3 = NULL;
    return n;
}

void free_node(node *n) {
    if (!n) return;
    free_node(n->o1);
    free_node(n->o2);
    free_node(n->o3);
    free(n);
    n = NULL;
}


// operator precedence table
const int op_precs[] = {
    [EQUAL] = 2,
    [EQEQ] = 9,
    [BANGEQ] = 9,
    [PLUS] = 12,
    [MINUS] = 12,
    [STAR] = 13,
    [SLASH] = 13,
    [BANG] = 14,
};

// operator associativity table
const int op_assocs[] = {
    [EQUAL] = RIGHT,
    [EQEQ] = LEFT,
    [BANGEQ] = LEFT,
    [PLUS] = LEFT,
    [MINUS] = LEFT,
    [STAR] = LEFT,
    [SLASH] = LEFT,
    [BANG] = RIGHT,
};

// operator words (for printing)
const char *op_words[] = {
    "add", "sub", "mul", "div", "set", "eq", "not", "neq"
};

void print_node(node *n, int s) {
    if (!n) return;
    switch (n->type) {
        case ROOT: {
            printf("%*sROOT\n", s, "");
            print_node(n->o1, s + 2);
            break;
        }
        case FN: {
            printf("%*sfn %s\n", s, "", n->id_val);
            printf("%*sargs\n", s, "");
            print_node(n->o1, s + 2);
            printf("%*sbody\n", s, "");
            print_node(n->o2, s + 2);
            break;
        }
        case CALL: {
            printf("%*scall %s\n", s, "", n->id_val);
            print_node(n->o1, s + 2);
            break;
        }
        case RETURN: {
            printf("%*sreturn\n", s, "");
            print_node(n->o1, s + 2);
            break;
        }
        case IF1: {
            printf("%*sif\n", s, "");
            print_node(n->o1, s + 2);
            printf("%*sthen\n", s, "");
            print_node(n->o2, s + 2);
            break;
        }
        case SEQ: {
            print_node(n->o1, s);
            print_node(n->o2, s);
            break;
        }
        case EXPR: {
            print_node(n->o1, s);
            break;
        }
        case ADD ... (ROOT - 1): {
            printf("%*s%s\n", s, "", op_words[n->type]);
            print_node(n->o1, s + 2);
            print_node(n->o2, s + 2);
            break;
        }
        case CST: {
            printf("%*sconst %d\n", s, "", n->int_val);
            break;
        }
        case VAR: {
            printf("%*svar %s\n", s, "", n->id_val);
            break;
        }
    }
}

#define advance if (!next_sym(l)) goto err;

// binop check
int is_binop(int x) {
    return x >= PLUS && x < EOI;
}

node *paren_expr(lexer *);
node *statement(lexer *);
node *expr(lexer *);
node *list(lexer *, node *(lexer *), int, int);

// <id> = identifier
node *id(lexer *l) {
    node *n = NULL;
    int el = l->line, ec = l->col;

    if (l->sym == ID) {
        n = new_node(VAR);
        strcpy(n->id_val, l->id_val);
    } else {
        syntax_error(el, ec, "unexpected symbol");
        goto err;
    }

    advance;

    return n;

err:
    syntax_error(el, ec, "in ID");
    free_node(n);
    return NULL;
}

// <primary> = <id>
//           | <id> <list<expr>>
//           | <int>
//           | <paren_expr>
node *primary(lexer *l) {
    node *n = NULL, *n2 = NULL;
    int el = l->line, ec = l->col;

    if (l->sym == ID) {
        // variable or func call
        if ((n = id(l)) == NULL)
            goto err;

        // arg list
        if (l->sym == LPAR) {
            n->type = CALL;

            // arg list
            if ((n->o1 = list(l, expr, LPAR, RPAR)) == NULL)
                goto err;

            advance;
        }
    } else if (l->sym == INT) {
        // constant (int)
        n = new_node(CST);
        n->int_val = l->int_val;
        advance;
    } else
        if ((n = paren_expr(l)) == NULL)
            goto err;

    return n;

err:
    syntax_error(el, ec, "in PRIMARY");
    free_node(n);
    free_node(n2);
    return NULL;
}

// <expr> = pratt parser (see comment on expr)
node *expr_1(lexer *l, node *lhs, int min_p) { 
    node *t = NULL, *rhs = NULL;
    int el = l->line, ec = l->col;
    int op, look;

    look = l->sym;

    while (is_binop(look) && op_precs[look] > min_p) {
        op = look;

        // rhs
        advance;
        if ((rhs = primary(l)) == NULL)
            goto err;

        look = l->sym;
        while (is_binop(look)
            && (op_precs[look] > op_precs[op]
            || (op_assocs[look] == RIGHT && 
                op_precs[look] == op_precs[op]))) {
            int prec_add = op_precs[look] > op_precs[op] ? 1 : 0;
            if ((rhs = expr_1(l, rhs, op_precs[op] + prec_add)) == NULL)
                goto err;
            look = l->sym;
        }
        // aligned enums, just subtract
        t = new_node(op - PLUS);
        t->o1 = lhs;
        t->o2 = rhs;
        lhs = t;
    }
    return lhs;

err:
    syntax_error(el, ec, "in EXPR");
    free_node(lhs);
    free_node(rhs);
    free_node(t);
    return NULL;
}

// <expr> = ...
// this is the entrance for an operator-precedence (pratt) parser
// makes the parser a lot more simple / extensible than using 
// recursive-descent for mathematical expressions.
node *expr(lexer *l) {
    node *n = expr_1(l, primary(l), 0);
    return n;
err:
    free_node(n);
    return NULL;
}

// <paren_expr> = "(" <expr> ")"
node *paren_expr(lexer *l) {
    node *n = NULL;
    int el = l->line, ec = l->col;

    // opening paren
    if (l->sym != LPAR) {
        syntax_error(l->line, l->col, "expected '('");
        goto err;
    }
    advance;

    // body
    if ((n = expr(l)) == NULL)
        goto err;

    // closing paren
    if (l->sym != RPAR) {
        syntax_error(l->line, l->col, "expected ')'");
        goto err;
    }
    advance;

    return n;

err:
    syntax_error(el, ec, "in PAREN EXPR");
    free_node(n);
    return NULL;
}

// <statement> = "if" <paren_expr> <statement>
//             | "if" <paren_expr> <statement> "else" <expr>
//             | "return" <statement>
//             | "do" { <statement> } "end"
//             | <expr>
node *statement(lexer *l) {
    node *n = NULL, *n2 = NULL;
    int el = l->line, ec = l->line;
    if (l->sym == IF_SYM) {
        n = new_node(IF1);
        
        // expr
        advance;
        if ((n->o1 = paren_expr(l)) == NULL)
            goto err;

        // body
        if ((n->o2 = statement(l)) == NULL)
            goto err;

        // check for else
        if (l->sym == ELSE_SYM) {
            n->type = IF2;
            advance;

            // body
            if ((n->o3 = statement(l)) == NULL)
                goto err;
        }
    } else if (l->sym == DO_SYM) {
        // sequence
        n = new_node(EMPTY);
        advance;
        while (l->sym != END_SYM) {
            n2 = n;
            n = new_node(SEQ);
            n->o1 = n2;
            if ((n->o2 = statement(l)) == NULL)
                goto err;
        }
        advance;
    } else if (l->sym == RET_SYM) {
        n = new_node(RETURN);

        // ret expr
        advance;
        if ((n->o1 = statement(l)) == NULL)
            goto err;
    } else {
        n = new_node(EXPR);

        // body
        if ((n->o1 = expr(l)) == NULL)
            goto err;
    }

    return n;

err:
    syntax_error(el, ec, "in STATEMENT");
    free_node(n);
    return NULL;
}

// <block> = "fn" <id> "(" ")" <statement> 
//         | <statement>
node *block(lexer *l) {
    node *n = NULL;

    // save line/col for trace
    int el = l->line, ec = l->col;
    if (l->sym == FN_SYM) {
        n = new_node(FN);

        // function name
        advance;
        if (l->sym != ID) {
            syntax_error(l->line, l->col, "expected ident");
            goto err;
        }
        strcpy(n->id_val, l->id_val);

        // arg list
        advance;
        if ((n->o1 = list(l, id, LPAR, RPAR)) == NULL)
            goto err;

        // function body
        advance;
        if ((n->o2 = statement(l)) == NULL)
            goto err;

    } else
        // allow statements on top level
        return statement(l);

    return n;

err:
    syntax_error(el, ec, "in BLOCK");
    free_node(n);
    return NULL;
}

// <list<f>> = "<start>" [ <f> { "," <f> } ] "<end>"
node *list(lexer *l, node *(f)(lexer *), int start, int end) {
    node *n = NULL, *n2 = NULL;
    int el = l->line, ec = l->col;

    // check start sym
    if (l->sym != start) {
        syntax_error(el, ec, "missing start of list");
        goto err;
    }

    // init node
    n = new_node(EMPTY);

    advance;
    while (l->sym != end) {
        n2 = n;
        n = new_node(SEQ);
        n->o1 = n2;

        // item
        if ((n->o2 = f(l)) == NULL)
            goto err;

        // check for comma or end
        if (l->sym == COMMA) {
            advance;
        } else if (l->sym != COMMA && l->sym != end) {
            syntax_error(el, ec, "unexpected end of arg list");
            goto err;
        }
    }

    return n;
err:
    syntax_error(el, ec, "in LIST");
    free_node(n);
    return NULL;
}

// <program> = { <block> }
node *program() {
    node *n = NULL, *n2 = NULL;

    // create a new lexer
    lexer *l = new_lexer();
    if (!l) {
        fprintf(stderr, "failed to init lexer");
        return NULL;
    }

    // root node
    n = new_node(ROOT);

    // save line and col for error
    int el = l->line, ec = l->col;
    // parse program, check for failure

    advance;
    while (l->sym != EOI) {
        n2 = n->o1;
        n->o1 = new_node(SEQ);
        n->o1->o1 = n2;
        if ((n->o1->o2 = block(l)) == NULL)
            goto err;
    }

    free_lexer(l);

    return n;

err:
    syntax_error(el, ec, "in ROOT");
    free_lexer(l);
    free_node(n);
    return NULL;
}
