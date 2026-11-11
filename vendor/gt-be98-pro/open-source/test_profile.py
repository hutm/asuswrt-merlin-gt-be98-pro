from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[3]


def profile(enabled, model):
    source = ('include release/src-rt/target.mak\n'
              f'$(info RESULT=$(filter TFAT=% NTFS=% HFS=%,$({model})))\n'
              '.PHONY: probe\nprobe:;@:\n')
    output = subprocess.check_output(
        ['make', '--no-print-directory', '-f', '-', 'probe', f'ASUS_OPEN_COMPONENTS={enabled}'],
        input=source, text=True, cwd=ROOT)
    return next(line.removeprefix('RESULT=') for line in output.splitlines() if line.startswith('RESULT='))


class ProfileTests(unittest.TestCase):
    def test_opt_in_selects_only_open_filesystems(self):
        self.assertEqual(profile('y', 'GT-BE98_PRO'), 'TFAT=n NTFS="open" HFS="open"')

    def test_default_retains_baseline(self):
        self.assertEqual(profile('n', 'GT-BE98_PRO'), 'TFAT=y NTFS="tuxera" HFS="tuxera"')

    def test_other_model_unchanged(self):
        self.assertEqual(profile('y', 'GT-BE98'), profile('n', 'GT-BE98'))
