#!/usr/bin/python3
#
# Generate a Unicode conforming LC_CTYPE category from a UnicodeData file.
# Copyright (C) 2014-2015 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Based on gen-unicode-ctype.c by Bruno Haible <haible@clisp.cons.org>, 2000.
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

'''
Generate a Unicode conforming LC_CTYPE category from UnicodeData.txt and
DerivedCoreProperties.txt files.

To see how this script is used, call it with the “-h” option:

    $ ./gen_unicode_ctype.py -h
    … prints usage message …
'''

import argparse
import sys
import time
import re

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

# Dictionary holding the entire contents of the DerivedCoreProperties.txt file
#
# Contents of this dictionary look like this:
#
# {917504: ['Default_Ignorable_Code_Point'],
#  917505: ['Case_Ignorable', 'Default_Ignorable_Code_Point'],
#  …
# }
DERIVED_CORE_PROPERTIES = {}

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

def fill_derived_core_properties(filename):
    '''Stores the entire contents of the DerivedCoreProperties.txt file
    in the DERIVED_CORE_PROPERTIES dictionary.

    Lines in DerivedCoreProperties.txt are either a code point range like
    this:

    0061..007A    ; Lowercase # L&  [26] LATIN SMALL LETTER A..LATIN SMALL LETTER Z

    or a single code point like this:

    00AA          ; Lowercase # Lo       FEMININE ORDINAL INDICATOR

    '''
    with open(filename, mode='r') as derived_core_properties_file:
        for line in derived_core_properties_file:
            match = re.match(
                r'^(?P<codepoint1>[0-9A-F]{4,6})'
                + r'(?:\.\.(?P<codepoint2>[0-9A-F]{4,6}))?'
                + r'\s*;\s*(?P<property>[a-zA-Z_]+)',
                line)
            if not match:
                continue
            start = match.group('codepoint1')
            end = match.group('codepoint2')
            if not end:
                end = start
            for code_point in range(int(start, 16), int(end, 16)+1):
                prop = match.group('property')
                if code_point in DERIVED_CORE_PROPERTIES:
                    DERIVED_CORE_PROPERTIES[code_point].append(prop)
                else:
                    DERIVED_CORE_PROPERTIES[code_point] = [prop]

def to_upper(code_point):
    '''Returns the code point of the uppercase version
    of the given code point'''
    if (UNICODE_ATTRIBUTES[code_point]['name']
        and UNICODE_ATTRIBUTES[code_point]['upper']):
        return UNICODE_ATTRIBUTES[code_point]['upper']
    else:
        return code_point

def to_lower(code_point):
    '''Returns the code point of the lowercase version
    of the given code point'''
    if (UNICODE_ATTRIBUTES[code_point]['name']
        and UNICODE_ATTRIBUTES[code_point]['lower']):
        return UNICODE_ATTRIBUTES[code_point]['lower']
    else:
        return code_point

def to_title(code_point):
    '''Returns the code point of the titlecase version
    of the given code point'''
    if (UNICODE_ATTRIBUTES[code_point]['name']
        and UNICODE_ATTRIBUTES[code_point]['title']):
        return UNICODE_ATTRIBUTES[code_point]['title']
    else:
        return code_point

def is_upper(code_point):
    '''Checks whether the character with this code point is uppercase'''
    return (to_lower(code_point) != code_point
            or (code_point in DERIVED_CORE_PROPERTIES
                and 'Uppercase' in DERIVED_CORE_PROPERTIES[code_point]))

def is_lower(code_point):
    '''Checks whether the character with this code point is lowercase'''
    # Some characters are defined as “Lowercase” in
    # DerivedCoreProperties.txt but do not have a mapping to upper
    # case. For example, ꜰ U+A72F “LATIN LETTER SMALL CAPITAL F” is
    # one of these.
    return (to_upper(code_point) != code_point
            # <U00DF> is lowercase, but without simple to_upper mapping.
            or code_point == 0x00DF
            or (code_point in DERIVED_CORE_PROPERTIES
                and 'Lowercase' in DERIVED_CORE_PROPERTIES[code_point]))

def is_alpha(code_point):
    '''Checks whether the character with this code point is alphabetic'''
    return ((code_point in DERIVED_CORE_PROPERTIES
             and
             'Alphabetic' in DERIVED_CORE_PROPERTIES[code_point])
            or
            # Consider all the non-ASCII digits as alphabetic.
            # ISO C 99 forbids us to have them in category “digit”,
            # but we want iswalnum to return true on them.
            (UNICODE_ATTRIBUTES[code_point]['category'] == 'Nd'
             and not (code_point >= 0x0030 and code_point <= 0x0039)))

