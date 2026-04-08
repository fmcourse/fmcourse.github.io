import subprocess
import sys
import re
import os

"""
Мониторит работу произвольных программ
и проверяет, открывают ли они успешно
файлы вне домашней директории.

Под открытием понимается системные вызовы
open(), openat().

Для наблюдения используется утилита strace.
"""

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <command> [args...]", file=sys.stderr)
        sys.exit(1)

    proc = subprocess.Popen(
        ["strace", "-f"] + sys.argv[1:],
        stderr=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1
    )

    # Читаем вывод strace
    for line in proc.stderr:
        line = line.rstrip()

        # Только успешные вызовы open/openat
        call, abs_path = is_open_or_openat(line)
        if not call:
            continue

        # Проверка: путь должен быть в HOME
        if not in_home(abs_path):
            print(f"[VIOLATION] {call} opened path outside HOME: {abs_path}", file=sys.stderr)

    proc.wait()
    sys.exit(proc.returncode)

# Регулярные выражения для парсинга open/openat
OPEN_RE = re.compile(
    r'^\s*(open|openat)\((.*?)\)\s+=\s+(\d+)\s*$'
)

def is_open_or_openat(line: str):

        m = OPEN_RE.match(line)
        if not m:
            return None, None

        call, args, result = m.group(1), m.group(2), int(m.group(3))

        if result < 0:
            return None, None

        path = extract_path(call, args)
        if not path:
            return None, None

        return call, os.path.abspath(path)

# Домашняя директория
HOME = os.path.expanduser("~")

def in_home(abs_path: str):
    return abs_path.startswith(HOME + "/") or abs_path == HOME


# Извлечение строки пути из аргументов open/openat
def extract_path(call: str, args: str) -> str | None:
    """
    Возвращает путь из аргументов системного вызова open/openat.
    Для open(...) путь — первый аргумент.
    Для openat(...) путь — второй аргумент.
    """
    # Разбиваем аргументы по запятым, но только на верхнем уровне
    parts = []
    depth = 0
    current = []
    for c in args:
        if c == ',' and depth == 0:
            parts.append(''.join(current).strip())
            current = []
            continue
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        current.append(c)
    if current:
        parts.append(''.join(current).strip())

    if call == "open":
        raw = parts[0]
    elif call == "openat":
        raw = parts[1]
    else:
        return None

    # raw обычно в кавычках
    m = re.match(r'"(.*)"', raw)
    return m.group(1) if m else None


if __name__ == "__main__":
    main()
