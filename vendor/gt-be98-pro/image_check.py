"""Read-only FIT payload integrity checks, not signature or hardware validation."""
import hashlib
import struct

MAGIC = b'\xd0\x0d\xfe\xed'


def fit(data):
    def take(start, size):
        if start < 0 or size < 0 or start + size > len(data):
            raise ValueError('Truncated FIT data')
        return data[start:start + size]

    header = struct.unpack('>10I', take(0, 40))
    if header[0] != 0xd00dfeed or header[1] > len(data):
        raise ValueError('Invalid FIT header')
    total, pos, strings = header[1:4]
    structure_end = pos + header[9]
    strings_end = strings + header[8]
    if max(structure_end, strings_end) > total:
        raise ValueError('FIT blocks exceed total size')
    stack, props = [], {}
    ended = False
    while pos < structure_end:
        token = struct.unpack('>I', take(pos, 4))[0]
        pos += 4
        if token == 1:
            end = data.index(0, pos, structure_end)
            stack.append(data[pos:end].decode('ascii'))
            pos = (end + 4) & ~3
        elif token == 2:
            if not stack:
                raise ValueError('Unbalanced FIT node')
            stack.pop()
        elif token == 3:
            if not stack:
                raise ValueError('FIT property outside a node')
            length, offset = struct.unpack('>II', take(pos, 8))
            pos += 8
            if strings + offset >= strings_end or pos + length > structure_end:
                raise ValueError('FIT property out of bounds')
            end = data.index(0, strings + offset, strings_end)
            name = data[strings + offset:end].decode('ascii')
            node = props.setdefault('/'.join(stack), {})
            if name in node:
                raise ValueError('Duplicate FIT property')
            node[name] = take(pos, length)
            pos = (pos + length + 3) & ~3
        elif token == 4:
            pass
        elif token == 9:
            ended = True
            break
        else:
            raise ValueError('Unknown FIT token')
    if not ended or stack:
        raise ValueError('Incomplete FIT structure')
    images = {}
    for node, values in props.items():
        if not node.startswith('/images/') or node.count('/') != 2:
            continue
        if 'data' in values:
            payload = values['data']
        else:
            if 'data-position' in values:
                start = int.from_bytes(values['data-position'], 'big')
            else:
                start = ((total + 3) & ~3) + int.from_bytes(values['data-offset'], 'big')
            payload = take(start, int.from_bytes(values['data-size'], 'big'))
        hashes = [v for n, v in props.items() if n.startswith(node + '/hash')
                  and n.count('/') == 3 and v.get('algo', b'').rstrip(b'\0') == b'sha256']
        if not hashes or any(v.get('value') != hashlib.sha256(payload).digest() for v in hashes):
            raise ValueError('Missing or mismatched SHA256 payload hash: ' + node)
        images[node.rsplit('/', 1)[-1]] = payload
    if not images:
        raise ValueError('FIT has no images')
    return images


def inspect(data, depth=0):
    if depth > 4:
        raise ValueError('Excessively nested FIT')
    result = {}
    for name, payload in fit(data).items():
        row = {'bytes': len(payload), 'sha256': hashlib.sha256(payload).hexdigest()}
        # Device trees share FDT magic but are not nested firmware bundles.
        if name.startswith('bootfs') and payload.startswith(MAGIC):
            row['nested'] = inspect(payload, depth + 1)
        result[name] = row
    return result
