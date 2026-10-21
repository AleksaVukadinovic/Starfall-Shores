# Generates the placeholder sea life models (kelp, fish) as glTF binaries with an embedded texture.
# Run from the repository root: python3 tools/make_sea_life_models.py
import json, math, struct, zlib, os

def png(width, height, pixel):
    rows = b''.join(b'\x00' + b''.join(bytes(pixel(x, y)) for x in range(width)) for y in range(height))
    def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(rows, 9)) + chunk(b'IEND', b''))

def normalize(v):
    l = math.sqrt(sum(c * c for c in v)) or 1.0
    return [c / l for c in v]

def smooth_normals(positions, indices):
    normals = [[0.0, 0.0, 0.0] for _ in positions]
    for i in range(0, len(indices), 3):
        a, b, c = (positions[indices[i + k]] for k in range(3))
        u = [b[k] - a[k] for k in range(3)]; v = [c[k] - a[k] for k in range(3)]
        n = [u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0]]
        for k in range(3):
            for j in range(3): normals[indices[i + k]][j] += n[j]
    return [normalize(n) for n in normals]

def write_glb(path, positions, normals, uvs, indices, image):
    def pad(b, fill=b'\x00'): return b + fill * ((4 - len(b) % 4) % 4)
    blobs = [struct.pack('<%df' % (3 * len(positions)), *sum(positions, [])),
             struct.pack('<%df' % (3 * len(normals)), *sum(normals, [])),
             struct.pack('<%df' % (2 * len(uvs)), *sum(uvs, [])),
             struct.pack('<%dI' % len(indices), *indices),
             image]
    binary, views = b'', []
    for blob in blobs:
        views.append({'buffer': 0, 'byteOffset': len(binary), 'byteLength': len(blob)})
        binary += pad(blob)
    for view, target in zip(views[:4], [34962, 34962, 34962, 34963]): view['target'] = target
    count = len(positions)
    gltf = {
        'asset': {'version': '2.0', 'generator': 'Starfall Shores make_models.py'},
        'scene': 0, 'scenes': [{'nodes': [0]}], 'nodes': [{'mesh': 0}],
        'meshes': [{'primitives': [{'attributes': {'POSITION': 0, 'NORMAL': 1, 'TEXCOORD_0': 2}, 'indices': 3, 'material': 0}]}],
        'accessors': [
            {'bufferView': 0, 'componentType': 5126, 'count': count, 'type': 'VEC3',
             'min': [min(p[k] for p in positions) for k in range(3)], 'max': [max(p[k] for p in positions) for k in range(3)]},
            {'bufferView': 1, 'componentType': 5126, 'count': count, 'type': 'VEC3'},
            {'bufferView': 2, 'componentType': 5126, 'count': count, 'type': 'VEC2'},
            {'bufferView': 3, 'componentType': 5125, 'count': len(indices), 'type': 'SCALAR'}],
        'materials': [{'pbrMetallicRoughness': {'baseColorTexture': {'index': 0}, 'metallicFactor': 0.0, 'roughnessFactor': 0.8},
                       'doubleSided': True}],
        'textures': [{'source': 0, 'sampler': 0}], 'samplers': [{}],
        'images': [{'bufferView': 4, 'mimeType': 'image/png'}],
        'bufferViews': views, 'buffers': [{'byteLength': len(binary)}]}
    js = pad(json.dumps(gltf, separators=(',', ':')).encode(), b' ')
    glb = struct.pack('<III', 0x46546C67, 2, 12 + 8 + len(js) + 8 + len(binary))
    glb += struct.pack('<II', len(js), 0x4E4F534A) + js + struct.pack('<II', len(binary), 0x004E4942) + binary
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, 'wb').write(glb)
    print(path, count, 'vertices', len(indices) // 3, 'triangles', len(glb), 'bytes')

def kelp():
    # Three twisting, tapering ribbons crossing at 60 degrees, 1 unit tall; the instance scale sets the real height.
    positions, uvs, indices = [], [], []
    segments = 12
    for ribbon in range(3):
        base_angle = ribbon * math.pi / 3
        first = len(positions)
        for s in range(segments + 1):
            y = s / segments
            angle = base_angle + y * 1.4
            half_width = 0.09 * (1.0 - 0.6 * y) + 0.02
            dx, dz = math.cos(angle) * half_width, math.sin(angle) * half_width
            positions += [[-dx, y, -dz], [dx, y, dz]]
            uvs += [[0.0, y], [1.0, y]]
        for s in range(segments):
            a = first + 2 * s
            indices += [a, a + 1, a + 2, a + 1, a + 3, a + 2]
    normals = smooth_normals(positions, indices)
    def pixel(x, y):
        t = 1.0 - y / 127.0          # image top is the tip (UVs are flipped on load)
        edge = abs(x / 31.0 - 0.5) * 2.0
        r, g, b = 50 + 90 * t, 62 + 95 * t, 22 + 30 * t
        stripe = 0.75 if edge < 0.15 else 1.0
        light = 1.0 + 0.25 * edge
        return [min(255, int(c * stripe * light)) for c in (r, g, b)]
    write_glb('app/resources/models/kelp/kelp.glb', positions, normals, uvs, indices, png(32, 128, pixel))

def fish():
    # A slender fish, 1 unit long along z, head towards +z; the flock scale sets its real size.
    positions, uvs, indices = [], [], []
    rings, around = 12, 12
    for r in range(rings + 1):
        t = r / rings                  # 0 at the tail end, 1 at the nose
        z = t - 0.5
        height = 0.2 * math.sin(math.pi * min(t * 1.08, 1.0)) ** 0.7 * (0.45 + 0.55 * t) + 0.01
        width = 0.42 * height
        for a in range(around + 1):
            theta = 2 * math.pi * a / around   # 0 on top
            positions.append([width * math.sin(theta), height * math.cos(theta), z])
            uvs.append([a / around, t])
    for r in range(rings):
        for a in range(around):
            i = r * (around + 1) + a
            j = i + around + 1
            indices += [i, j, i + 1, i + 1, j, j + 1]
    def fin(points, u):
        first = len(positions)
        for p in points:
            positions.append(p); uvs.append([u, (p[2] + 0.5)])
        indices.extend(first + k for k in range(len(points)))
    # Tail fin (two lobes) and dorsal fin; thin, two-sided triangles.
    fin([[0, 0, -0.46], [0, 0.2, -0.72], [0, 0, -0.6]], 0.02)
    fin([[0, 0, -0.46], [0, 0, -0.6], [0, -0.2, -0.72]], 0.02)
    fin([[0, 0.15, 0.12], [0, 0.27, -0.05], [0, 0.14, -0.15]], 0.02)
    normals = smooth_normals(positions, indices)
    def pixel(x, y):
        u, v = x / 127.0, 1.0 - y / 63.0      # v: 0 tail, 1 nose
        side = math.cos(2 * math.pi * u)      # 1 on top, -1 on the belly
        top, flank, belly = (28, 58, 108), (150, 182, 205), (232, 236, 240)
        if side > 0:
            color = [f + (t - f) * side for t, f in zip(top, flank)]
        else:
            color = [f + (b - f) * -side for b, f in zip(belly, flank)]
        if abs(side) < 0.18:                  # a golden stripe along each flank
            color = [c * 0.6 + s * 0.4 for c, s in zip(color, (230, 190, 60))]
        for eye_u in (0.25, 0.75):            # eyes near the nose
            if (u - eye_u) ** 2 * 4 + (v - 0.86) ** 2 < 0.0016:
                color = [10, 10, 15]
        return [int(c) for c in color]
    write_glb('app/resources/models/fish/fish.glb', positions, normals, uvs, indices, png(128, 64, pixel))

kelp()
fish()