def is_digit(code_point):
    '''Checks whether the character with this code point is a digit'''
    if False:
        return (UNICODE_ATTRIBUTES[code_point]['name']
                and UNICODE_ATTRIBUTES[code_point]['category'] == 'Nd')
        # Note: U+0BE7..U+0BEF and U+1369..U+1371 are digit systems without
        # a zero.  Must add <0> in front of them by hand.
    else:
        # SUSV2 gives us some freedom for the "digit" category, but ISO C 99
        # takes it away:
        # 7.25.2.1.5:
        #    The iswdigit function tests for any wide character that
        #    corresponds to a decimal-digit character (as defined in 5.2.1).
        # 5.2.1:
        #    the 10 decimal digits 0 1 2 3 4 5 6 7 8 9
        return (code_point >= 0x0030 and code_point <= 0x0039)

def is_outdigit(code_point):
    '''Checks whether the character with this code point is outdigit'''
    return (code_point >= 0x0030 and code_point <= 0x0039)

def is_blank(code_point):
    '''Checks whether the character with this code point is blank'''
    return (code_point == 0x0009 # '\t'
            # Category Zs without mention of '<noBreak>'
            or (UNICODE_ATTRIBUTES[code_point]['name']
                and UNICODE_ATTRIBUTES[code_point]['category'] == 'Zs'
                and '<noBreak>' not in
                UNICODE_ATTRIBUTES[code_point]['decomposition']))

def is_space(code_point):
    '''Checks whether the character with this code point is a space'''
    # Don’t make U+00A0 a space. Non-breaking space means that all programs
    # should treat it like a punctuation character, not like a space.
    return (code_point == 0x0020 # ' '
            or code_point == 0x000C # '\f'
            or code_point == 0x000A # '\n'
            or code_point == 0x000D # '\r'
            or code_point == 0x0009 # '\t'
            or code_point == 0x000B # '\v'
            # Categories Zl, Zp, and Zs without mention of "<noBreak>"
            or (UNICODE_ATTRIBUTES[code_point]['name']
                and
                (UNICODE_ATTRIBUTES[code_point]['category'] in ['Zl', 'Zp']
                 or
                 (UNICODE_ATTRIBUTES[code_point]['category'] in ['Zs']
                  and
                  '<noBreak>' not in
                  UNICODE_ATTRIBUTES[code_point]['decomposition']))))

def is_cntrl(code_point):
    '''Checks whether the character with this code point is
    a control character'''
    return (UNICODE_ATTRIBUTES[code_point]['name']
            and (UNICODE_ATTRIBUTES[code_point]['name'] == '<control>'
                 or
                 UNICODE_ATTRIBUTES[code_point]['category'] in ['Zl', 'Zp']))

def is_xdigit(code_point):
    '''Checks whether the character with this code point is
    a hexadecimal digit'''
    if False:
        return (is_digit(code_point)
                or (code_point >= 0x0041 and code_point <= 0x0046)
                or (code_point >= 0x0061 and code_point <= 0x0066))
    else:
        # SUSV2 gives us some freedom for the "xdigit" category, but ISO C 99
        # takes it away:
        # 7.25.2.1.12:
        #    The iswxdigit function tests for any wide character that
        #    corresponds to a hexadecimal-digit character (as defined
        #    in 6.4.4.1).
        # 6.4.4.1:
        #    hexadecimal-digit: one of
        #    0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
        return ((code_point >= 0x0030 and code_point  <= 0x0039)
                or (code_point >= 0x0041 and code_point <= 0x0046)
                or (code_point >= 0x0061 and code_point <= 0x0066))

def is_graph(code_point):
    '''Checks whether the character with this code point is
    a graphical character'''
    return (UNICODE_ATTRIBUTES[code_point]['name']
            and UNICODE_ATTRIBUTES[code_point]['name'] != '<control>'
            and not is_space(code_point))

def is_print(code_point):
    '''Checks whether the character with this code point is printable'''
    return (UNICODE_ATTRIBUTES[code_point]['name']
            and UNICODE_ATTRIBUTES[code_point]['name'] != '<control>'
            and UNICODE_ATTRIBUTES[code_point]['category'] not in ['Zl', 'Zp'])

