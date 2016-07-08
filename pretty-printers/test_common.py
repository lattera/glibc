# Common functions and variables for testing the Python pretty printers.
#
# Copyright (C) 2016 Free Software Foundation, Inc.
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

"""These tests require PExpect.

Attributes:
    PASS, FAIL, UNSUPPORTED (int): Test exit codes, as per evaluate-test.sh.
    GDB (string): A string with the name of the gdb binary.
    gdb (pexpect.spawn): The gdb process, as handled by PExpect.
    gdb_prompt (raw string): A pattern for matching the gdb prompt.
"""

import os
import re

PASS = 0
FAIL = 1
UNSUPPORTED = 77
GDB = 'gdb'

try:
    import pexpect
except ImportError:
    print('PExpect must be installed in order to test the pretty printers.')
    exit(UNSUPPORTED)

if not pexpect.which(GDB):
    print('gdb must be installed in order to test the pretty printers.')
    exit(UNSUPPORTED)

class NoLineError(Exception):
    """Custom exception which indicates that a test file doesn't contain
    the requested string.
    """

    def __init__(self, file_name, string):
        """Constructor.

        Args:
            file_name (string): The name of the test file.
            string (string): The string that was requested.
        """

        super(NoLineError, self).__init__()
        self.file_name = file_name
        self.string = string

    def __str__(self):
        """Shows a readable representation of the exception."""

        return ('File {0} has no line containing the following string: {1}'
                .format(self.file_name, self.string))

timeout = 1
TIMEOUTFACTOR = os.environ.get('TIMEOUTFACTOR')

if TIMEOUTFACTOR:
    timeout = int(TIMEOUTFACTOR)

gdb = pexpect.spawn(GDB, echo=False, timeout=timeout)

# Set the gdb prompt to a custom one, so that user-defined prompts won't
# interfere.  We assume the user won't have his prompt set to this.
gdb_prompt = r'gdb-test% '
gdb.sendline('set prompt {0}'.format(gdb_prompt))
gdb.expect(gdb_prompt)

def test(command, pattern):
    """Sends 'command' to gdb and expects the given 'pattern'.

    If 'pattern' is None, simply consumes everything up to and including
    the gdb prompt.

    Args:
        command (string): The command we'll send to gdb.
        pattern (raw string): A pattern the gdb output should match.

    Returns:
        string: The string that matched 'pattern', or an empty string if
            'pattern' was None.
    """

    match = ''

    gdb.sendline(command)

    if pattern:
        # PExpect does a non-greedy match for '+' and '*'.  Since it can't look
        # ahead on the gdb output stream, if 'pattern' ends with a '+' or a '*'
        # we may end up matching only part of the required output.
        # To avoid this, we'll consume 'pattern' and anything that follows it
        # up to and including the gdb prompt, then extract 'pattern' later.
        index = gdb.expect([r'{0}.+{1}'.format(pattern, gdb_prompt),
                            pexpect.TIMEOUT])

        if index == 0:
            # gdb.after now contains the whole match.  Extract the text that
            # matches 'pattern'.
            match = re.match(pattern, gdb.after, re.DOTALL).group()
        elif index == 1:
            # We got a timeout exception.  Print information on what caused it
            # and bail out.
            error = ('Response does not match the expected pattern.\n'
                     'Command: {0}\n'
                     'Expected pattern: {1}\n'
                     'Response: {2}'.format(command, pattern, gdb.before))

            raise pexpect.TIMEOUT(error)
    else:
        # Consume just the the gdb prompt.
        gdb.expect(gdb_prompt)

    return match

def init_test(test_bin):
    """Loads the test binary file to gdb.

    Args:
        test_bin (string): The name of the test binary file.
    """

    test('file {0}'.format(test_bin), None)

def go_to_main():
    """Executes a gdb 'start' command, which takes us to main."""

    test('start', r'main')

def get_line_number(file_name, string):
    """Returns the number of the line in which 'string' appears within a file.

    Args:
        file_name (string): The name of the file we'll search through.
        string (string): The string we'll look for.

    Returns:
        int: The number of the line in which 'string' appears, starting from 1.
    """
    number = -1

    with open(file_name) as src_file:
        for i, line in enumerate(src_file):
            if string in line:
                number = i + 1
                break

    if number == -1:
        raise NoLineError(file_name, string)

    return number

