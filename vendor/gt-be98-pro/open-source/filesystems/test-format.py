#!/usr/bin/env python3
"""Test actual format function with mocked disk operations, plus UI selection."""
import os
from pathlib import Path
import subprocess
import tempfile
here = Path(__file__).resolve().parent
root = here.parents[3]
source = (root / 'release/src/router/rc/usb.c').read_text(errors='surrogateescape')
begin = source.index('static void start_diskformat(')
end = source.index('// -1: manully scan', begin)
code = (here / 'format-stubs.c').read_text().replace('/* FUNCTION_INSERT */', source[begin:end])
with tempfile.TemporaryDirectory() as tmp:
    Path(tmp, 'test.c').write_text(code, errors='surrogateescape')
    subprocess.run(['docker', 'run', '--rm', '--network', 'none', '--user', f'{os.getuid()}:{os.getgid()}',
                    '-v', f'{tmp}:/tests:ro', 'amng-build', 'bash', '-c',
                    'set -eu; gcc -std=c99 -Wall -Wextra -Werror -Wno-unused-value -Wno-sign-compare '
                    '-fsanitize=address,undefined -DASUS_OPEN_COMPONENTS /tests/test.c -o /tmp/test; /tmp/test'], check=True)
subprocess.run(['node', str(here / 'test-ui.js'), str(root / 'release/src/router/www/device-map/disk_format.asp')], check=True)
