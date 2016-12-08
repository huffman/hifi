import json
import sys

if len(sys.argv) < 3:
    print("usage: {} oldmap.json newmap.json".format(sys.argv[0]))
    sys.exit(1)

oldfile = sys.argv[1]
newfile = sys.argv[2]

with open(oldfile) as f:
    oldmap = json.load(f)

with open(newfile) as f:
    newmap = json.load(f)

added = (path for path in newmap if path not in oldmap)
deleted = (path for path in oldmap if path not in newmap)
modified = (path for path in newmap if path in oldmap and newmap[path] != oldmap[path])

for path in added:
    print('+ {}: {}'.format(path, newmap[path]))

for path in deleted:
    print('- {}: {}'.format(path, oldmap[path]))

for path in modified:
    print('* {}: {} -> {}'.format(path, oldmap[path], newmap[path]))

