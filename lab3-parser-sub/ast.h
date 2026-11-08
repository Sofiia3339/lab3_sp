#ifndef AST_H
#define AST_H
#include <stdio.h>
typedef struct Node {
    char *kind, *text;
    int line;
    struct Node *a, *b, *c, *next;
} Node;
extern Node *root;
Node *node(const char *, const char *, int, Node *, Node *, Node *);
Node *append(Node *, Node *);
char *copy_text(const char *);
void free_tree(Node *);
void write_json(FILE *, Node *);
void validate(Node *);
long execute(Node *);
void generate_js(FILE *, Node *);
void fail(int, const char *);
#endif
