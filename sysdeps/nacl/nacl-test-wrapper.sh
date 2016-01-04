#!/bin/bash
# test-wrapper script for NaCl.

# Copyright (C) 2015-2016 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

progname="$(basename "$0")"

usage="usage: ${progname} --arch=ARCH [VAR=VAL...] COMMAND ..."
help="
"

use_bootstrap=true
arch=
env=()
envi=0
while [ $# -gt 0 ]; do
  case "$1" in

    --help)
      echo "$usage"
      echo "$help"
      exit 0
      ;;

    --arch=*)
      arch="${1#--arch=}"
      shift
      ;;

    *=*)
      env[envi++]='-E'
      env[envi++]="$1"
      shift
      ;;

    --)
      shift
      break
      ;;

    *)
      break
      ;;
  esac
done

if [ $# -lt 1 -o -z "$arch" ]; then
  echo "$usage" >&2
  echo "Type '${progname} --help' for more detailed help." >&2
  exit 1
fi

test_args=("$@")

if [ -z "$NACL_SDK_ROOT" ]; then
  echo >&2 "$0: NACL_SDK_ROOT must be set in the environment"
  exit 77
fi

# We use a handful of things from the NaCl SDK, or at least
# from a directory matching the layout of the NaCl SDK.
sdk_tools="${NACL_SDK_ROOT}/tools"

NACL_BOOTSTRAP="${sdk_tools}/nacl_helper_bootstrap_${arch}"
NACL_SEL_LDR="${sdk_tools}/sel_ldr_${arch}"
NACL_IRT="${sdk_tools}/irt_core_${arch}.nexe"
NACL_LOADER="${sdk_tools}/elf_loader_${arch}.nexe"

if [ ! -x "$NACL_BOOTSTRAP" -o ! -x "$NACL_SEL_LDR" ]; then
  echo >&2 "$0: sel_ldr_${arch} and/or nacl_helper_bootstrap_${arch} missing"
  echo >&2 "$0: from directory $sdk_tools"
  exit 77
fi

if [ ! -r "$NACL_IRT" -o ! -r "$NACL_LOADER" ]; then
  echo >&2 "$0: irt_core_${arch}.nexe and/or loader_${arch}.nexe missing"
  echo >&2 "$0: from directory $sdk_tools"
  exit 77
fi

# Figure out if we are building for the native machine or not.
# If not, we'll run sel_ldr under qemu.
decide_use_emulator()
{
  local arg
  for arg; do
    if [[ "$(uname -m)" = "$1" ]]; then
      return
    fi
  done
  use_emulator=true
}

use_emulator=false
case "$arch" in
arm)
  decide_use_emulator 'arm*'
  emulator=(qemu-arm -cpu cortex-a15 -L "${sdk_tools}/arm_trusted")
  ;;
x86_32)
  decide_use_emulator 'i?86' 'x86_64*'
  emulator=(qemu-i386)
  ;;
x86_64)
  decide_use_emulator 'x86_64*'
  emulator=(qemu-x86_64)
  ;;
esac

if $use_emulator; then
  ldr_args=('-Q')
  emulator_factor=10
else
  emulator=()
  ldr_args=()
  emulator_factor=1
fi

if $use_bootstrap; then
  ldr=(
    "${NACL_BOOTSTRAP}"
    "${NACL_SEL_LDR}"
    '--r_debug=0xXXXXXXXXXXXXXXXX'
    '--reserved_at_zero=0xXXXXXXXXXXXXXXXX'
  )
else
  ldr=("${NACL_SEL_LDR}")
fi

static=true
case "$1" in
*/ld-nacl*) static=false ;;
esac

if $static; then
  loader=()
else
  loader=(-f "${NACL_LOADER}")
fi

