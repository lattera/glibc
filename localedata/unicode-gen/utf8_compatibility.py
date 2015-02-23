#!/usr/bin/python3
# -*- coding: utf-8 -*-
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

'''
This script is useful for checking backward compatibility of newly
generated UTF-8 file from utf8_gen.py script

To see how this script is used, call it with the “-h” option:

    $ ./utf8_compatibility.py -h
    … prints usage message …
'''

import sys
import re
import argparse

# Dictionary holding the entire contents of the UnicodeData.txt file
#
# Contents of this dictionary look like this:
#
# {0: {'category': 'Cc',
#      'title': None,
#      'digit': '',
#      'name': '<control>',
#      'bidi': 'BN',
#      'combining': '0',
#      'comment': '',
#      'oldname': 'NULL',
#      'decomposition': '',
#      'upper': None,
#      'mirrored': 'N',
#      'lower': None,
#      'decdigit': '',
#      'numeric': ''},
#      …
# }
UNICODE_ATTRIBUTES = {}

# Dictionary holding the entire contents of the EastAsianWidths.txt file
#
# Contents of this dictionary look like this:
#
# {0: 'N', … , 45430: 'W', …}
EAST_ASIAN_WIDTHS = {}

def fill_attribute(code_point, fields):
    '''Stores in UNICODE_ATTRIBUTES[code_point] the values from the fields.

    One entry in the UNICODE_ATTRIBUTES dictionary represents one line
    in the UnicodeData.txt file.

    '''
    UNICODE_ATTRIBUTES[code_point] =  {
        'name': fields[1],          # Character name
        'category': fields[2],      # General category
        'combining': fields[3],     # Canonical combining classes
        'bidi': fields[4],          # Bidirectional category
        'decomposition': fields[5], # Character decomposition mapping
        'decdigit': fields[6],      # Decimal digit value
        'digit': fields[7],         # Digit value
        'numeric': fields[8],       # Numeric value
        'mirrored': fields[9],      # mirrored
        'oldname': fields[10],      # Old Unicode 1.0 name
        'comment': fields[11],      # comment
        # Uppercase mapping
        'upper': int(fields[12], 16) if fields[12] else None,
        # Lowercase mapping
        'lower': int(fields[13], 16) if fields[13] else None,
        # Titlecase mapping
        'title': int(fields[14], 16) if fields[14] else None,
    }

def fill_attributes(filename):
    '''Stores the entire contents of the UnicodeData.txt file
    in the UNICODE_ATTRIBUTES dictionary.

    A typical line for a single code point in UnicodeData.txt looks
    like this:

    0041;LATIN CAPITAL LETTER A;Lu;0;L;;;;;N;;;;0061;

    Code point ranges are indicated by pairs of lines like this:

    4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
    9FCC;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
    '''
    with open(filename, mode='r') as unicode_data_file:
        fields_start = []
        for line in unicode_data_file:
            fields = line.strip().split(';')
            if len(fields) != 15:
                sys.stderr.write(
                    'short line in file "%(f)s": %(l)s\n' %{
                    'f': filename, 'l': line})
                exit(1)
            if fields[2] == 'Cs':
                # Surrogates are UTF-16 artefacts,
                # not real characters. Ignore them.
                fields_start = []
                continue
            if fields[1].endswith(', First>'):
                fields_start = fields
                fields_start[1] = fields_start[1].split(',')[0][1:]
                continue
            if fields[1].endswith(', Last>'):
                fields[1] = fields[1].split(',')[0][1:]
                if fields[1:] != fields_start[1:]:
                    sys.stderr.write(
                        'broken code point range in file "%(f)s": %(l)s\n' %{
                            'f': filename, 'l': line})
                    exit(1)
                for code_point in range(
                        int(fields_start[0], 16),
                        int(fields[0], 16)+1):
                    fill_attribute(code_point, fields)
                fields_start = []
                continue
            fill_attribute(int(fields[0], 16), fields)
            fields_start = []

def fill_east_asian_widths(filename):
    '''Stores the entire contents of the EastAsianWidths.txt file
    in the EAST_ASIAN_WIDTHS dictionary.

    Lines in EastAsianWidths.txt are either a code point range like
    this:

    9FCD..9FFF;W     # Cn    [51] <reserved-9FCD>..<reserved-9FFF>

    or a single code point like this:

    A015;W           # Lm         YI SYLLABLE WU
    '''
    with open(filename, mode='r') as east_asian_widths_file:
        for line in east_asian_widths_file:
            match = re.match(
                r'^(?P<codepoint1>[0-9A-F]{4,6})'
                +r'(?:\.\.(?P<codepoint2>[0-9A-F]{4,6}))?'
                +r'\s*;\s*(?P<property>[a-zA-Z]+)',
                line)
            if not match:
                continue
            start = match.group('codepoint1')
            end = match.group('codepoint2')
            if not end:
                end = start
            for code_point in range(int(start, 16), int(end, 16)+1):
                EAST_ASIAN_WIDTHS[code_point] = match.group('property')

