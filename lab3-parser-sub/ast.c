#include "ast.h"
#include <stdlib.h>
#include <string.h>
void fail(int line, const char *message) {
    fprintf(stderr, "Error, line %d: %s\n", line, message);
    exit(1);
}
char *copy_text(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (!p) fail(0, "Out of memory");
    strcpy(p, s); return p;
}
Node *node(const char *kind, const char *text, int line, Node *a, Node *b, Node *c) {
    Node *n = calloc(1, sizeof(*n));
    if (!n) fail(line, "Out of memory");
    n->kind = copy_text(kind); n->text = copy_text(text); n->line = line;
    n->a = a; n->b = b; n->c = c; return n;
}
Node *append(Node *list, Node *n) {
    Node *tail = list;
    if (!list) return n;
    while (tail->next) tail = tail->next;
    tail->next = n; return list;
}
void free_tree(Node *n) {
    if (!n) return;
    free_tree(n->a); free_tree(n->b); free_tree(n->c); free_tree(n->next);
    free(n->kind); free(n->text); free(n);
}
static void quoted(FILE *out, const char *s) {
    fputc('"', out);
    for (; *s; ++s) {
        if (*s == '"' || *s == '\\') fputc('\\', out);
        fputc(*s, out);
    }
    fputc('"', out);
}
void write_json(FILE *out, Node *n) {
    Node *children[3]; int i, comma = 0;
    fprintf(out, "{\"kind\":"); quoted(out, n->kind);
    fprintf(out, ",\"text\":"); quoted(out, n->text);
    fprintf(out, ",\"line\":%d,\"children\":[", n->line);
    children[0] = n->a; children[1] = n->b; children[2] = n->c;
    for (i = 0; i < 3; ++i) {
        Node *p;
        for (p = children[i]; p; p = p->next) {
            if (comma++) fputc(',', out);
            write_json(out, p);
        }
    }
    fputs("]}", out);
}