run_test()
{
  local test_fifo="$1"
  local cmd=(
    "${emulator[@]}" "${ldr[@]}" -q -S -a "${ldr_args[@]}" -B "${NACL_IRT}"
    "${loader[@]}" "${env[@]}" -E TEST_DIRECT="$test_fifo" -- "${test_args[@]}"
  )
  if [ "${NACLVERBOSITY:+set}" = set ]; then
    "${cmd[@]}"
  else
    NACLLOG=/dev/null "${cmd[@]}"
  fi
}

temp_files=()
test_fifo=
do_cleanup()
{
  rm -rf "$test_fifo" "${temp_files[@]}"
}
trap do_cleanup EXIT HUP INT TERM

# Create a named pipe to receive the TEST_DIRECT information from the test
# program.
test_fifo=${TMPDIR:-/tmp}/libc-test-fifo.$$
rm -f "$test_fifo"
mkfifo "$test_fifo" || {
  echo "Cannot create test FIFO '$test_fifo'"
  exit 1
}

# Run the test in the background, so we can implement a timeout.
# The no-op redirection defeats the default behavior of "< /dev/null"
# for a background command.
run_test "$test_fifo" <&0 & test_pid=$!

# Set up a short timeout before we read from the FIFO, in case
# the program doesn't actually write to the FIFO at all (it is
# not a test-skeleton.c program, or it dies very early).
no_skeleton=false
script_pid=$$
trap 'no_skeleton=true' USR1
(sleep 2; kill -USR1 $script_pid) 2> /dev/null &

# The test should first write into the FIFO to describe its expectations.
# Our open-for-reading of the FIFO will block until the test starts up and
# opens it for writing.  Then our reads will block until the test finishes
# writing out info and closes the FIFO.  At that point we will have
# collected (and evaluated) what it emitted.  It sets these variables:
#	timeout=%u
#	timeoutfactor=%u
#	exit=%u
#	signal=%s
unset exit signal
. "$test_fifo" 2> /dev/null

# If we got this far, either the 'no_skeleton=true' watchdog already
# fired, or else we don't want it to.
trap '' USR1

if $no_skeleton; then
  # We hit the timeout, so we didn't get full information about test
  # expectations.  Reset any partial results we may have gotten.
  unset exit signal
else
  # Now we know the expected timeout, so we can start the timer running.
  ((sleep_time = timeout * timeoutfactor * emulator_factor))

  # Now start a background subshell to enforce the timeout.
  (sleep "$sleep_time"; kill -ALRM $test_pid) 2> /dev/null &
fi

# This corresponds to '#ifdef EXPECTED_STATUS' in test-skeleton.c.
expected_status()
{
  test "${exit+yes}" = yes
}
# This corresponds to '#ifdef EXPECTED_SIGNAL' in test-skeleton.c.
expected_signal()
{
  test "${signal+yes}" = yes
}
# This corresponds to 'if (WIFEXITED (status))' in test-skeleton.c.
wifexited()
{
  test $test_rc -lt 128
}

# Now wait for the test process to finish.
wait $test_pid
test_rc=$?

# This exactly duplicates the logic in test-skeleton.c.
if wifexited; then
  if ! expected_status; then
    if ! expected_signal; then
      # Simply exit with the return value of the test.  */
      exit $test_rc
    else
      echo "Expected signal '${signal}' from child, got none"
      exit 1
    fi
  else
    if [ $test_rc -ne $exit ]; then
      echo "Expected status $exit, got $test_rc"
      exit 1
    fi
    exit 0
  fi
else
  # Process was killed by timer or other signal.
  ((test_signal = test_rc > 192 ? 256 - test_rc : test_rc - 128 ))
  test_signame=$(kill -l "$test_signal")
  if ! expected_signal; then
    echo "Didn't expect signal from child; got '${test_signame}'"
    exit 1
  else
    if [ "$test_signame" != "$signal" ]; then
      echo "\
Incorrect signal from child: got '${test_signame}', need '${signal}'"
      exit 1
    fi
    exit 0
  fi
fi
