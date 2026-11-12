from pathlib import Path
import hashlib,json,sys,io
from elftools.elf.elffile import ELFFile
repo,run=map(Path,sys.argv[1:])
def sha(data):return hashlib.sha256(data).hexdigest()
def allocated(data):
    if not data.startswith(b'\x7fELF'):return None
    elf=ELFFile(io.BytesIO(data))
    if elf.header['e_type']=='ET_REL':return None
    sections=[]
    for s in elf.iter_sections():
        if s['sh_flags']&2:
            sections.append((s.name,s['sh_type'],s['sh_flags'],s['sh_addr'],s['sh_size'],sha(s.data())))
    return sha(json.dumps([elf.header['e_machine'],elf.header['e_type'],sorted(sections)]).encode())
inputs=[];exact={};sections={}
for manifest in ['inputs.json','supplemental-inputs.json']:
    for entry in json.loads((repo/'vendor/gt-be98-pro'/manifest).read_text())['inputs']:
        path=repo/entry['destination'];data=path.read_bytes()
        row={'input':entry['destination'],'pinned_sha256':entry['sha256'],'current_matches_pin':sha(data)==entry['sha256'],'packaged_matches':[]}
        inputs.append(row)
        if not row['current_matches_pin']:continue
        exact.setdefault(sha(data),[]).append(row)
        key=allocated(data)
        if key:sections.setdefault(key,[]).append(row)
files=0;elfs=0
for path in sorted((run/'root').rglob('*')):
    if path.is_symlink() or not path.is_file():continue
    data=path.read_bytes();files+=1;elfs+=data.startswith(b'\x7fELF')
    matched=exact.get(sha(data),[]);kind='exact_file_sha256'
    if not matched:
        key=allocated(data);matched=sections.get(key,[]) if key else [];kind='allocated_elf_sections'
    for row in matched:row['packaged_matches'].append({'path':str(path.relative_to(run/'root')),'evidence':kind,'sha256':sha(data)})
report={'rootfs':str(run/'root'),'regular_files':files,'elf_files':elfs,'pinned_inputs':len(inputs),'matched_inputs':sum(bool(r['packaged_matches']) for r in inputs),'distinct_matched_paths':len({m['path'] for r in inputs for m in r['packaged_matches']}),'inputs':inputs,'limits':['Matching pinned prebuilt inputs does not establish licensing or absence of available source.','Unmatched inputs can still be linked into executables or modules; no absence claim follows.','Embedded firmware, boot components and inputs outside these partial manifests are not comprehensively covered.','Allocated-section matching allows differences in non-allocated ELF metadata; it is not a whole-file checksum match.']}
(run/'final-vendor-inventory.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k not in ('inputs','limits')},indent=2))
from collections import Counter
print(dict(Counter(m['evidence'] for r in inputs for m in r['packaged_matches'])))
