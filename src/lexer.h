#ifndef LEXER_H
#define LEXER_H

// lexer types
enum { 
    // literals
    FN_SYM, IF_SYM, ELSE_SYM, DO_SYM, END_SYM, RET_SYM, ID, INT,

    // symbols
    LBRA, RBRA, LPAR, RPAR, SEMI, COMMA,

    // operators
    PLUS, MINUS, STAR, SLASH, EQUAL, EQEQ,
    BANG, BANGEQ, 

    // other
    EOI,
};

// lexer struct
typedef struct lexer {
    // position info
    int line;
    int col;
    int pos;

    // character info
    int ch;
    int sym;
    int int_val;
    char id_val[100];

    // lookahead info
    struct {
        int ready;
        int ch;
        int sym;
        int int_val;
        char id_val[100];
    } peek;
} lexer;

// public functions

lexer *new_lexer();
void free_lexer(lexer *);

// next symbol
// returns 1 on success, 0 on failure
int next_sym(lexer *);

 #endif
