#include "ast.h"
#include <string.h>
#include <stdlib.h>
static int is(Node *n, const char *kind) { return !strcmp(n->kind, kind); }
static void expr(FILE *f, Node *n) {
    if (is(n, "Number")) { fprintf(f, "%ld", strtol(n->text, 0, 10)); return; }
    if (is(n, "Variable")) { fprintf(f, "read(v_%s, %d)", n->text, n->line); return; }
    if (is(n, "Unary")) {
        fprintf(f, "unary(\"%s\", ", n->text); expr(f, n->a); fprintf(f, ", %d)", n->line); return;
    }
    if (!strcmp(n->text, "&&") || !strcmp(n->text, "||")) {
        fputs("Number(Boolean(", f); expr(f, n->a);
        fprintf(f, ") %s Boolean(", n->text); expr(f, n->b); fputs("))", f); return;
    }
    fprintf(f, "binary(\"%s\", ", n->text); expr(f, n->a); fputs(", ", f);
    expr(f, n->b); fprintf(f, ", %d)", n->line);
}
static void indent(FILE *f, int d) { while (d--) fputs("    ", f); }
static void stmt(FILE *f, Node *n, int d) {
    Node *p;
    if (is(n, "Program")) { stmt(f, n->a, d); return; }
    indent(f, d);
    if (is(n, "Block")) {
        fputs("{\n", f);
        for (p = n->a; p; p = p->next) stmt(f, p, d + 1);
        indent(f, d); fputs("}\n", f); return;
    }
    fprintf(f, "tick(%d);\n", n->line); indent(f, d);
    if (is(n, "Declare")) {
        fprintf(f, "let v_%s;\n", n->text);
        if (n->a) { indent(f, d); fprintf(f, "v_%s = ", n->text); expr(f, n->a); fputs(";\n", f); }
    } else if (is(n, "Assign")) {
        fprintf(f, "v_%s = ", n->text); expr(f, n->a); fputs(";\n", f);
    } else if (is(n, "Return")) { fputs("return ", f); expr(f, n->a); fputs(";\n", f); }
    else if (is(n, "If")) {
        fputs("if (", f); expr(f, n->a); fputs(") {\n", f); stmt(f, n->b, d + 1);
        indent(f, d); fputs("}", f);
        if (n->c) { fputs(" else {\n", f); stmt(f, n->c, d + 1); indent(f, d); fputs("}", f); }
        fputc('\n', f);
    } else if (is(n, "While")) {
        fputs("while (", f); expr(f, n->a); fputs(") {\n", f);
        indent(f, d + 1); fprintf(f, "tick(%d);\n", n->line);
        stmt(f, n->b, d + 1); indent(f, d); fputs("}\n", f);
    } else fputs(";\n", f);
}
void generate_js(FILE *f, Node *n) {
    fputs("\"use strict\";\nlet steps = 0;\n"
    "function error(line, message) { throw new Error('Line ' + line + ': ' + message); }\n"
    "function tick(line) { if (++steps > 1000000) error(line, 'Execution limit exceeded'); }\n"
    "function checked(x, line) { if (x < -2147483648n || x > 2147483647n) error(line, '32-bit integer overflow'); return Number(x); }\n"
    "function read(x, line) { tick(line); if (x === undefined) error(line, 'Reading an uninitialized variable'); return x; }\n"
    "function unary(op, a, line) { tick(line); return op === '!' ? Number(!a) : checked(-BigInt(a), line); }\n"
    "function binary(op, a, b, line) {\n"
    "  tick(line);\n"
    "  switch(op) {\n"
    "    case '+': return checked(BigInt(a) + BigInt(b), line);\n"
    "    case '-': return checked(BigInt(a) - BigInt(b), line);\n"
    "    case '*': return checked(BigInt(a) * BigInt(b), line);\n"
    "    case '/': case '%':\n"
    "      if (!b) error(line, 'Division by zero');\n"
    "      if (a === -2147483648 && b === -1) error(line, '32-bit integer overflow');\n"
    "      return checked(op === '/' ? BigInt(a) / BigInt(b) : BigInt(a) % BigInt(b), line);\n"
    "    case '<': return Number(a < b); case '>': return Number(a > b);\n"
    "    case '<=': return Number(a <= b); case '>=': return Number(a >= b);\n"
    "    case '==': return Number(a === b); case '!=': return Number(a !== b);\n"
    "  }\n"
    "  error(line, 'Unknown operator');\n"
    "}\nfunction main() {\n", f);
    stmt(f, n, 1);
    fputs("  return 0;\n}\nconst result = main();\nconsole.log('Result: ' + result);\n"
          "if (typeof document !== 'undefined') document.getElementById('result').textContent = 'Result: ' + result;\n", f);
}
