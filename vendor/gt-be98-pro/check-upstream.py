#!/usr/bin/env python3
"""Read upstream tags without modifying the checkout or staging vendor inputs."""
import json
from pathlib import Path
import re
import subprocess

UPSTREAM = 'https://github.com/RMerl/asuswrt-merlin.ng.git'


def stable_tags(output):
    tags = {}
    for line in output.splitlines():
        sha, ref = line.split()
        match = re.fullmatch(r'refs/tags/(3006\.102\.(\d+)(?:_(\d+))?)(\^\{\})?', ref)
        if match:
            name = match[1]
            # Annotated tags resolve to the peeled commit, not the tag object.
            if name not in tags or match[4]:
                tags[name] = (int(match[2]), int(match[3] or 0), sha)
    return tags


def main():
    manifest = json.loads((Path(__file__).with_name('inputs.json')).read_text())
    result = subprocess.run(
        ['git', 'ls-remote', '--tags', UPSTREAM, '3006.102.*'],
        check=True, text=True, capture_output=True, timeout=120)
    tags = stable_tags(result.stdout)
    if not tags:
        raise SystemExit('No stable 3006.102 tags returned; do not change baseline')
    name = max(tags, key=lambda n: tags[n][:2])
    commit = tags[name][2]
    print(json.dumps({
        'upstream': UPSTREAM,
        'latest_stable_tag': name,
        'latest_stable_commit': commit,
        'recorded_base': manifest['merlin_base'],
        'rebase_needed': commit != manifest['merlin_base'],
        'note': 'Read-only check; rebase/build/audit/hardware validation remain separate.',
    }, indent=2))


if __name__ == '__main__':
    main()
