#!/usr/bin/python
# Copyright (C) 2014-2015 Free Software Foundation, Inc.
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
"""Benchmark output validator

Given a benchmark output file in json format and a benchmark schema file,
validate the output against the schema.
"""

from __future__ import print_function
import json
import sys
import os

try:
    import jsonschema
except ImportError:
    print('Could not find jsonschema module.  Output not validated.')
    # Return success because we don't want the bench target to fail just
    # because the jsonschema module was not found.
    sys.exit(os.EX_OK)


def validate_bench(benchfile, schemafile):
    """Validate benchmark file

    Validate a benchmark output file against a JSON schema.

    Args:
        benchfile: The file name of the bench.out file.
        schemafile: The file name of the JSON schema file to validate
        bench.out against.

    Exceptions:
        jsonschema.ValidationError: When bench.out is not valid
        jsonschema.SchemaError: When the JSON schema is not valid
        IOError: If any of the files are not found.
    """
    with open(benchfile, 'r') as bfile:
        with open(schemafile, 'r') as sfile:
            bench = json.load(bfile)
            schema = json.load(sfile)
            jsonschema.validate(bench, schema)

    # If we reach here, we're all good.
    print("Benchmark output in %s is valid." % benchfile)


def main(args):
    """Main entry point

    Args:
        args: The command line arguments to the program

    Returns:
        0 on success or a non-zero failure code

    Exceptions:
        Exceptions thrown by validate_bench
    """
    if len(args) != 2:
        print("Usage: %s <bench.out file> <bench.out schema>" % sys.argv[0],
                file=sys.stderr)
        return os.EX_USAGE

    validate_bench(args[0], args[1])
    return os.EX_OK


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
