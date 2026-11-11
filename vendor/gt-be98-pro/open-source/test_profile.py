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


class KernelProfileTests(unittest.TestCase):
    def evaluate(self, enabled, model):
        # Evaluate the actual override block against conflicting baseline settings.
        text = (ROOT / 'release/src-rt/Makefile').read_text()
        block = text.split('# Source replacement experiment:', 1)[1]
        block = block[block.index('\n') + 1:block.index('EXTRA_KERNEL_YES_CONFIGS_1 :=')]
        source = ('EXTRA_KERNEL_CONFIGS=FAT_FS=n FUSE_FS=n KEEP=y\n' + block +
                  '\n$(info RESULT=$(EXTRA_KERNEL_CONFIGS))\n.PHONY: probe\nprobe:;@:\n')
        output = subprocess.check_output(
            ['make', '--no-print-directory', '-f', '-', 'probe',
             f'ASUS_OPEN_COMPONENTS={enabled}', f'BUILD_NAME={model}'], input=source, text=True)
        return next(line.removeprefix('RESULT=') for line in output.splitlines() if line.startswith('RESULT='))

    def test_required_modules_override_conflicting_baseline(self):
        tokens = self.evaluate('y', 'GT-BE98_PRO').split()
        self.assertIn('KEEP=y', tokens)
        for name in ('FAT', 'VFAT', 'HFS', 'HFSPLUS'):
            self.assertIn(f'{name}_FS=m', tokens)
        self.assertIn('FUSE_FS=y', tokens)
        self.assertNotIn('FAT_FS=n', tokens)
        self.assertNotIn('FUSE_FS=n', tokens)

    def test_baseline_and_other_models_unchanged(self):
        expected = 'FAT_FS=n FUSE_FS=n KEEP=y'
        self.assertEqual(self.evaluate('n', 'GT-BE98_PRO'), expected)
        self.assertEqual(self.evaluate('y', 'GT-BE98'), expected)


    def test_actual_kernel_config_writer(self):
        import tempfile
        text = (ROOT / 'release/src-rt/Makefile').read_text()
        start = text.index('# Source replacement experiment:')
        end = text.index('ifeq ($(NVRAM_SIZE),)', start)
        settings = text[start:end]
        start = text.index('define extraKernelConfig')
        end = text.index('endef', start) + len('endef')
        writer = text[start:end]
        with tempfile.TemporaryDirectory() as directory:
            config = Path(directory) / 'kernel.config'
            config.write_text('# CONFIG_FAT_FS is not set\n# CONFIG_HFSPLUS_FS is not set\n')
            source = ('EXTRA_KERNEL_CONFIGS=FAT_FS=n FUSE_FS=n\n' + settings + writer +
                      '\n.PHONY: probe\nprobe:\n\t$(call extraKernelConfig,' + str(config) + ')\n')
            subprocess.run(['make', '--no-print-directory', '-f', '-', 'probe',
                            'ASUS_OPEN_COMPONENTS=y', 'BUILD_NAME=GT-BE98_PRO'],
                           input=source, text=True, check=True)
            result = config.read_text().splitlines()
            for name in ('FAT', 'VFAT', 'HFS', 'HFSPLUS'):
                self.assertIn(f'CONFIG_{name}_FS=m', result)
            self.assertIn('CONFIG_FUSE_FS=y', result)
            self.assertFalse(any('CONFIG_CONFIG_' in line for line in result))
            self.assertFalse(any('not set' in line for line in result))