def is_punct(code_point):
    '''Checks whether the character with this code point is punctuation'''
    if False:
        return (UNICODE_ATTRIBUTES[code_point]['name']
                and UNICODE_ATTRIBUTES[code_point]['category'].startswith('P'))
    else:
        # The traditional POSIX definition of punctuation is every graphic,
        # non-alphanumeric character.
        return (is_graph(code_point)
                and not is_alpha(code_point)
                and not is_digit(code_point))

def is_combining(code_point):
    '''Checks whether the character with this code point is
    a combining character'''
    # Up to Unicode 3.0.1 we took the Combining property from the PropList.txt
    # file. In 3.0.1 it was identical to the union of the general categories
    # "Mn", "Mc", "Me". In Unicode 3.1 this property has been dropped from the
    # PropList.txt file, so we take the latter definition.
    return (UNICODE_ATTRIBUTES[code_point]['name']
            and
            UNICODE_ATTRIBUTES[code_point]['category'] in ['Mn', 'Mc', 'Me'])

def is_combining_level3(code_point):
    '''Checks whether the character with this code point is
    a combining level3 character'''
    return (is_combining(code_point)
            and
            int(UNICODE_ATTRIBUTES[code_point]['combining']) in range(0, 200))

def ucs_symbol(code_point):
    '''Return the UCS symbol string for a Unicode character.'''
    if code_point < 0x10000:
        return '<U{:04X}>'.format(code_point)
    else:
        return '<U{:08X}>'.format(code_point)

def ucs_symbol_range(code_point_low, code_point_high):
    '''Returns a string UCS symbol string for a code point range.

    Example:

    <U0041>..<U005A>
    '''
    return ucs_symbol(code_point_low) + '..' + ucs_symbol(code_point_high)

def code_point_ranges(is_class_function):
    '''Returns a list of ranges of code points for which is_class_function
    returns True.

    Example:

    [[65, 90], [192, 214], [216, 222], [256], … ]
    '''
    cp_ranges  = []
    for code_point in sorted(UNICODE_ATTRIBUTES):
        if is_class_function(code_point):
            if (cp_ranges
                and cp_ranges[-1][-1] == code_point - 1):
                if len(cp_ranges[-1]) == 1:
                    cp_ranges[-1].append(code_point)
                else:
                    cp_ranges[-1][-1] = code_point
            else:
                cp_ranges.append([code_point])
    return cp_ranges

def output_charclass(i18n_file, class_name, is_class_function):
    '''Output a LC_CTYPE character class section

    Example:

    upper /
       <U0041>..<U005A>;<U00C0>..<U00D6>;<U00D8>..<U00DE>;<U0100>;<U0102>;/
       …
       <U0001D790>..<U0001D7A8>;<U0001D7CA>;<U0001F130>..<U0001F149>;/
       <U0001F150>..<U0001F169>;<U0001F170>..<U0001F189>
    '''
    cp_ranges = code_point_ranges(is_class_function)
    if cp_ranges:
        i18n_file.write('%s /\n' %class_name)
        max_column = 75
        prefix = '   '
        line = prefix
        range_string = ''
        for code_point_range in cp_ranges:
            if line.strip():
                line  += ';'
            if len(code_point_range) == 1:
                range_string = ucs_symbol(code_point_range[0])
            else:
                range_string = ucs_symbol_range(
                    code_point_range[0], code_point_range[-1])
            if len(line+range_string) > max_column:
                i18n_file.write(line+'/\n')
                line = prefix
            line += range_string
        if line.strip():
            i18n_file.write(line+'\n')
        i18n_file.write('\n')

def output_charmap(i18n_file, map_name, map_function):
    '''Output a LC_CTYPE character map section

    Example:

    toupper /
      (<U0061>,<U0041>);(<U0062>,<U0042>);(<U0063>,<U0043>);(<U0064>,<U0044>);/
      …
      (<U000118DC>,<U000118BC>);(<U000118DD>,<U000118BD>);/
      (<U000118DE>,<U000118BE>);(<U000118DF>,<U000118BF>)
    '''
    max_column = 75
    prefix = '   '
    line = prefix
    map_string = ''
    i18n_file.write('%s /\n' %map_name)
    for code_point in sorted(UNICODE_ATTRIBUTES):
        mapped = map_function(code_point)
        if code_point != mapped:
            if line.strip():
                line += ';'
            map_string = '(' \
                         + ucs_symbol(code_point) \
                         + ',' \
                         + ucs_symbol(mapped) \
                         + ')'
            if len(line+map_string) > max_column:
                i18n_file.write(line+'/\n')
                line = prefix
            line += map_string
    if line.strip():
        i18n_file.write(line+'\n')
    i18n_file.write('\n')

