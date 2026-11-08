#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define MIN_INT (-2147483647L - 1)
#define MAX_INT 2147483647L
typedef struct Symbol {
    const char *name;
    long value;
    int depth, initialized;
    struct Symbol *next;
} Symbol;
static Symbol *symbols;
static int depth, returned;
static long result, steps;
static int is(Node *n, const char *kind) { return strcmp(n->kind, kind) == 0; }
static Symbol *find(Node *n) {
    Symbol *s;
    for (s = symbols; s; s = s->next) if (!strcmp(s->name, n->text)) return s;
    fail(n->line, "Undeclared variable"); return 0;
}
static Symbol *declare(Node *n) {
    Symbol *s;
    for (s = symbols; s && s->depth == depth; s = s->next)
        if (!strcmp(s->name, n->text)) fail(n->line, "Duplicate declaration in the same scope");
    s = calloc(1, sizeof(*s)); if (!s) fail(n->line, "Out of memory");
    s->name = n->text; s->depth = depth; s->next = symbols; symbols = s; return s;
}
static void leave(void) {
    while (symbols && symbols->depth == depth) {
        Symbol *s = symbols; symbols = s->next; free(s);
    }
    --depth;
}
static long checked(Node *n, long long x) {
    if (x < MIN_INT || x > MAX_INT) fail(n->line, "32-bit integer overflow");
    return (long)x;
}
static long number(Node *n) {
    char *end; long long x;
    errno = 0; x = strtoll(n->text, &end, 10);
    if (errno || *end || x > MAX_INT) fail(n->line, "Integer literal outside 0..2147483647");
    return (long)x;
}
static void check(Node *n) {
    Node *p;
    if (!n) return;
    if (is(n, "Block")) {
        ++depth; for (p = n->a; p; p = p->next) check(p); leave(); return;
    }
    if (is(n, "Declare")) { declare(n); check(n->a); return; }
    if (is(n, "Assign") || is(n, "Variable")) find(n);
    if (is(n, "Number")) number(n);
    check(n->a); check(n->b); check(n->c);
}
void validate(Node *n) { depth = 0; check(n); }
static void tick(Node *n) {
    if (++steps > 1000000) fail(n->line, "Execution limit exceeded (1000000 steps)");
}
static long eval(Node *n) {
    long a, b; const char *op = n->text;
    tick(n);
    if (is(n, "Number")) return number(n);
    if (is(n, "Variable")) {
        Symbol *s = find(n);
        if (!s->initialized) fail(n->line, "Reading an uninitialized variable");
        return s->value;
    }
    a = eval(n->a);
    if (is(n, "Unary")) return !strcmp(op, "!") ? !a : checked(n, -(long long)a);
    if (!strcmp(op, "&&")) return a ? !!eval(n->b) : 0;
    if (!strcmp(op, "||")) return a ? 1 : !!eval(n->b);
    b = eval(n->b);
    if (!strcmp(op, "+")) return checked(n, (long long)a + b);
    if (!strcmp(op, "-")) return checked(n, (long long)a - b);
    if (!strcmp(op, "*")) return checked(n, (long long)a * b);
    if (!strcmp(op, "/") || !strcmp(op, "%")) {
        if (!b) fail(n->line, "Division by zero");
        if (a == MIN_INT && b == -1) fail(n->line, "32-bit integer overflow");
        return !strcmp(op, "/") ? a / b : a % b;
    }
    if (!strcmp(op, "==")) return a == b;
    if (!strcmp(op, "!=")) return a != b;
    if (!strcmp(op, "<")) return a < b;
    if (!strcmp(op, ">")) return a > b;
    if (!strcmp(op, "<=")) return a <= b;
    if (!strcmp(op, ">=")) return a >= b;
    fail(n->line, "Unknown operator"); return 0;
}
static void run(Node *n) {
    Node *p;
    if (!n || returned) return;
    tick(n);
    if (is(n, "Program")) run(n->a);
    else if (is(n, "Block")) {
        ++depth;
        for (p = n->a; p && !returned; p = p->next) run(p);
        leave();
    } else if (is(n, "Declare")) {
        Symbol *s = declare(n);
        if (n->a) { s->value = eval(n->a); s->initialized = 1; }
    } else if (is(n, "Assign")) {
        Symbol *s = find(n); s->value = eval(n->a); s->initialized = 1;
    } else if (is(n, "Return")) { result = eval(n->a); returned = 1; }
    else if (is(n, "If")) { if (eval(n->a)) run(n->b); else run(n->c); }
    else if (is(n, "While")) { while (!returned && eval(n->a)) run(n->b); }
}
long execute(Node *n) {
    returned = 0; result = 0; steps = 0; depth = 0;
    run(n); return result;
}
