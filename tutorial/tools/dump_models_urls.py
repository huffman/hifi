import gzip
import sys
import json

with gzip.open(sys.argv[1], 'rb') as f:
    entities = json.load(f)['Entities']

for entity in entities:
    for prop in ('modelURL', 'collisionURL'):
        if prop in entity and 'atp:/' not in entity[prop]:
            print(prop, entity[prop])

