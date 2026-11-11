const fs = require('fs'), vm = require('vm'), assert = require('assert');
const source = fs.readFileSync(process.argv[2], 'utf8');
const start = source.indexOf('function set_disk_info');
const begin = source.indexOf('var usb_fatfs_mod', start);
const end = source.indexOf('var selected_disk_system', begin);
assert(begin > start && end > begin);
const block = source.slice(begin, end);
function check(capability, modes, expected) {
  const values = {rc_support: capability, usb_fatfs_mod: modes[0], usb_ntfs_mod: modes[1], usb_hfs_mod: modes[2]};
  const code = block.replace(/<% nvram_get\("([^"]+)"\); %>/g, (_, name) => values[name]);
  const options = new Set(['tfat', 'tntfs', 'thfsplus']);
  vm.runInNewContext(code, {$: selector => ({remove() {
    const match = selector.match(/value='([^']+)'/);assert(match);options.delete(match[1]);
  }})});
  assert.deepStrictEqual([...options].sort(), expected.sort());
}
check('', ['open','open','open'], []);
check('usbX2 open_fs_format', ['open','open','open'], ['tfat','tntfs','thfsplus']);
check('', ['tuxera','tuxera','tuxera'], ['tfat','tntfs','thfsplus']);
check('not_open_fs_format', ['open','open','open'], []);
check('open_fs_format', ['open','','paragon'], ['tfat']);
console.log('UI format capability selection passed');
