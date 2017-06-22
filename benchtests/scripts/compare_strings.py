#!/usr/bin/python
# Copyright (C) 2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.
"""Compare results of string functions

Given a string benchmark result file, print a table with comparisons with a
baseline.  The baseline is the first function, which typically is the builtin
function.
"""
import sys
import os
import json
import pylab

try:
    import jsonschema as validator
except ImportError:
    print('Could not find jsonschema module.')
    raise


def parse_file(filename, schema_filename):
    with open(schema_filename, 'r') as schemafile:
        schema = json.load(schemafile)
        with open(filename, 'r') as benchfile:
            bench = json.load(benchfile)
            validator.validate(bench, schema)
            return bench


def draw_graph(f, v, ifuncs, results):
    """Plot graphs for functions

    Plot line graphs for each of the ifuncs

    Args:
        f: Function name
        v: Benchmark variant for the function.
        ifuncs: List of ifunc names
        results: Dictionary of results for each test criterion
    """
    xkeys = results.keys()

    pylab.clf()
    fig = pylab.figure(frameon=False)
    fig.set_size_inches(32, 18)
    pylab.ylabel('Performance improvement from base')
    X = range(len(xkeys))
    pylab.xticks(X, xkeys)

    i = 0

    while i < len(ifuncs):
        Y = [results[k][i] for k in xkeys]
        lines = pylab.plot(X, Y, label=':'+ifuncs[i])
        i = i + 1

    pylab.legend()
    pylab.grid()
    pylab.savefig('%s-%s.png' % (f, v), bbox_inches='tight')


def process_results(results, attrs):
    """ Process results and print them

    Args:
        results: JSON dictionary of results
        attrs: Attributes that form the test criteria
    """

    for f in results['functions'].keys():
        print('Function: %s' % f)
        print('\t'.join(results['functions'][f]['ifuncs']))
        v = results['functions'][f]['bench-variant']
        print('Variant: %s' % v)
        print("=" * 80)
        graph_res = {}
        for res in results['functions'][f]['results']:
            attr_list = ['%s=%s' % (a, res[a]) for a in attrs]
            first = True
            key = ','.join(attr_list)
            sys.stdout.write('%s: \t' % key)
            graph_res[key] = res['timings']
            for t in res['timings']:
                sys.stdout.write ('%.2f' % t)
                if first:
                    first = False
                else:
                    diff = (res['timings'][0] - t) * 100 / res['timings'][0]

                    sys.stdout.write (' (%.2f%%)' % diff)
                sys.stdout.write('\t')
            print('')
        draw_graph(f, v, results['functions'][f]['ifuncs'], graph_res)


def main(args):
    """Program Entry Point

    Take a string benchmark output file and compare timings.
    """
    if len(args) < 3:
        print('Usage: %s <input file> <schema file> attr1 [attr2 ...]' % sys.argv[0])
        sys.exit(os.EX_USAGE)

    filename = args[0]
    schema_filename = args[1]
    attrs = args[2:]

    results = parse_file(filename, schema_filename)
    process_results(results, attrs)


if __name__ == '__main__':
    main(sys.argv[1:])
