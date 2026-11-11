"""Check deterministic public test transcripts against recorded reference results."""
import hashlib
import json
from pathlib import Path
import sys

expected = json.loads(Path(__file__).with_name('expected.json').read_text())
for name, row in expected.items():
    data = (Path(sys.argv[1]) / (name + '-arm.log')).read_bytes()
    if hashlib.sha256(data).hexdigest() != row['sha256']:
        raise SystemExit(name + ': result differs from reference fixture')
    if len(data.splitlines()) != row['records']:
        raise SystemExit(name + ': record count differs')
    print(name + ': native/ARM match; ' + str(row['records']) + ' reference records passed')
