#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "lexer.h"
#include "error.h"

/* Util */

int is_digit(int x) {
    return x >= '0' && x <= '9';
}

int is_alpha(int x) {
    return (x >= 'a' && x <= 'z')
        || (x >= 'A' && x <= 'Z');
}

/* Lexer */

char *words[] = {
    "fn", "if", "else", "do", "end", "return", NULL
};

lexer *new_lexer() {
    lexer *l = (lexer *)malloc(sizeof(lexer));
    if (!l) return NULL;
    l->line = 1;
    l->col = 1;
    l->pos = 0;
    l->ch = ' ';
    l->peek.ch = ' ';
    l->peek.ready = 0;
    return l;
}

void free_lexer(lexer *l) {
    if (l) free(l);
}

void next_ch(lexer *l) {
    if (l->peek.ch == '\n') {
        l->line += 1;
        l->col = 0;
    }
    l->pos += 1;
    l->col += 1;

    l->peek.ch = getchar();
}

int peek_ch(lexer *l) {
    int ch = getchar();
    ungetc(ch, stdin);
    return ch;
}

#define skip_whitespace for(;l->peek.ch==' '||l->peek.ch=='\n';next_ch(l))

int next_sym(lexer *l) {
    if (l->ch == EOI) {
        syntax_error(l->line, l->col, "unexpected EOI");
        goto err;
    }

    l->ch = l->peek.ch;
    l->sym = l->peek.sym;
    l->int_val = l->peek.int_val;

    memset(l->id_val, 0, sizeof(l->id_val));
    strcpy(l->id_val, l->peek.id_val);

    skip_whitespace;

    switch (l->peek.ch) {
        case EOF: l->peek.sym = EOI; break;
        case '{': next_ch(l); l->peek.sym = LBRA; break;
        case '}': next_ch(l); l->peek.sym = RBRA; break;
        case '(': next_ch(l); l->peek.sym = LPAR; break;
        case ')': next_ch(l); l->peek.sym = RPAR; break;
        case ';': next_ch(l); l->peek.sym = SEMI; break;
        case ',': next_ch(l); l->peek.sym = COMMA; break;
        case '+': next_ch(l); l->peek.sym = PLUS; break;
        case '-': next_ch(l); l->peek.sym = MINUS; break;
        case '*': next_ch(l); l->peek.sym = STAR; break;
        case '/': next_ch(l); l->peek.sym = SLASH; break;
        case '=': {
            if (peek_ch(l) == '=') {
                next_ch(l);
                l->peek.sym = EQEQ;
            } else
                l->peek.sym = EQUAL;
            next_ch(l);
            break;
        }
        case '!': {
            if (peek_ch(l) == '=') {
                next_ch(l);
                l->peek.sym = BANGEQ;
            } else 
                l->peek.sym = BANG;
            next_ch(l);
            break;
        }
        default:
            if (is_digit(l->peek.ch)) {
                // add int to l->int_val
                l->peek.int_val = 0;
                do {
                    // overflow check
                    if (l->peek.int_val > (INT_MAX - l->peek.ch) / 10) {
                        syntax_error(l->line, l->col, "integer overflow");
                        goto err;
                    }
                    l->peek.int_val = l->peek.int_val * 10 + (l->peek.ch - '0');
                    next_ch(l);
                } while (is_digit(l->peek.ch));
                l->peek.sym = INT;
            } else if (is_alpha(l->peek.ch)) {
                // add id to l->id_val
                int i = 0;
                do {
                    if (i >= 99) {// leave room for \0
                        syntax_error(l->line, l->col, "identifier too long");
                        goto err;
                    }
                    l->peek.id_val[i++] = l->peek.ch;
                    next_ch(l);
                } while (is_alpha(l->peek.ch) || l->peek.ch == '_');
                // can't forget our null terminator
                l->peek.id_val[i] = '\0';
                l->peek.sym = 0;

                // look for sym
                while (words[l->peek.sym] != NULL && strcmp(words[l->peek.sym], l->peek.id_val) != 0)
                    l->peek.sym++;

                // no id found
                if (words[l->peek.sym] == NULL)
                    l->peek.sym = ID;

            } else {
                printf("sym: '%c'\n", l->peek.ch);
                syntax_error(l->line, l->col, "unrecognized symbol");
                goto err;
            }
    }

    if (!l->peek.ready) {
        l->peek.ready = 1;
        return next_sym(l);
    }
    return 1;

err:
    return 0;
}
