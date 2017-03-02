import json
import sys
import gzip

#print "Loading " + sys.argv[1]
with gzip.open(sys.argv[1]) as f:
    data = json.load(f)

print "Filename, Type, Total Size, Size"
form = "{}, {}, {}, {}"

events_by_filename = {}
for ev in data:
    name = ev['name']
    if name == 'fbx':
        events_by_filename[ev['args']['fbx_filename']] = ev

for ev in events_by_filename.itervalues():
    name = ev['name']
    if name == 'fbx':
        total_size = ev['args']['size']
        mesh_size = total_size
        for size in ev['args']['embedded_textures'].itervalues():
            mesh_size -= size

        print form.format(ev['args']['fbx_filename'], 'mesh', total_size / 1000000.0, mesh_size / 1000000.0)

        for path, size in ev['args']['embedded_textures'].iteritems():
            print form.format(path, 'texture', '', size / 1000000.0)