def break_at(file_name, string, temporary=True, thread=None):
    """Places a breakpoint on the first line in 'file_name' containing 'string'.

    'string' is usually a comment like "Stop here".  Notice this may fail unless
    the comment is placed inline next to actual code, e.g.:

        ...
        /* Stop here */
        ...

    may fail, while:

        ...
        some_func(); /* Stop here */
        ...

    will succeed.

    If 'thread' isn't None, the breakpoint will be set for all the threads.
    Otherwise, it'll be set only for 'thread'.

    Args:
        file_name (string): The name of the file we'll place the breakpoint in.
        string (string): A string we'll look for inside the file.
            We'll place a breakpoint on the line which contains it.
        temporary (bool): Whether the breakpoint should be automatically deleted
            after we reach it.
        thread (int): The number of the thread we'll place the breakpoint for,
            as seen by gdb.  If specified, it should be greater than zero.
    """

    if not thread:
        thread_str = ''
    else:
        thread_str = 'thread {0}'.format(thread)

    if temporary:
        command = 'tbreak'
        break_type = 'Temporary breakpoint'
    else:
        command = 'break'
        break_type = 'Breakpoint'

    line_number = str(get_line_number(file_name, string))

    test('{0} {1}:{2} {3}'.format(command, file_name, line_number, thread_str),
         r'{0} [0-9]+ at 0x[a-f0-9]+: file {1}, line {2}\.'.format(break_type,
                                                                   file_name,
                                                                   line_number))

def continue_cmd(thread=None):
    """Executes a gdb 'continue' command.

    If 'thread' isn't None, the command will be applied to all the threads.
    Otherwise, it'll be applied only to 'thread'.

    Args:
        thread (int): The number of the thread we'll apply the command to,
            as seen by gdb.  If specified, it should be greater than zero.
    """

    if not thread:
        command = 'continue'
    else:
        command = 'thread apply {0} continue'.format(thread)

    test(command, None)

def next_cmd(count=1, thread=None):
    """Executes a gdb 'next' command.

    If 'thread' isn't None, the command will be applied to all the threads.
    Otherwise, it'll be applied only to 'thread'.

    Args:
        count (int): The 'count' argument of the 'next' command.
        thread (int): The number of the thread we'll apply the command to,
            as seen by gdb.  If specified, it should be greater than zero.
    """

    if not thread:
        command = 'next'
    else:
        command = 'thread apply {0} next'

    test('{0} {1}'.format(command, count), None)

def select_thread(thread):
    """Selects the thread indicated by 'thread'.

    Args:
        thread (int): The number of the thread we'll switch to, as seen by gdb.
            This should be greater than zero.
    """

    if thread > 0:
        test('thread {0}'.format(thread), None)

def get_current_thread_lwpid():
    """Gets the current thread's Lightweight Process ID.

    Returns:
        string: The current thread's LWP ID.
    """

    # It's easier to get the LWP ID through the Python API than the gdb CLI.
    command = 'python print(gdb.selected_thread().ptid[1])'

    return test(command, r'[0-9]+')

def set_scheduler_locking(mode):
    """Executes the gdb 'set scheduler-locking' command.

    Args:
        mode (bool): Whether the scheduler locking mode should be 'on'.
    """
    modes = {
        True: 'on',
        False: 'off'
    }

    test('set scheduler-locking {0}'.format(modes[mode]), None)

def test_printer(var, to_string, children=None, is_ptr=True):
    """ Tests the output of a pretty printer.

    For a variable called 'var', this tests whether its associated printer
    outputs the expected 'to_string' and children (if any).

    Args:
        var (string): The name of the variable we'll print.
        to_string (raw string): The expected output of the printer's 'to_string'
            method.
        children (map {raw string->raw string}): A map with the expected output
            of the printer's children' method.
        is_ptr (bool): Whether 'var' is a pointer, and thus should be
            dereferenced.
    """

    if is_ptr:
        var = '*{0}'.format(var)

    test('print {0}'.format(var), to_string)

    if children:
        for name, value in children.items():
            # Children are shown as 'name = value'.
            test('print {0}'.format(var), r'{0} = {1}'.format(name, value))
