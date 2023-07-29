#ifndef PARSER_H
#define PARSER_H

// node types
enum {
    // binary ops
    ADD, SUB, MUL, DIV, SET, EQ, NOT, NEQ,

    ROOT, EMPTY, FN, IF1, IF2, SEQ, EXPR, CST, VAR, CALL,
    RETURN,
};

// associativity
enum {
    LEFT, RIGHT
};

// ast node
struct node {
    int type;
    // nodes have a max of three children
    struct node *o1, *o2, *o3;
    int int_val;
    char id_val[100];
};
typedef struct node node;

// public functions
node *program();
void free_node(node *);
void print_node(node *, int);

#endif