def ucs_symbol(code_point):
    '''Return the UCS symbol string for a Unicode character.'''
    if code_point < 0x10000:
        return '<U{:04X}>'.format(code_point)
    else:
        return '<U{:08X}>'.format(code_point)

def create_charmap_dictionary(file_name):
    '''Create a dictionary for all code points found in the CHARMAP
    section of a file
    '''
    with open(file_name, mode='r') as utf8_file:
        charmap_dictionary = {}
        for line in utf8_file:
            if line.startswith('CHARMAP'):
                break
        for line in utf8_file:
            if line.startswith('END CHARMAP'):
                return charmap_dictionary
            if line.startswith('%'):
                continue
            match = re.match(
                r'^<U(?P<codepoint1>[0-9A-F]{4,8})>'
                +r'(:?\.\.<U(?P<codepoint2>[0-9-A-F]{4,8})>)?'
                +r'\s+(?P<hexutf8>(/x[0-9a-f]{2}){1,4})',
                line)
            if not match:
                continue
            codepoint1 = match.group('codepoint1')
            codepoint2 = match.group('codepoint2')
            if not codepoint2:
                codepoint2 = codepoint1
            for i in range(int(codepoint1, 16),
                           int(codepoint2, 16) + 1):
                charmap_dictionary[i] = match.group('hexutf8')
        sys.stderr.write('No “CHARMAP” or no “END CHARMAP” found in %s\n'
                         %file_name)
        exit(1)

def check_charmap(original_file_name, new_file_name):
    '''Report differences in the CHARMAP section between the old and the
    new file
    '''
    print('************************************************************')
    print('Report on CHARMAP:')
    ocharmap = create_charmap_dictionary(original_file_name)
    ncharmap = create_charmap_dictionary(new_file_name)
    print('------------------------------------------------------------')
    print('Total removed characters in newly generated CHARMAP: %d'
          %len(set(ocharmap)-set(ncharmap)))
    if ARGS.show_missing_characters:
        for key in sorted(set(ocharmap)-set(ncharmap)):
            print('removed: {:s}     {:s} {:s}'.format(
                ucs_symbol(key),
                ocharmap[key],
                UNICODE_ATTRIBUTES[key]['name'] \
                if key in UNICODE_ATTRIBUTES else None))
    print('------------------------------------------------------------')
    changed_charmap = {}
    for key in set(ocharmap).intersection(set(ncharmap)):
        if ocharmap[key] != ncharmap[key]:
            changed_charmap[key] = (ocharmap[key], ncharmap[key])
    print('Total changed characters in newly generated CHARMAP: %d'
          %len(changed_charmap))
    if ARGS.show_changed_characters:
        for key in sorted(changed_charmap):
            print('changed: {:s}     {:s}->{:s} {:s}'.format(
                ucs_symbol(key),
                changed_charmap[key][0],
                changed_charmap[key][1],
                UNICODE_ATTRIBUTES[key]['name'] \
                if key in UNICODE_ATTRIBUTES else None))
    print('------------------------------------------------------------')
    print('Total added characters in newly generated CHARMAP: %d'
          %len(set(ncharmap)-set(ocharmap)))
    if ARGS.show_added_characters:
        for key in sorted(set(ncharmap)-set(ocharmap)):
            print('added: {:s}     {:s} {:s}'.format(
                ucs_symbol(key),
                ncharmap[key],
                UNICODE_ATTRIBUTES[key]['name'] \
                if key in UNICODE_ATTRIBUTES else None))

def create_width_dictionary(file_name):
    '''Create a dictionary for all code points found in the WIDTH
    section of a file
    '''
    with open(file_name, mode='r') as utf8_file:
        width_dictionary = {}
        for line in utf8_file:
            if line.startswith('WIDTH'):
                break
        for line in utf8_file:
            if line.startswith('END WIDTH'):
                return width_dictionary
            match = re.match(
                r'^<U(?P<codepoint1>[0-9A-F]{4,8})>'
                +r'(:?\.\.\.<U(?P<codepoint2>[0-9-A-F]{4,8})>)?'
                +r'\s+(?P<width>[02])',
                line)
            if not match:
                continue
            codepoint1 = match.group('codepoint1')
            codepoint2 = match.group('codepoint2')
            if not codepoint2:
                codepoint2 = codepoint1
            for i in range(int(codepoint1, 16),
                           int(codepoint2, 16) + 1):
                width_dictionary[i] = int(match.group('width'))
        sys.stderr.write('No “WIDTH” or no “END WIDTH” found in %s\n' %file)