def verifications():
    '''Tests whether the is_* functions observe the known restrictions'''
    for code_point in sorted(UNICODE_ATTRIBUTES):
        # toupper restriction: "Only characters specified for the keywords
        # lower and upper shall be specified.
        if (to_upper(code_point) != code_point
            and not (is_lower(code_point) or is_upper(code_point))):
            sys.stderr.write(
                ('%(sym)s is not upper|lower '
                 + 'but toupper(0x%(c)04X) = 0x%(uc)04X\n') %{
                    'sym': ucs_symbol(code_point),
                    'c': code_point,
                    'uc': to_upper(code_point)})
        # tolower restriction: "Only characters specified for the keywords
        # lower and upper shall be specified.
        if (to_lower(code_point) != code_point
            and not (is_lower(code_point) or is_upper(code_point))):
            sys.stderr.write(
                ('%(sym)s is not upper|lower '
                 + 'but tolower(0x%(c)04X) = 0x%(uc)04X\n') %{
                    'sym': ucs_symbol(code_point),
                    'c': code_point,
                    'uc': to_lower(code_point)})
        # alpha restriction: "Characters classified as either upper or lower
        # shall automatically belong to this class.
        if ((is_lower(code_point) or is_upper(code_point))
             and not is_alpha(code_point)):
            sys.stderr.write('%(sym)s is upper|lower but not alpha\n' %{
                'sym': ucs_symbol(code_point)})
        # alpha restriction: “No character specified for the keywords cntrl,
        # digit, punct or space shall be specified.”
        if (is_alpha(code_point) and is_cntrl(code_point)):
            sys.stderr.write('%(sym)s is alpha and cntrl\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_alpha(code_point) and is_digit(code_point)):
            sys.stderr.write('%(sym)s is alpha and digit\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_alpha(code_point) and is_punct(code_point)):
            sys.stderr.write('%(sym)s is alpha and punct\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_alpha(code_point) and is_space(code_point)):
            sys.stderr.write('%(sym)s is alpha and space\n' %{
                'sym': ucs_symbol(code_point)})
        # space restriction: “No character specified for the keywords upper,
        # lower, alpha, digit, graph or xdigit shall be specified.”
        # upper, lower, alpha already checked above.
        if (is_space(code_point) and is_digit(code_point)):
            sys.stderr.write('%(sym)s is space and digit\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_space(code_point) and is_graph(code_point)):
            sys.stderr.write('%(sym)s is space and graph\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_space(code_point) and is_xdigit(code_point)):
            sys.stderr.write('%(sym)s is space and xdigit\n' %{
                'sym': ucs_symbol(code_point)})
        # cntrl restriction: “No character specified for the keywords upper,
        # lower, alpha, digit, punct, graph, print or xdigit shall be
        # specified.”  upper, lower, alpha already checked above.
        if (is_cntrl(code_point) and is_digit(code_point)):
            sys.stderr.write('%(sym)s is cntrl and digit\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_cntrl(code_point) and is_punct(code_point)):
            sys.stderr.write('%(sym)s is cntrl and punct\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_cntrl(code_point) and is_graph(code_point)):
            sys.stderr.write('%(sym)s is cntrl and graph\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_cntrl(code_point) and is_print(code_point)):
            sys.stderr.write('%(sym)s is cntrl and print\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_cntrl(code_point) and is_xdigit(code_point)):
            sys.stderr.write('%(sym)s is cntrl and xdigit\n' %{
                'sym': ucs_symbol(code_point)})
        # punct restriction: “No character specified for the keywords upper,
        # lower, alpha, digit, cntrl, xdigit or as the <space> character shall
        # be specified.”  upper, lower, alpha, cntrl already checked above.
        if (is_punct(code_point) and is_digit(code_point)):
            sys.stderr.write('%(sym)s is punct and digit\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_punct(code_point) and is_xdigit(code_point)):
            sys.stderr.write('%(sym)s is punct and xdigit\n' %{
                'sym': ucs_symbol(code_point)})
        if (is_punct(code_point) and code_point == 0x0020):
            sys.stderr.write('%(sym)s is punct\n' %{
                'sym': ucs_symbol(code_point)})
        # graph restriction: “No character specified for the keyword cntrl
        # shall be specified.”  Already checked above.

        # print restriction: “No character specified for the keyword cntrl
        # shall be specified.”  Already checked above.

        # graph - print relation: differ only in the <space> character.
        # How is this possible if there are more than one space character?!
        # I think susv2/xbd/locale.html should speak of “space characters”,
        # not “space character”.
        if (is_print(code_point)
            and not (is_graph(code_point) or is_space(code_point))):
            sys.stderr.write('%(sym)s is print but not graph|<space>\n' %{
                'sym': ucs_symbol(code_point)})
        if (not is_print(code_point)
            and (is_graph(code_point) or code_point == 0x0020)):
            sys.stderr.write('%(sym)s is graph|<space> but not print\n' %{
                'sym': ucs_symbol(code_point)})

def read_input_file(filename):
    '''Reads the original glibc i18n file to get the original head
    and tail.

    We want to replace only the character classes in LC_CTYPE, and the
    date stamp. All the rest of the i18n file should stay unchanged.
    To avoid having to cut and paste the generated data into the
    original file, it is helpful to read the original file here
    to be able to generate a complete result file.
    '''
    head = tail = ''
    with open(filename, mode='r') as i18n_file:
        for line in i18n_file:
            match = re.match(
                r'^(?P<key>date\s+)(?P<value>"[0-9]{4}-[0-9]{2}-[0-9]{2}")',
                line)
            if match:
                line = match.group('key') \
                       + '"{:s}"\n'.format(time.strftime('%Y-%m-%d'))
            head = head + line
            if line.startswith('LC_CTYPE'):
                break
        for line in i18n_file:
            if line.startswith('translit_start'):
                tail = line
                break
        for line in i18n_file:
            tail = tail + line
    return (head, tail)

def output_head(i18n_file, unicode_version, head=''):
    '''Write the header of the output file, i.e. the part of the file
    before the “LC_CTYPE” line.
    '''
    if ARGS.input_file and head:
        i18n_file.write(head)
    else:
        i18n_file.write('escape_char /\n')
        i18n_file.write('comment_char %\n')
        i18n_file.write('\n')
        i18n_file.write('% Generated automatically by '
                        + 'gen_unicode_ctype.py '
                        + 'for Unicode {:s}.\n'.format(unicode_version))
        i18n_file.write('\n')
        i18n_file.write('LC_IDENTIFICATION\n')
        i18n_file.write('title     "Unicode {:s} FDCC-set"\n'.format(
            unicode_version))
        i18n_file.write('source    "UnicodeData.txt, '
                        + 'DerivedCoreProperties.txt"\n')
        i18n_file.write('address   ""\n')
        i18n_file.write('contact   ""\n')
        i18n_file.write('email     "bug-glibc-locales@gnu.org"\n')
        i18n_file.write('tel       ""\n')
        i18n_file.write('fax       ""\n')
        i18n_file.write('language  ""\n')
        i18n_file.write('territory "Earth"\n')
        i18n_file.write('revision  "{:s}"\n'.format(unicode_version))
        i18n_file.write('date      "{:s}"\n'.format(
            time.strftime('%Y-%m-%d')))
        i18n_file.write('category  "unicode:2014";LC_CTYPE\n')
        i18n_file.write('END LC_IDENTIFICATION\n')
        i18n_file.write('\n')
        i18n_file.write('LC_CTYPE\n')

def output_tail(i18n_file, tail=''):
    '''Write the tail of the output file, i.e. the part of the file
    after the last “LC_CTYPE” character class.
    '''
    if ARGS.input_file and tail:
        i18n_file.write(tail)
    else:
        i18n_file.write('END LC_CTYPE\n')

def output_tables(i18n_file, unicode_version):
    '''Write the new LC_CTYPE character classes to the output file'''
    i18n_file.write('% The following is the 14652 i18n fdcc-set '
                    + 'LC_CTYPE category.\n')
    i18n_file.write('% It covers Unicode version {:s}.\n'.format(
        unicode_version))
    i18n_file.write('% The character classes and mapping tables were '
                    + 'automatically\n')
    i18n_file.write('% generated using the gen_unicode_ctype.py '
                    + 'program.\n\n')
    i18n_file.write('% The "upper" class reflects the uppercase '
                    + 'characters of class "alpha"\n')
    output_charclass(i18n_file, 'upper', is_upper)
    i18n_file.write('% The "lower" class reflects the lowercase '
                    + 'characters of class "alpha"\n')
    output_charclass(i18n_file, 'lower', is_lower)
    i18n_file.write('% The "alpha" class of the "i18n" FDCC-set is '
                    + 'reflecting\n')
    i18n_file.write('% the recommendations in TR 10176 annex A\n')
    output_charclass(i18n_file, 'alpha', is_alpha)
    i18n_file.write('% The "digit" class must only contain the '
                    + 'BASIC LATIN digits, says ISO C 99\n')
    i18n_file.write('% (sections 7.25.2.1.5 and 5.2.1).\n')
    output_charclass(i18n_file, 'digit', is_digit)
    i18n_file.write('% The "outdigit" information is by default '
                    + '"0" to "9".  We don\'t have to\n')
    i18n_file.write('% provide it here since localedef will fill '
               + 'in the bits and it would\n')
    i18n_file.write('% prevent locales copying this file define '
                    + 'their own values.\n')
    i18n_file.write('% outdigit /\n')
    i18n_file.write('%    <U0030>..<U0039>\n\n')
    # output_charclass(i18n_file, 'outdigit', is_outdigit)
    output_charclass(i18n_file, 'space', is_space)
    output_charclass(i18n_file, 'cntrl', is_cntrl)
    output_charclass(i18n_file, 'punct', is_punct)
    output_charclass(i18n_file, 'graph', is_graph)
    output_charclass(i18n_file, 'print', is_print)
    i18n_file.write('% The "xdigit" class must only contain the '
                    + 'BASIC LATIN digits and A-F, a-f,\n')
    i18n_file.write('% says ISO C 99 '
                    + '(sections 7.25.2.1.12 and 6.4.4.1).\n')
    output_charclass(i18n_file, 'xdigit', is_xdigit)
    output_charclass(i18n_file, 'blank', is_blank)
    output_charmap(i18n_file, 'toupper', to_upper)
    output_charmap(i18n_file, 'tolower', to_lower)
    output_charmap(i18n_file, 'map "totitle";', to_title)
    i18n_file.write('% The "combining" class reflects ISO/IEC 10646-1 '
                    + 'annex B.1\n')
    i18n_file.write('% That is, all combining characters (level 2+3).\n')
    output_charclass(i18n_file, 'class "combining";', is_combining)
    i18n_file.write('% The "combining_level3" class reflects '
                    + 'ISO/IEC 10646-1 annex B.2\n')
    i18n_file.write('% That is, combining characters of level 3.\n')
    output_charclass(i18n_file,
                     'class "combining_level3";', is_combining_level3)

if __name__ == "__main__":
    PARSER = argparse.ArgumentParser(
        description='''
        Generate a Unicode conforming LC_CTYPE category from
        UnicodeData.txt and DerivedCoreProperties.txt files.
        ''')
    PARSER.add_argument(
        '-u', '--unicode_data_file',
        nargs='?',
        type=str,
        default='UnicodeData.txt',
        help=('The UnicodeData.txt file to read, '
              + 'default: %(default)s'))
    PARSER.add_argument(
        '-d', '--derived_core_properties_file',
        nargs='?',
        type=str,
        default='DerivedCoreProperties.txt',
        help=('The DerivedCoreProperties.txt file to read, '
              + 'default: %(default)s'))
    PARSER.add_argument(
        '-i', '--input_file',
        nargs='?',
        type=str,
        help='''The original glibc/localedata/locales/i18n file.''')
    PARSER.add_argument(
        '-o', '--output_file',
        nargs='?',
        type=str,
        default='i18n.new',
        help='''The file which shall contain the generated LC_CTYPE category,
        default: %(default)s.  If the original
        glibc/localedata/locales/i18n has been given
        as an option, all data from the original file
        except the newly generated LC_CTYPE character
        classes and the date stamp in
        LC_IDENTIFICATION will be copied unchanged
        into the output file.  ''')
    PARSER.add_argument(
        '--unicode_version',
        nargs='?',
        required=True,
        type=str,
        help='The Unicode version of the input files used.')
    ARGS = PARSER.parse_args()

    fill_attributes(ARGS.unicode_data_file)
    fill_derived_core_properties(ARGS.derived_core_properties_file)
    verifications()
    HEAD = TAIL = ''
    if ARGS.input_file:
        (HEAD, TAIL) = read_input_file(ARGS.input_file)
    with open(ARGS.output_file, mode='w') as I18N_FILE:
        output_head(I18N_FILE, ARGS.unicode_version, head=HEAD)
        output_tables(I18N_FILE, ARGS.unicode_version)
        output_tail(I18N_FILE, tail=TAIL)
