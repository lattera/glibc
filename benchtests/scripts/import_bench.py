#!/usr/bin/python
# Copyright (C) 2015 Free Software Foundation, Inc.
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
"""Functions to import benchmark data and process it"""

import json
try:
    import jsonschema as validator
except ImportError:
    print('Could not find jsonschema module.')
    raise


def parse_bench(filename, schema_filename):
    """Parse the input file

    Parse and validate the json file containing the benchmark outputs.  Return
    the resulting object.
    Args:
        filename: Name of the benchmark output file.
    Return:
        The bench dictionary.
    """
    with open(schema_filename, 'r') as schemafile:
        schema = json.load(schemafile)
        with open(filename, 'r') as benchfile:
            bench = json.load(benchfile)
            validator.validate(bench, schema)
            do_for_all_timings(bench, lambda b, f, v:
                    b['functions'][f][v]['timings'].sort())
            return bench