def check_width(original_file_name, new_file_name):
    '''Report differences in the WIDTH section between the old and the new
    file
    '''
    print('************************************************************')
    print('Report on WIDTH:')
    owidth = create_width_dictionary(original_file_name)
    nwidth = create_width_dictionary(new_file_name)
    print('------------------------------------------------------------')
    print('Total removed characters in newly generated WIDTH: %d'
          %len(set(owidth)-set(nwidth)))
    print('(Characters not in WIDTH get width 1 by default, '
          + 'i.e. these have width 1 now.)')
    if ARGS.show_missing_characters:
        for key in sorted(set(owidth)-set(nwidth)):
            print('removed: {:s} '.format(ucs_symbol(key))
                  + '{:d} : '.format(owidth[key])
                  + 'eaw={:s} '.format(
                      EAST_ASIAN_WIDTHS[key]
                      if key in EAST_ASIAN_WIDTHS else None)
                  + 'category={:2s} '.format(
                      UNICODE_ATTRIBUTES[key]['category']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'bidi={:3s} '.format(
                      UNICODE_ATTRIBUTES[key]['bidi']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'name={:s}'.format(
                      UNICODE_ATTRIBUTES[key]['name']
                      if key in UNICODE_ATTRIBUTES else None))
    print('------------------------------------------------------------')
    changed_width = {}
    for key in set(owidth).intersection(set(nwidth)):
        if owidth[key] != nwidth[key]:
            changed_width[key] = (owidth[key], nwidth[key])
    print('Total changed characters in newly generated WIDTH: %d'
          %len(changed_width))
    if ARGS.show_changed_characters:
        for key in sorted(changed_width):
            print('changed width: {:s} '.format(ucs_symbol(key))
                  + '{:d}->{:d} : '.format(changed_width[key][0],
                                          changed_width[key][1])
                  + 'eaw={:s} '.format(
                      EAST_ASIAN_WIDTHS[key]
                      if key in EAST_ASIAN_WIDTHS else None)
                  + 'category={:2s} '.format(
                      UNICODE_ATTRIBUTES[key]['category']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'bidi={:3s} '.format(
                      UNICODE_ATTRIBUTES[key]['bidi']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'name={:s}'.format(
                      UNICODE_ATTRIBUTES[key]['name']
                      if key in UNICODE_ATTRIBUTES else None))
    print('------------------------------------------------------------')
    print('Total added characters in newly generated WIDTH: %d'
          %len(set(nwidth)-set(owidth)))
    print('(Characters not in WIDTH get width 1 by default, '
          + 'i.e. these had width 1 before.)')
    if ARGS.show_added_characters:
        for key in sorted(set(nwidth)-set(owidth)):
            print('added: {:s} '.format(ucs_symbol(key))
                  + '{:d} : '.format(nwidth[key])
                  + 'eaw={:s} '.format(
                      EAST_ASIAN_WIDTHS[key]
                      if key in EAST_ASIAN_WIDTHS else None)
                  + 'category={:2s} '.format(
                      UNICODE_ATTRIBUTES[key]['category']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'bidi={:3s} '.format(
                      UNICODE_ATTRIBUTES[key]['bidi']
                      if key in UNICODE_ATTRIBUTES else None)
                  + 'name={:s}'.format(
                      UNICODE_ATTRIBUTES[key]['name']
                      if key in UNICODE_ATTRIBUTES else None))

if __name__ == "__main__":
    PARSER = argparse.ArgumentParser(
        description='''
        Compare the contents of LC_CTYPE in two files and check for errors.
        ''')
    PARSER.add_argument(
        '-o', '--old_utf8_file',
        nargs='?',
        required=True,
        type=str,
        help='The old UTF-8 file.')
    PARSER.add_argument(
        '-n', '--new_utf8_file',
        nargs='?',
        required=True,
        type=str,
        help='The new UTF-8 file.')
    PARSER.add_argument(
        '-u', '--unicode_data_file',
        nargs='?',
        type=str,
        help='The UnicodeData.txt file to read.')
    PARSER.add_argument(
        '-e', '--east_asian_width_file',
        nargs='?',
        type=str,
        help='The EastAsianWidth.txt file to read.')
    PARSER.add_argument(
        '-a', '--show_added_characters',
        action='store_true',
        help='Show characters which were added in detail.')
    PARSER.add_argument(
        '-m', '--show_missing_characters',
        action='store_true',
        help='Show characters which were removed in detail.')
    PARSER.add_argument(
        '-c', '--show_changed_characters',
        action='store_true',
        help='Show characters whose width was changed in detail.')
    ARGS = PARSER.parse_args()

    if ARGS.unicode_data_file:
        fill_attributes(ARGS.unicode_data_file)
    if ARGS.east_asian_width_file:
        fill_east_asian_widths(ARGS.east_asian_width_file)
    check_charmap(ARGS.old_utf8_file, ARGS.new_utf8_file)
    check_width(ARGS.old_utf8_file, ARGS.new_utf8_file)
