"""Перевірки синтаксису, семантики та відповідності JavaScript."""
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
node = shutil.which("node")
valid = [
    ("precedence", "return 2 + 3 * 4;", 14),
    ("parentheses", "return (2 + 3) * 4;", 20),
    ("division", "return -7 / 2;", -3),
    ("remainder", "return -7 % 2;", -1),
    ("logical", "return (2 && 3) + (0 || 5) + !0;", 3),
    ("short_and", "return 0 && (1 / 0);", 0),
    ("short_or", "return 1 || (1 / 0);", 1),
    ("comparisons", "return (1<2)+(2<=2)+(3>2)+(3>=3)+(1==1)+(1!=2);", 6),
    ("if_else", "int a=0; if (1) a=7; else a=9; return a;", 7),
    ("dangling_else", "int a=0; if (1) if (0) a=2; else a=3; return a;", 3),
    ("scope", "int a=7; {int a=20; a=a+1;} return a;", 7),
    ("return", "while(1) {return 6;} return 9;", 6),
    ("default_return", "int a=1;", 0),
    ("js_keyword", "int let=4; int read=3; return let+read;", 7),
    ("leading_zero", "return 00015;", 15),
    ("comments", "/* first\nsecond */ int a=4; // line\nreturn a;", 4),
    ("initialize_later", "int a; a=8; return a;", 8),
    ("loop_scope", "int i=0; int s=0; while(i<4){int x=i; s=s+x; i=i+1;} return s;", 6),
    ("integer_min", "return -2147483647 - 1;", -2147483648),
]
invalid = [
    ("semicolon", "int a=2 return a;", "Syntax error"),
    ("symbol", "return @;", "Lexical error"),
    ("undeclared", "return absent;", "Undeclared"),
    ("unreachable_name", "if(0){return absent;} return 0;", "Undeclared"),
    ("duplicate", "int a=1; int a=2; return a;", "Duplicate"),
    ("uninitialized", "int a; return a;", "uninitialized"),
    ("self_init", "int a=a; return a;", "uninitialized"),
    ("scope_escape", "{int a=1;} return a;", "Undeclared"),
    ("zero", "return 1/0;", "Division by zero"),
    ("overflow", "return 2147483647+1;", "overflow"),
    ("multiply_overflow", "return 2147483647*2147483647;", "overflow"),
    ("divide_overflow", "return (-2147483647-1)/-1;", "overflow"),
    ("literal", "return 2147483648;", "literal"),
    ("loop_limit", "while(1);", "limit"),
    ("comment", "/* unclosed", "Unterminated"),
    ("declaration_as_body", "if(1) int a=2; return 0;", "Syntax error"),
    ("trailing_input", "return 0;} int main(){return 1;", "Syntax error"),
]
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    for name, body, expected in valid + invalid:
        source = path / (name + ".c")
        source.write_text("int main(){\n" + body + "\n}\n")
        prefix = path / name
        p = subprocess.run([str(ROOT / "parser"), str(source), str(prefix)], capture_output=True, text=True, timeout=10)
        if isinstance(expected, int):
            assert p.returncode == 0, (name, p.stderr)
            actual = int(re.search(r"Result: (-?\d+)", p.stdout)[1])
            assert actual == expected, (name, actual, expected)
            tree = json.loads(prefix.with_suffix(".ast.json").read_text())
            assert tree["kind"] == "Program"
            if node:
                js = subprocess.run([node, str(prefix.with_suffix(".js"))], capture_output=True, text=True, timeout=10)
                assert js.returncode == 0 and js.stdout.strip() == f"Result: {expected}", (name, js.stdout, js.stderr)
        else:
            assert p.returncode != 0 and expected in p.stderr, (name, p.stdout, p.stderr)
        print("PASS", name)
print(f"Passed {len(valid) + len(invalid)} checks; JavaScript comparison: {'enabled' if node else 'skipped (Node.js not installed)' }.")
