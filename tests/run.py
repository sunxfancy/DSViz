#!/usr/bin/env python3
"""Run isolated cases, validate DOT with GraphViz, retain artifacts and JUnit."""
import argparse
from pathlib import Path
import subprocess
import sys
import time
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser()
parser.add_argument('--bin-dir', default='bin')
parser.add_argument('--dot', default='dot')
args = parser.parse_args()
bin_dir = Path(args.bin_dir).resolve()
artifacts = bin_dir / 'test-results'
artifacts.mkdir(parents=True, exist_ok=True)
root = ET.Element('testsuite', name='DSViz')
failures = 0

def run(name, command, graph=False, graph_path=None):
    global failures
    started = time.monotonic()
    case = ET.SubElement(root, 'testcase', name=name, classname='DSViz')
    output = ''
    try:
        proc = subprocess.run(command, capture_output=True, text=True, timeout=60)
        output = proc.stdout + proc.stderr
        if proc.returncode:
            raise RuntimeError(f'Exit {proc.returncode}\n{output}')
        if graph:
            path = graph_path or artifacts / (name + '.dot')
            if graph_path is None:
                path.write_text(proc.stdout, encoding='utf-8')
            svg = artifacts / (name + '.svg')
            parsed = subprocess.run([args.dot, '-Tsvg', str(path), '-o', str(svg)],
                                    capture_output=True, text=True, timeout=60)
            output += parsed.stderr
            if parsed.returncode:
                raise RuntimeError(f'GraphViz rejected DOT\n{parsed.stderr}')
            if not ET.parse(svg).getroot().tag.endswith('svg'):
                raise RuntimeError('GraphViz output is not SVG')
    except Exception as error:
        failures += 1
        output += '\n' + str(error)
        ET.SubElement(case, 'failure', message=str(error)).text = str(error)
        print(f'FAIL {name}: {error}', file=sys.stderr)
    else:
        print(f'PASS {name}')
    case.set('time', f'{time.monotonic() - started:.3f}')
    ET.SubElement(case, 'system-out').text = output
    (artifacts / (name + '.log')).write_text(output, encoding='utf-8')

run('basic', [str(bin_dir / 'basic')], graph=True)
run('bst', [str(bin_dir / 'bst')])
run('regression', [str(bin_dir / 'regression'), str(artifacts / 'regression.dot')],
    graph=True, graph_path=artifacts / 'regression.dot')
run('debugger-buffer', [str(bin_dir / 'debugger-test')])
run('header-multiple-translation-units', [str(bin_dir / 'header-test')])
for name in ['invalid', 'fields', 'children', 'plain', 'config', 'subgraphs', 'stress']:
    run('api-' + name, [str(bin_dir / 'api-test'), name], graph=True)
run('python-debugger', [sys.executable, str(Path(__file__).with_name('debugger_test.py')), '-v'])
root.set('tests', str(len(root)))
root.set('failures', str(failures))
ET.ElementTree(root).write(artifacts / 'junit.xml', encoding='utf-8', xml_declaration=True)
print(f'{len(root)} cases, {failures} failures; reports: {artifacts}')
sys.exit(bool(failures))
