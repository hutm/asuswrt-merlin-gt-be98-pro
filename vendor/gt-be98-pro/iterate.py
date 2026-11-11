#!/usr/bin/env python3
"""Local GT-BE98 Pro build iterations. Never connects to or flashes a router."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

# Allow both direct invocation and import-based local tests.
sys.path.insert(0, str(Path(__file__).resolve().parent))
from image_check import inspect

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
BASE = '69838e4a60564e5550e97f11d8a8cf2c6260c4f0'
IMAGE_DIR = ROOT / 'release/src-rt-5.04behnd.4916/image'


def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(data)
    return h.hexdigest()


def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


def save(path, data):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(data, indent=2) + '\n')
    temporary.replace(path)


def preflight(kit):
    run(sys.executable, str(HERE / 'provenance.py'), 'verify')
    run('git', '-C', str(ROOT), 'merge-base', '--is-ancestor', BASE, 'HEAD')
    run('docker', 'info', stdout=subprocess.DEVNULL)
    for name in ('Dockerfile', 'scripts/20-stage-toolchain.sh',
                 'scripts/30-build-firmware.sh', 'openssl-3.5-static-atomic.patch',
                 'buildroot-2021.02.4/output/host/bin/aarch64-buildroot-linux-gnu-gcc',
                 'buildroot-2021.02.4/output-arm/host/bin/arm-buildroot-linux-gnueabi-gcc'):
        if not (kit / name).is_file():
            raise ValueError(f'Missing local build prerequisite: {kit / name}')
    run('bash', '-n', str(HERE / 'build.sh'))
    print('Preflight passed. Provenance coverage remains partial; no flash approval.', flush=True)


def collect_images(directory, output, started_ns):
    images = sorted(directory.glob('GT-BE98_PRO_*_nand_squashfs.pkgtb'))
    if len(images) != 1:
        raise ValueError('Expected exactly one normal GT-BE98_PRO image; inspect stale versions')
    source = images[0]
    if source.is_symlink() or source.stat().st_mtime_ns < started_ns or source.stat().st_size == 0:
        raise ValueError('Image is stale, empty, or a symlink; refusing to record build success')
    dest = output / source.name
    shutil.copyfile(source, dest)
    if sha(source) != sha(dest):
        raise ValueError('Image changed while being copied')
    # Loader images deliberately remain in the build tree and are never candidates.
    return {dest.name: {'bytes': dest.stat().st_size, 'sha256': sha(dest)}}


def build(kit, runs):
    runs.mkdir(parents=True, exist_ok=True)
    # Lock by checkout, independent of --runs path. Advisory: use this entry point.
    git_dir = Path(subprocess.check_output(
        ['git', '-C', str(ROOT), 'rev-parse', '--absolute-git-dir'], text=True).strip())
    with (git_dir / 'asus-iteration.lock').open('w') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise ValueError('An iteration is already running for this checkout')
        preflight(kit)
        stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ-')
        output = Path(tempfile.mkdtemp(prefix=stamp, dir=runs))
        print(f'Iteration evidence: {output}', flush=True)
        record = {'model': 'GT-BE98_PRO', 'state': 'building', 'installable': False,
                  'provenance_coverage': 'partial', 'hardware_tests': 'not performed',
                  'git_head': subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
                  'checks_not_automated': ['complete input provenance', 'userspace and kernel ABI checks',
                                           'flash partition review', 'hardware validation'],
                  'images': {}}
        # Record tracked changes without dumping binary blobs. Build trees are dirty;
        # this is an evidence record, not a claim of reproducible source coverage.
        record['tracked_changes'] = subprocess.check_output(
            ['git', '-C', str(ROOT), 'diff', '--name-only', 'HEAD'], text=True).splitlines()
        for name in ('inputs.json', 'supplemental-inputs.json', 'release-status.json'):
            shutil.copyfile(HERE / name, output / name)
        record['kit_files'] = {name: sha(kit / name) for name in (
            'Dockerfile', 'scripts/20-stage-toolchain.sh', 'scripts/30-build-firmware.sh',
            'openssl-3.5-static-atomic.patch')}
        # A separate launcher avoids the edit-during-execution failure from iteration 4.
        launcher = output / 'launcher.sh'
        shutil.copyfile(HERE / 'build.sh', launcher)
        record['launcher_sha256'] = sha(launcher)
        env = dict(os.environ, ASUS_BUILD_KIT=str(kit), ASUS_SOURCE_DIR=str(ROOT),
                   ASUS_BUILD_LOG=str(output / 'build.log'))
        started = time.time_ns()
        save(output / 'result.json', record)
        try:
            run('bash', str(launcher), env=env)
            run(sys.executable, str(HERE / 'provenance.py'), 'verify')
            record['images'] = collect_images(IMAGE_DIR, output, started)
            for name in record['images']:
                save(output / (name + '.fit.json'), inspect((output / name).read_bytes()))
            record['fit_payload_hashes'] = 'passed'
            record['state'] = 'built-experimental'
            (output / 'SHA256SUMS').write_text(''.join(
                f"{row['sha256']}  {name}\n" for name, row in record['images'].items()))
        except BaseException as exc:
            record['state'] = 'failed'
            record['error'] = str(exc)
            raise
        finally:
            record['finished_utc'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
            save(output / 'result.json', record)
        print(f'Build archived: {output}\nExperimental only; hardware and complete provenance gates remain closed.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('check', 'build', 'status', 'release-check'))
    parser.add_argument('--kit', type=Path, default=ROOT.parent / 'asuswrt-build')
    parser.add_argument('--runs', type=Path)
    args = parser.parse_args()
    kit = args.kit.resolve()
    try:
        if args.command == 'check':
            preflight(kit)
        elif args.command == 'build':
            build(kit, (args.runs or kit / 'iterations').resolve())
        elif args.command == 'release-check':
            run(sys.executable, str(HERE / 'provenance.py'), 'release-check')
        else:
            runs = args.runs or kit / 'iterations'
            results = sorted(runs.glob('*/result.json'))
            if results:
                record = json.loads(results[-1].read_text())
                print(json.dumps({key: value for key, value in record.items()
                                  if key not in ('tracked_changes', 'kit_files')}, indent=2))
                print(f'Evidence: {results[-1].parent}')
            else:
                print('No iterations recorded by this runner. Historical result:')
                print((HERE / 'build-result.json').read_text())
        return 0
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
