#!/usr/bin/python

import os
import json

from jinja2 import Template, Environment, PackageLoader
from apigen.filters import apigen_filters
from apigen.filters import normalize_field_name

def run(bfrt_path='bf-rt.json'):
    env = Environment(loader=PackageLoader('apigen', 'templates'), trim_blocks=True, lstrip_blocks=True)
    env.filters.update(apigen_filters)
    src_tmpl = env.get_template('source.jinja')
    header_tmpl = env.get_template('header.jinja')

    with open(bfrt_path, 'r') as f:
        bfrt = json.load(f)

    header_files = []
    for table in bfrt['tables']:
        prefix = 'bf_api'
        filename = normalize_field_name(table['name'])
        filename = prefix + '_' + filename.lower()

        if '_snapshot_' in filename:
            continue

        result = src_tmpl.render(table=table, prefix = prefix)
        with open(filename + '.c', 'w+') as f:
            f.write(result)

        result = header_tmpl.render(table=table, prefix = prefix) 
        with open(os.path.join('include', filename + '.h'), 'w+') as f:
            f.write(result)
            header_files.append(os.path.basename(f.name))


    # Generate netp4api.h 
    t = env.get_template('netp4api.h.jinja')
    r = t.render(header_files = header_files, prefix = prefix)
    with open(os.path.join('include', prefix + '.h'), 'w+') as f:
        f.write(r)

    print('Done!')
    
if __name__ == '__main__':
    with open('bf-rt.json') as f:
        bfrt = json.load(f)

    with open('templates/c.tmpl') as f:
        src_tmpl = Template(f.read())

    for table in bfrt['tables']:
        src_filename = table['name'].replace('.', '_') + '.c'
        src_filename = src_filename.replace('$', '').lower()
        result = src_tmpl.render(table=table)
        with open(src_filename, 'w+') as f:
            f.write(result)
