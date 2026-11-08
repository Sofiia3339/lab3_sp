#include "ast.h"
#include <stdlib.h>
#include <string.h>
extern FILE *yyin;
int yyparse(void);
static FILE *output(const char *prefix, const char *suffix) {
    size_t size = strlen(prefix) + strlen(suffix) + 1;
    char *path = malloc(size); FILE *f;
    if (!path) fail(0, "Out of memory");
    snprintf(path, size, "%s%s", prefix, suffix); f = fopen(path, "w");
    if (!f) { perror(path); free(path); exit(1); }
    free(path); return f;
}
static void close_output(FILE *f) { if (fclose(f)) { perror("Writing output"); exit(1); } }
int main(int argc, char **argv) {
    long result; FILE *f;
    if (argc < 2 || argc > 3) { fprintf(stderr, "Usage: %s source.c [output-prefix]\n", argv[0]); return 1; }
    yyin = fopen(argv[1], "r"); if (!yyin) { perror(argv[1]); return 1; }
    if (yyparse()) { fclose(yyin); return 1; }
    fclose(yyin);
    validate(root);
    result = execute(root);
    printf("Syntax: OK\nSemantics: OK\nResult: %ld\n", result);
    if (argc == 3) {
        f = output(argv[2], ".ast.json"); write_json(f, root); fputc('\n', f); close_output(f);
        f = output(argv[2], ".js"); generate_js(f, root); close_output(f);
        f = output(argv[2], ".html");
        fputs("<!doctype html><html lang=\"uk\"><meta charset=\"utf-8\"><title>Generated JavaScript</title>"
              "<h1>Результат згенерованого JavaScript</h1><pre id=\"result\">Виконання…</pre>"
              "<script>window.onerror = function(message) { document.getElementById('result').textContent = message; };</script><script>\n", f);
        generate_js(f, root); fputs("</script></html>\n", f); close_output(f);
        printf("Created: %s.ast.json, %s.js, %s.html\n", argv[2], argv[2], argv[2]);
    }
    free_tree(root); return 0;
}
