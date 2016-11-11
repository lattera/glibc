#!/usr/bin/python3
# Build many configurations of glibc.
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

"""Build many configurations of glibc.

This script takes as arguments a directory name (containing a src
subdirectory with sources of the relevant toolchain components) and a
description of what to do: 'checkout', to check out sources into that
directory, 'host-libraries', to build libraries required by the
toolchain, 'compilers', to build cross-compilers for various
configurations, or 'glibcs', to build glibc for various configurations
and run the compilation parts of the testsuite.  Subsequent arguments
name the versions of components to check out (<component>-<version),
for 'checkout', or, for actions other than 'checkout', name
configurations for which compilers or glibc are to be built.
"""

import argparse
import os
import os.path
import re
import shutil
import stat
import subprocess
import sys
import urllib.request


class Context:

    """The global state associated with builds in a given directory."""

    def __init__(self, topdir, parallelism, keep, action):
        """Initialize the context."""
        self.topdir = topdir
        self.parallelism = parallelism
        self.keep = keep
        self.srcdir = os.path.join(topdir, 'src')
        self.installdir = os.path.join(topdir, 'install')
        self.host_libraries_installdir = os.path.join(self.installdir,
                                                      'host-libraries')
        self.builddir = os.path.join(topdir, 'build')
        self.logsdir = os.path.join(topdir, 'logs')
        self.makefile = os.path.join(self.builddir, 'Makefile')
        self.wrapper = os.path.join(self.builddir, 'wrapper')
        self.save_logs = os.path.join(self.builddir, 'save-logs')
        if action != 'checkout':
            self.build_triplet = self.get_build_triplet()
            self.glibc_version = self.get_glibc_version()
        self.configs = {}
        self.glibc_configs = {}
        self.makefile_pieces = ['.PHONY: all\n']
        self.add_all_configs()

    def get_build_triplet(self):
        """Determine the build triplet with config.guess."""
        config_guess = os.path.join(self.component_srcdir('gcc'),
                                    'config.guess')
        cg_out = subprocess.run([config_guess], stdout=subprocess.PIPE,
                                check=True, universal_newlines=True).stdout
        return cg_out.rstrip()

    def get_glibc_version(self):
        """Determine the glibc version number (major.minor)."""
        version_h = os.path.join(self.component_srcdir('glibc'), 'version.h')
        with open(version_h, 'r') as f:
            lines = f.readlines()
        starttext = '#define VERSION "'
        for l in lines:
            if l.startswith(starttext):
                l = l[len(starttext):]
                l = l.rstrip('"\n')
                m = re.fullmatch('([0-9]+)\.([0-9]+)[.0-9]*', l)
                return '%s.%s' % m.group(1, 2)
        print('error: could not determine glibc version')
        exit(1)

    def add_all_configs(self):
        """Add all known glibc build configurations."""
        self.add_config(arch='aarch64',
                        os_name='linux-gnu')
        self.add_config(arch='aarch64_be',
                        os_name='linux-gnu')
        self.add_config(arch='alpha',
                        os_name='linux-gnu')
        self.add_config(arch='arm',
                        os_name='linux-gnueabi')
        self.add_config(arch='armeb',
                        os_name='linux-gnueabi')
        self.add_config(arch='armeb',
                        os_name='linux-gnueabi',
                        variant='be8',
                        gcc_cfg=['--with-arch=armv7-a'])
        self.add_config(arch='arm',
                        os_name='linux-gnueabihf')
        self.add_config(arch='armeb',
                        os_name='linux-gnueabihf')
        self.add_config(arch='armeb',
                        os_name='linux-gnueabihf',
                        variant='be8',
                        gcc_cfg=['--with-arch=armv7-a'])
        self.add_config(arch='hppa',
                        os_name='linux-gnu')
        self.add_config(arch='ia64',
                        os_name='linux-gnu',
                        first_gcc_cfg=['--with-system-libunwind'])
        self.add_config(arch='m68k',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib'])
        self.add_config(arch='m68k',
                        os_name='linux-gnu',
                        variant='coldfire',
                        gcc_cfg=['--with-arch=cf', '--disable-multilib'])
        self.add_config(arch='microblaze',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib'])
        self.add_config(arch='microblazeel',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib'])
        self.add_config(arch='mips64',
                        os_name='linux-gnu',
                        gcc_cfg=['--with-mips-plt'],
                        glibcs=[{'variant': 'n32'},
                                {'arch': 'mips',
                                 'ccopts': '-mabi=32'},
                                {'variant': 'n64',
                                 'ccopts': '-mabi=64'}])
        self.add_config(arch='mips64',
                        os_name='linux-gnu',
                        variant='soft',
                        gcc_cfg=['--with-mips-plt', '--with-float=soft'],
                        glibcs=[{'variant': 'n32-soft',
                                 'cfg': ['--without-fp']},
                                {'variant': 'soft',
                                 'arch': 'mips',
                                 'ccopts': '-mabi=32',
                                 'cfg': ['--without-fp']},
                                {'variant': 'n64-soft',
                                 'ccopts': '-mabi=64',
                                 'cfg': ['--without-fp']}])
        self.add_config(arch='mips64',
                        os_name='linux-gnu',
                        variant='nan2008',
                        gcc_cfg=['--with-mips-plt', '--with-nan=2008',
                                 '--with-arch-64=mips64r2',
                                 '--with-arch-32=mips32r2'],
                        glibcs=[{'variant': 'n32-nan2008'},
                                {'variant': 'nan2008',
                                 'arch': 'mips',
                                 'ccopts': '-mabi=32'},
                                {'variant': 'n64-nan2008',
                                 'ccopts': '-mabi=64'}])
        self.add_config(arch='mips64',
                        os_name='linux-gnu',
                        variant='nan2008-soft',
                        gcc_cfg=['--with-mips-plt', '--with-nan=2008',
                                 '--with-arch-64=mips64r2',
                                 '--with-arch-32=mips32r2',
                                 '--with-float=soft'],
                        glibcs=[{'variant': 'n32-nan2008-soft',
                                 'cfg': ['--without-fp']},
                                {'variant': 'nan2008-soft',
                                 'arch': 'mips',
                                 'ccopts': '-mabi=32',
                                 'cfg': ['--without-fp']},
                                {'variant': 'n64-nan2008-soft',
                                 'ccopts': '-mabi=64',
                                 'cfg': ['--without-fp']}])
        self.add_config(arch='mips64el',
                        os_name='linux-gnu',
                        gcc_cfg=['--with-mips-plt'],
                        glibcs=[{'variant': 'n32'},
                                {'arch': 'mipsel',
                                 'ccopts': '-mabi=32'},
                                {'variant': 'n64',
                                 'ccopts': '-mabi=64'}])
        self.add_config(arch='mips64el',
                        os_name='linux-gnu',
                        variant='soft',
                        gcc_cfg=['--with-mips-plt', '--with-float=soft'],
                        glibcs=[{'variant': 'n32-soft',
                                 'cfg': ['--without-fp']},
                                {'variant': 'soft',
                                 'arch': 'mipsel',
                                 'ccopts': '-mabi=32',
                                 'cfg': ['--without-fp']},
                                {'variant': 'n64-soft',
                                 'ccopts': '-mabi=64',
                                 'cfg': ['--without-fp']}])
        self.add_config(arch='mips64el',
                        os_name='linux-gnu',
                        variant='nan2008',
                        gcc_cfg=['--with-mips-plt', '--with-nan=2008',
                                 '--with-arch-64=mips64r2',
                                 '--with-arch-32=mips32r2'],
                        glibcs=[{'variant': 'n32-nan2008'},
                                {'variant': 'nan2008',
                                 'arch': 'mipsel',
                                 'ccopts': '-mabi=32'},
                                {'variant': 'n64-nan2008',
                                 'ccopts': '-mabi=64'}])
        self.add_config(arch='mips64el',
                        os_name='linux-gnu',
                        variant='nan2008-soft',
                        gcc_cfg=['--with-mips-plt', '--with-nan=2008',
                                 '--with-arch-64=mips64r2',
                                 '--with-arch-32=mips32r2',
                                 '--with-float=soft'],
                        glibcs=[{'variant': 'n32-nan2008-soft',
                                 'cfg': ['--without-fp']},
                                {'variant': 'nan2008-soft',
                                 'arch': 'mipsel',
                                 'ccopts': '-mabi=32',
                                 'cfg': ['--without-fp']},
                                {'variant': 'n64-nan2008-soft',
                                 'ccopts': '-mabi=64',
                                 'cfg': ['--without-fp']}])
        self.add_config(arch='nios2',
                        os_name='linux-gnu')
        self.add_config(arch='powerpc',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib', '--enable-secureplt'])
        self.add_config(arch='powerpc',
                        os_name='linux-gnu',
                        variant='soft',
                        gcc_cfg=['--disable-multilib', '--with-float=soft',
                                 '--enable-secureplt'],
                        glibcs=[{'variant': 'soft', 'cfg': ['--without-fp']}])
        self.add_config(arch='powerpc64',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib', '--enable-secureplt'])
        self.add_config(arch='powerpc64le',
                        os_name='linux-gnu',
                        gcc_cfg=['--disable-multilib', '--enable-secureplt'])
        self.add_config(arch='powerpc',
                        os_name='linux-gnuspe',
                        gcc_cfg=['--disable-multilib', '--enable-secureplt',
                                 '--enable-e500-double'],
                        glibcs=[{'cfg': ['--without-fp']}])
        self.add_config(arch='powerpc',
                        os_name='linux-gnuspe',
                        variant='e500v1',
                        gcc_cfg=['--disable-multilib', '--enable-secureplt'],
                        glibcs=[{'variant': 'e500v1', 'cfg': ['--without-fp']}])
        self.add_config(arch='s390x',
                        os_name='linux-gnu',
                        glibcs=[{},
                                {'arch': 's390', 'ccopts': '-m31'}])
        # SH is missing __builtin_trap support, so work around this;
        # see <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70216>.
        no_isolate = ('-fno-isolate-erroneous-paths-dereference'
                      ' -fno-isolate-erroneous-paths-attribute')
        self.add_config(arch='sh3',
                        os_name='linux-gnu',
                        glibcs=[{'ccopts': no_isolate}])
        self.add_config(arch='sh3eb',
                        os_name='linux-gnu',
                        glibcs=[{'ccopts': no_isolate}])
        self.add_config(arch='sh4',
                        os_name='linux-gnu',
                        glibcs=[{'ccopts': no_isolate}])
        self.add_config(arch='sh4eb',
                        os_name='linux-gnu',
                        glibcs=[{'ccopts': no_isolate}])
        self.add_config(arch='sh4',
                        os_name='linux-gnu',
                        variant='soft',
                        gcc_cfg=['--without-fp'],
                        glibcs=[{'variant': 'soft',
                                 'cfg': ['--without-fp'],
                                 'ccopts': no_isolate}])
        self.add_config(arch='sh4eb',
                        os_name='linux-gnu',
                        variant='soft',
                        gcc_cfg=['--without-fp'],
                        glibcs=[{'variant': 'soft',
                                 'cfg': ['--without-fp'],
                                 'ccopts': no_isolate}])
        self.add_config(arch='sparc64',
                        os_name='linux-gnu',
                        glibcs=[{},
                                {'arch': 'sparcv9',
                                 'ccopts': '-m32 -mlong-double-128'}])
        self.add_config(arch='tilegx',
                        os_name='linux-gnu',
                        glibcs=[{},
                                {'variant': '32', 'ccopts': '-m32'}])
        self.add_config(arch='tilegxbe',
                        os_name='linux-gnu',
                        glibcs=[{},
                                {'variant': '32', 'ccopts': '-m32'}])
        self.add_config(arch='tilepro',
                        os_name='linux-gnu')
        self.add_config(arch='x86_64',
                        os_name='linux-gnu',
                        gcc_cfg=['--with-multilib-list=m64,m32,mx32'],
                        glibcs=[{},
                                {'variant': 'x32', 'ccopts': '-mx32'},
                                {'arch': 'i686', 'ccopts': '-m32 -march=i686'}],
                        extra_glibcs=[{'variant': 'disable-multi-arch',
                                       'cfg': ['--disable-multi-arch']},
                                      {'variant': 'disable-multi-arch',
                                       'arch': 'i686',
                                       'ccopts': '-m32 -march=i686',
                                       'cfg': ['--disable-multi-arch']},
                                      {'arch': 'i486',
                                       'ccopts': '-m32 -march=i486'},
                                      {'arch': 'i586',
                                       'ccopts': '-m32 -march=i586'}])

    def add_config(self, **args):
        """Add an individual build configuration."""
        cfg = Config(self, **args)
        if cfg.name in self.configs:
            print('error: duplicate config %s' % cfg.name)
            exit(1)
        self.configs[cfg.name] = cfg
        for c in cfg.all_glibcs:
            if c.name in self.glibc_configs:
                print('error: duplicate glibc config %s' % c.name)
                exit(1)
            self.glibc_configs[c.name] = c

    def component_srcdir(self, component):
        """Return the source directory for a given component, e.g. gcc."""
        return os.path.join(self.srcdir, component)

    def component_builddir(self, action, config, component, subconfig=None):
        """Return the directory to use for a build."""
        if config is None:
            # Host libraries.
            assert subconfig is None
            return os.path.join(self.builddir, action, component)
        if subconfig is None:
            return os.path.join(self.builddir, action, config, component)
        else:
            # glibc build as part of compiler build.
            return os.path.join(self.builddir, action, config, component,
                                subconfig)

    def compiler_installdir(self, config):
        """Return the directory in which to install a compiler."""
        return os.path.join(self.installdir, 'compilers', config)

    def compiler_bindir(self, config):
        """Return the directory in which to find compiler binaries."""
        return os.path.join(self.compiler_installdir(config), 'bin')

    def compiler_sysroot(self, config):
        """Return the sysroot directory for a compiler."""
        return os.path.join(self.compiler_installdir(config), 'sysroot')

    def glibc_installdir(self, config):
        """Return the directory in which to install glibc."""
        return os.path.join(self.installdir, 'glibcs', config)

    def run_builds(self, action, configs):
        """Run the requested builds."""
        if action == 'checkout':
            self.checkout(configs)
            return
        elif action == 'host-libraries':
            if configs:
                print('error: configurations specified for host-libraries')
                exit(1)
            self.build_host_libraries()
        elif action == 'compilers':
            self.build_compilers(configs)
        else:
            self.build_glibcs(configs)
        self.write_files()
        self.do_build()

    @staticmethod
    def remove_dirs(*args):
        """Remove directories and their contents if they exist."""
        for dir in args:
            shutil.rmtree(dir, ignore_errors=True)

    @staticmethod
    def remove_recreate_dirs(*args):
        """Remove directories if they exist, and create them as empty."""
        Context.remove_dirs(*args)
        for dir in args:
            os.makedirs(dir, exist_ok=True)

    def add_makefile_cmdlist(self, target, cmdlist, logsdir):
        """Add makefile text for a list of commands."""
        commands = cmdlist.makefile_commands(self.wrapper, logsdir)
        self.makefile_pieces.append('all: %s\n.PHONY: %s\n%s:\n%s\n' %
                                    (target, target, target, commands))

    def write_files(self):
        """Write out the Makefile and wrapper script."""
        mftext = ''.join(self.makefile_pieces)
        with open(self.makefile, 'w') as f:
            f.write(mftext)
        wrapper_text = (
            '#!/bin/sh\n'
            'prev_base=$1\n'
            'this_base=$2\n'
            'desc=$3\n'
            'dir=$4\n'
            'path=$5\n'
            'shift 5\n'
            'prev_status=$prev_base-status.txt\n'
            'this_status=$this_base-status.txt\n'
            'this_log=$this_base-log.txt\n'
            'date > "$this_log"\n'
            'echo >> "$this_log"\n'
            'echo "Description: $desc" >> "$this_log"\n'
            'echo "Command: $*" >> "$this_log"\n'
            'echo "Directory: $dir" >> "$this_log"\n'
            'echo "Path addition: $path" >> "$this_log"\n'
            'echo >> "$this_log"\n'
            'record_status ()\n'
            '{\n'
            '  echo >> "$this_log"\n'
            '  echo "$1: $desc" > "$this_status"\n'
            '  echo "$1: $desc" >> "$this_log"\n'
            '  echo >> "$this_log"\n'
            '  date >> "$this_log"\n'
            '  echo "$1: $desc"\n'
            '  exit 0\n'
            '}\n'
            'check_error ()\n'
            '{\n'
            '  if [ "$1" != "0" ]; then\n'
            '    record_status FAIL\n'
            '  fi\n'
            '}\n'
            'if [ "$prev_base" ] && ! grep -q "^PASS" "$prev_status"; then\n'
            '    record_status UNRESOLVED\n'
            'fi\n'
            'if [ "$dir" ]; then\n'
            '  cd "$dir"\n'
            '  check_error "$?"\n'
            'fi\n'
            'if [ "$path" ]; then\n'
            '  PATH=$path:$PATH\n'
            'fi\n'
            '"$@" < /dev/null >> "$this_log" 2>&1\n'
            'check_error "$?"\n'
            'record_status PASS\n')
        with open(self.wrapper, 'w') as f:
            f.write(wrapper_text)
        os.chmod(self.wrapper,
                 (stat.S_IRWXU|stat.S_IRGRP|stat.S_IXGRP|
                  stat.S_IROTH|stat.S_IXOTH))
        save_logs_text = (
            '#!/bin/sh\n'
            'if ! [ -f tests.sum ]; then\n'
            '  echo "No test summary available."\n'
            '  exit 0\n'
            'fi\n'
            'save_file ()\n'
            '{\n'
            '  echo "Contents of $1:"\n'
            '  echo\n'
            '  cat "$1"\n'
            '  echo\n'
            '  echo "End of contents of $1."\n'
            '  echo\n'
            '}\n'
            'save_file tests.sum\n'
            'non_pass_tests=$(grep -v "^PASS: " tests.sum | sed -e "s/^PASS: //")\n'
            'for t in $non_pass_tests; do\n'
            '  if [ -f "$t.out" ]; then\n'
            '    save_file "$t.out"\n'
            '  fi\n'
            'done\n')
        with open(self.save_logs, 'w') as f:
            f.write(save_logs_text)
        os.chmod(self.save_logs,
                 (stat.S_IRWXU|stat.S_IRGRP|stat.S_IXGRP|
                  stat.S_IROTH|stat.S_IXOTH))

    def do_build(self):
        """Do the actual build."""
        cmd = ['make', '-j%d' % self.parallelism]
        subprocess.run(cmd, cwd=self.builddir, check=True)

    def build_host_libraries(self):
        """Build the host libraries."""
        installdir = self.host_libraries_installdir
        builddir = os.path.join(self.builddir, 'host-libraries')
        logsdir = os.path.join(self.logsdir, 'host-libraries')
        self.remove_recreate_dirs(installdir, builddir, logsdir)
        cmdlist = CommandList('host-libraries', self.keep)
        self.build_host_library(cmdlist, 'gmp')
        self.build_host_library(cmdlist, 'mpfr',
                                ['--with-gmp=%s' % installdir])
        self.build_host_library(cmdlist, 'mpc',
                                ['--with-gmp=%s' % installdir,
                                '--with-mpfr=%s' % installdir])
        cmdlist.add_command('done', ['touch', os.path.join(installdir, 'ok')])
        self.add_makefile_cmdlist('host-libraries', cmdlist, logsdir)

    def build_host_library(self, cmdlist, lib, extra_opts=None):
        """Build one host library."""
        srcdir = self.component_srcdir(lib)
        builddir = self.component_builddir('host-libraries', None, lib)
        installdir = self.host_libraries_installdir
        cmdlist.push_subdesc(lib)
        cmdlist.create_use_dir(builddir)
        cfg_cmd = [os.path.join(srcdir, 'configure'),
                   '--prefix=%s' % installdir,
                   '--disable-shared']
        if extra_opts:
            cfg_cmd.extend (extra_opts)
        cmdlist.add_command('configure', cfg_cmd)
        cmdlist.add_command('build', ['make'])
        cmdlist.add_command('check', ['make', 'check'])
        cmdlist.add_command('install', ['make', 'install'])
        cmdlist.cleanup_dir()
        cmdlist.pop_subdesc()

    def build_compilers(self, configs):
        """Build the compilers."""
        if not configs:
            self.remove_dirs(os.path.join(self.builddir, 'compilers'))
            self.remove_dirs(os.path.join(self.installdir, 'compilers'))
            self.remove_dirs(os.path.join(self.logsdir, 'compilers'))
            configs = sorted(self.configs.keys())
        for c in configs:
            self.configs[c].build()

    def build_glibcs(self, configs):
        """Build the glibcs."""
        if not configs:
            self.remove_dirs(os.path.join(self.builddir, 'glibcs'))
            self.remove_dirs(os.path.join(self.installdir, 'glibcs'))
            self.remove_dirs(os.path.join(self.logsdir, 'glibcs'))
            configs = sorted(self.glibc_configs.keys())
        for c in configs:
            self.glibc_configs[c].build()

    def checkout(self, versions):
        """Check out the desired component versions."""
        default_versions = {'binutils': 'vcs-2.27',
                            'gcc': 'vcs-6',
                            'glibc': 'vcs-mainline',
                            'gmp': '6.1.1',
                            'linux': '4.8.6',
                            'mpc': '1.0.3',
                            'mpfr': '3.1.5'}
        use_versions = {}
        for v in versions:
            found_v = False
            for k in default_versions.keys():
                kx = k + '-'
                if v.startswith(kx):
                    vx = v[len(kx):]
                    if k in use_versions:
                        print('error: multiple versions for %s' % k)
                        exit(1)
                    use_versions[k] = vx
                    found_v = True
                    break
            if not found_v:
                print('error: unknown component in %s' % v)
                exit(1)
        for k in default_versions.keys():
            if k not in use_versions:
                use_versions[k] = default_versions[k]
        os.makedirs(self.srcdir, exist_ok=True)
        for k in sorted(default_versions.keys()):
            update = os.access(self.component_srcdir(k), os.F_OK)
            v = use_versions[k]
            if v.startswith('vcs-'):
                self.checkout_vcs(k, v[4:], update)
            else:
                self.checkout_tar(k, v, update)

    def checkout_vcs(self, component, version, update):
        """Check out the given version of the given component from version
        control."""
        if component == 'binutils':
            git_url = 'git://sourceware.org/git/binutils-gdb.git'
            if version == 'mainline':
                git_branch = 'master'
            else:
                trans = str.maketrans({'.': '_'})
                git_branch = 'binutils-%s-branch' % version.translate(trans)
            self.git_checkout(component, git_url, git_branch, update)
        elif component == 'gcc':
            if version == 'mainline':
                branch = 'trunk'
            else:
                trans = str.maketrans({'.': '_'})
                branch = 'branches/gcc-%s-branch' % version.translate(trans)
            svn_url = 'svn://gcc.gnu.org/svn/gcc/%s' % branch
            self.gcc_checkout(svn_url, update)
        elif component == 'glibc':
            git_url = 'git://sourceware.org/git/glibc.git'
            if version == 'mainline':
                git_branch = 'master'
            else:
                git_branch = 'release/%s/master' % version
            self.git_checkout(component, git_url, git_branch, update)
            self.fix_glibc_timestamps()
        else:
            print('error: component %s coming from VCS' % component)
            exit(1)

    def git_checkout(self, component, git_url, git_branch, update):
        """Check out a component from git."""
        if update:
            subprocess.run(['git', 'remote', 'prune', 'origin'],
                           cwd=self.component_srcdir(component), check=True)
            subprocess.run(['git', 'pull', '-q'],
                           cwd=self.component_srcdir(component), check=True)
        else:
            subprocess.run(['git', 'clone', '-q', '-b', git_branch, git_url,
                            self.component_srcdir(component)], check=True)

    def fix_glibc_timestamps(self):
        """Fix timestamps in a glibc checkout."""
        # Ensure that builds do not try to regenerate generated files
        # in the source tree.
        srcdir = self.component_srcdir('glibc')
        for dirpath, dirnames, filenames in os.walk(srcdir):
            for f in filenames:
                if (f == 'configure' or
                    f == 'preconfigure' or
                    f.endswith('-kw.h')):
                    to_touch = os.path.join(dirpath, f)
                    subprocess.run(['touch', to_touch], check=True)

    def gcc_checkout(self, svn_url, update):
        """Check out GCC from SVN."""
        if not update:
            subprocess.run(['svn', 'co', '-q', svn_url,
                            self.component_srcdir('gcc')], check=True)
        subprocess.run(['contrib/gcc_update', '--silent'],
                       cwd=self.component_srcdir('gcc'), check=True)

    def checkout_tar(self, component, version, update):
        """Check out the given version of the given component from a
        tarball."""
        if update:
            return
        url_map = {'binutils': 'https://ftp.gnu.org/gnu/binutils/binutils-%(version)s.tar.bz2',
                   'gcc': 'https://ftp.gnu.org/gnu/gcc/gcc-%(version)s/gcc-%(version)s.tar.bz2',
                   'gmp': 'https://ftp.gnu.org/gnu/gmp/gmp-%(version)s.tar.xz',
                   'linux': 'https://www.kernel.org/pub/linux/kernel/v4.x/linux-%(version)s.tar.xz',
                   'mpc': 'https://ftp.gnu.org/gnu/mpc/mpc-%(version)s.tar.gz',
                   'mpfr': 'https://ftp.gnu.org/gnu/mpfr/mpfr-%(version)s.tar.xz'}
        if component not in url_map:
            print('error: component %s coming from tarball' % component)
            exit(1)
        url = url_map[component] % {'version': version}
        filename = os.path.join(self.srcdir, url.split('/')[-1])
        response = urllib.request.urlopen(url)
        data = response.read()
        with open(filename, 'wb') as f:
            f.write(data)
        subprocess.run(['tar', '-C', self.srcdir, '-x', '-f', filename],
                       check=True)
        os.rename(os.path.join(self.srcdir, '%s-%s' % (component, version)),
                  self.component_srcdir(component))
        os.remove(filename)


class Config:

    """A configuration for building a compiler and associated libraries."""

    def __init__(self, ctx, arch, os_name, variant=None, gcc_cfg=None,
                 first_gcc_cfg=None, glibcs=None, extra_glibcs=None):
        """Initialize a Config object."""
        self.ctx = ctx
        self.arch = arch
        self.os = os_name
        self.variant = variant
        if variant is None:
            self.name = '%s-%s' % (arch, os_name)
        else:
            self.name = '%s-%s-%s' % (arch, os_name, variant)
        self.triplet = '%s-glibc-%s' % (arch, os_name)
        if gcc_cfg is None:
            self.gcc_cfg = []
        else:
            self.gcc_cfg = gcc_cfg
        if first_gcc_cfg is None:
            self.first_gcc_cfg = []
        else:
            self.first_gcc_cfg = first_gcc_cfg
        if glibcs is None:
            glibcs = [{'variant': variant}]
        if extra_glibcs is None:
            extra_glibcs = []
        glibcs = [Glibc(self, **g) for g in glibcs]
        extra_glibcs = [Glibc(self, **g) for g in extra_glibcs]
        self.all_glibcs = glibcs + extra_glibcs
        self.compiler_glibcs = glibcs
        self.installdir = ctx.compiler_installdir(self.name)
        self.bindir = ctx.compiler_bindir(self.name)
        self.sysroot = ctx.compiler_sysroot(self.name)
        self.builddir = os.path.join(ctx.builddir, 'compilers', self.name)
        self.logsdir = os.path.join(ctx.logsdir, 'compilers', self.name)

    def component_builddir(self, component):
        """Return the directory to use for a (non-glibc) build."""
        return self.ctx.component_builddir('compilers', self.name, component)

    def build(self):
        """Generate commands to build this compiler."""
        self.ctx.remove_recreate_dirs(self.installdir, self.builddir,
                                      self.logsdir)
        cmdlist = CommandList('compilers-%s' % self.name, self.ctx.keep)
        cmdlist.add_command('check-host-libraries',
                            ['test', '-f',
                             os.path.join(self.ctx.host_libraries_installdir,
                                          'ok')])
        cmdlist.use_path(self.bindir)
        self.build_cross_tool(cmdlist, 'binutils', 'binutils',
                              ['--disable-gdb',
                               '--disable-libdecnumber',
                               '--disable-readline',
                               '--disable-sim'])
        if self.os.startswith('linux'):
            self.install_linux_headers(cmdlist)
        self.build_gcc(cmdlist, True)
        for g in self.compiler_glibcs:
            cmdlist.push_subdesc('glibc')
            cmdlist.push_subdesc(g.name)
            g.build_glibc(cmdlist, True)
            cmdlist.pop_subdesc()
            cmdlist.pop_subdesc()
        self.build_gcc(cmdlist, False)
        cmdlist.add_command('done', ['touch',
                                     os.path.join(self.installdir, 'ok')])
        self.ctx.add_makefile_cmdlist('compilers-%s' % self.name, cmdlist,
                                      self.logsdir)

    def build_cross_tool(self, cmdlist, tool_src, tool_build, extra_opts=None):
        """Build one cross tool."""
        srcdir = self.ctx.component_srcdir(tool_src)
        builddir = self.component_builddir(tool_build)
        cmdlist.push_subdesc(tool_build)
        cmdlist.create_use_dir(builddir)
        cfg_cmd = [os.path.join(srcdir, 'configure'),
                   '--prefix=%s' % self.installdir,
                   '--build=%s' % self.ctx.build_triplet,
                   '--host=%s' % self.ctx.build_triplet,
                   '--target=%s' % self.triplet,
                   '--with-sysroot=%s' % self.sysroot]
        if extra_opts:
            cfg_cmd.extend(extra_opts)
        cmdlist.add_command('configure', cfg_cmd)
        cmdlist.add_command('build', ['make'])
        cmdlist.add_command('install', ['make', 'install'])
        cmdlist.cleanup_dir()
        cmdlist.pop_subdesc()

    def install_linux_headers(self, cmdlist):
        """Install Linux kernel headers."""
        arch_map = {'aarch64': 'arm64',
                    'alpha': 'alpha',
                    'arm': 'arm',
                    'hppa': 'parisc',
                    'i486': 'x86',
                    'i586': 'x86',
                    'i686': 'x86',
                    'i786': 'x86',
                    'ia64': 'ia64',
                    'm68k': 'm68k',
                    'microblaze': 'microblaze',
                    'mips': 'mips',
                    'nios2': 'nios2',
                    'powerpc': 'powerpc',
                    's390': 's390',
                    'sh': 'sh',
                    'sparc': 'sparc',
                    'tile': 'tile',
                    'x86_64': 'x86'}
        linux_arch = None
        for k in arch_map:
            if self.arch.startswith(k):
                linux_arch = arch_map[k]
                break
        assert linux_arch is not None
        srcdir = self.ctx.component_srcdir('linux')
        builddir = self.component_builddir('linux')
        headers_dir = os.path.join(self.sysroot, 'usr')
        cmdlist.push_subdesc('linux')
        cmdlist.create_use_dir(builddir)
        cmdlist.add_command('install-headers',
                            ['make', '-C', srcdir, 'O=%s' % builddir,
                             'ARCH=%s' % linux_arch,
                             'INSTALL_HDR_PATH=%s' % headers_dir,
                             'headers_install'])
        cmdlist.cleanup_dir()
        cmdlist.pop_subdesc()

    def build_gcc(self, cmdlist, bootstrap):
        """Build GCC."""
        # libsanitizer commonly breaks because of glibc header
        # changes, or on unusual targets.  libssp is of little
        # relevance with glibc's own stack checking support.
        cfg_opts = list(self.gcc_cfg)
        cfg_opts += ['--disable-libsanitizer', '--disable-libssp']
        if bootstrap:
            tool_build = 'gcc-first'
            # Building a static-only, C-only compiler that is
            # sufficient to build glibc.  Various libraries and
            # features that may require libc headers must be disabled.
            # When configuring with a sysroot, --with-newlib is
            # required to define inhibit_libc (to stop some parts of
            # libgcc including libc headers); --without-headers is not
            # sufficient.
            cfg_opts += ['--enable-languages=c', '--disable-shared',
                         '--disable-threads',
                         '--disable-libatomic',
                         '--disable-decimal-float',
                         '--disable-libffi',
                         '--disable-libgomp',
                         '--disable-libitm',
                         '--disable-libmpx',
                         '--disable-libquadmath',
                         '--without-headers', '--with-newlib',
                         '--with-glibc-version=%s' % self.ctx.glibc_version
                         ]
            cfg_opts += self.first_gcc_cfg
        else:
            tool_build = 'gcc'
            cfg_opts += ['--enable-languages=c,c++', '--enable-shared',
                         '--enable-threads']
        self.build_cross_tool(cmdlist, 'gcc', tool_build, cfg_opts)


class Glibc:

    """A configuration for building glibc."""

    def __init__(self, compiler, arch=None, os_name=None, variant=None,
                 cfg=None, ccopts=None):
        """Initialize a Glibc object."""
        self.ctx = compiler.ctx
        self.compiler = compiler
        if arch is None:
            self.arch = compiler.arch
        else:
            self.arch = arch
        if os_name is None:
            self.os = compiler.os
        else:
            self.os = os_name
        self.variant = variant
        if variant is None:
            self.name = '%s-%s' % (self.arch, self.os)
        else:
            self.name = '%s-%s-%s' % (self.arch, self.os, variant)
        self.triplet = '%s-glibc-%s' % (self.arch, self.os)
        if cfg is None:
            self.cfg = []
        else:
            self.cfg = cfg
        self.ccopts = ccopts

    def tool_name(self, tool):
        """Return the name of a cross-compilation tool."""
        ctool = '%s-%s' % (self.compiler.triplet, tool)
        if self.ccopts and (tool == 'gcc' or tool == 'g++'):
            ctool = '%s %s' % (ctool, self.ccopts)
        return ctool

    def build(self):
        """Generate commands to build this glibc."""
        builddir = self.ctx.component_builddir('glibcs', self.name, 'glibc')
        installdir = self.ctx.glibc_installdir(self.name)
        logsdir = os.path.join(self.ctx.logsdir, 'glibcs', self.name)
        self.ctx.remove_recreate_dirs(installdir, builddir, logsdir)
        cmdlist = CommandList('glibcs-%s' % self.name, self.ctx.keep)
        cmdlist.add_command('check-compilers',
                            ['test', '-f',
                             os.path.join(self.compiler.installdir, 'ok')])
        cmdlist.use_path(self.compiler.bindir)
        self.build_glibc(cmdlist, False)
        self.ctx.add_makefile_cmdlist('glibcs-%s' % self.name, cmdlist,
                                      logsdir)

    def build_glibc(self, cmdlist, for_compiler):
        """Generate commands to build this glibc, either as part of a compiler
        build or with the bootstrapped compiler (and in the latter case, run
        tests as well)."""
        srcdir = self.ctx.component_srcdir('glibc')
        if for_compiler:
            builddir = self.ctx.component_builddir('compilers',
                                                   self.compiler.name, 'glibc',
                                                   self.name)
            installdir = self.compiler.sysroot
            srcdir_copy = self.ctx.component_builddir('compilers',
                                                      self.compiler.name,
                                                      'glibc-src',
                                                      self.name)
        else:
            builddir = self.ctx.component_builddir('glibcs', self.name,
                                                   'glibc')
            installdir = self.ctx.glibc_installdir(self.name)
            srcdir_copy = self.ctx.component_builddir('glibcs', self.name,
                                                      'glibc-src')
        cmdlist.create_use_dir(builddir)
        # glibc builds write into the source directory, and even if
        # not intentionally there is a risk of bugs that involve
        # writing into the working directory.  To avoid possible
        # concurrency issues, copy the source directory.
        cmdlist.create_copy_dir(srcdir, srcdir_copy)
        cfg_cmd = [os.path.join(srcdir_copy, 'configure'),
                   '--prefix=/usr',
                   '--enable-add-ons',
                   '--build=%s' % self.ctx.build_triplet,
                   '--host=%s' % self.triplet,
                   'CC=%s' % self.tool_name('gcc'),
                   'CXX=%s' % self.tool_name('g++'),
                   'AR=%s' % self.tool_name('ar'),
                   'AS=%s' % self.tool_name('as'),
                   'LD=%s' % self.tool_name('ld'),
                   'NM=%s' % self.tool_name('nm'),
                   'OBJCOPY=%s' % self.tool_name('objcopy'),
                   'OBJDUMP=%s' % self.tool_name('objdump'),
                   'RANLIB=%s' % self.tool_name('ranlib'),
                   'READELF=%s' % self.tool_name('readelf'),
                   'STRIP=%s' % self.tool_name('strip')]
        cfg_cmd += self.cfg
        cmdlist.add_command('configure', cfg_cmd)
        cmdlist.add_command('build', ['make'])
        cmdlist.add_command('install', ['make', 'install',
                                        'install_root=%s' % installdir])
        # GCC uses paths such as lib/../lib64, so make sure lib
        # directories always exist.
        cmdlist.add_command('mkdir-lib', ['mkdir', '-p',
                                          os.path.join(installdir, 'lib'),
                                          os.path.join(installdir,
                                                       'usr', 'lib')])
        if not for_compiler:
            cmdlist.add_command('check', ['make', 'check'])
            cmdlist.add_command('save-logs', [self.ctx.save_logs],
                                always_run=True)
        cmdlist.cleanup_dir('cleanup-src', srcdir_copy)
        cmdlist.cleanup_dir()


class Command:

    """A command run in the build process."""

    def __init__(self, desc, num, dir, path, command, always_run=False):
        """Initialize a Command object."""
        self.dir = dir
        self.path = path
        self.desc = desc
        trans = str.maketrans({' ': '-'})
        self.logbase = '%03d-%s' % (num, desc.translate(trans))
        self.command = command
        self.always_run = always_run

    @staticmethod
    def shell_make_quote_string(s):
        """Given a string not containing a newline, quote it for use by the
        shell and make."""
        assert '\n' not in s
        if re.fullmatch('[]+,./0-9@A-Z_a-z-]+', s):
            return s
        strans = str.maketrans({"'": "'\\''"})
        s = "'%s'" % s.translate(strans)
        mtrans = str.maketrans({'$': '$$'})
        return s.translate(mtrans)

    @staticmethod
    def shell_make_quote_list(l, translate_make):
        """Given a list of strings not containing newlines, quote them for use
        by the shell and make, returning a single string.  If translate_make
        is true and the first string is 'make', change it to $(MAKE)."""
        l = [Command.shell_make_quote_string(s) for s in l]
        if translate_make and l[0] == 'make':
            l[0] = '$(MAKE)'
        return ' '.join(l)

    def shell_make_quote(self):
        """Return this command quoted for the shell and make."""
        return self.shell_make_quote_list(self.command, True)


class CommandList:

    """A list of commands run in the build process."""

    def __init__(self, desc, keep):
        """Initialize a CommandList object."""
        self.cmdlist = []
        self.dir = None
        self.path = None
        self.desc = [desc]
        self.keep = keep

    def desc_txt(self, desc):
        """Return the description to use for a command."""
        return '%s %s' % (' '.join(self.desc), desc)

    def use_dir(self, dir):
        """Set the default directory for subsequent commands."""
        self.dir = dir

    def use_path(self, path):
        """Set a directory to be prepended to the PATH for subsequent
        commands."""
        self.path = path

    def push_subdesc(self, subdesc):
        """Set the default subdescription for subsequent commands (e.g., the
        name of a component being built, within the series of commands
        building it)."""
        self.desc.append(subdesc)

    def pop_subdesc(self):
        """Pop a subdescription from the list of descriptions."""
        self.desc.pop()

    def create_use_dir(self, dir):
        """Remove and recreate a directory and use it for subsequent
        commands."""
        self.add_command_dir('rm', None, ['rm', '-rf', dir])
        self.add_command_dir('mkdir', None, ['mkdir', '-p', dir])
        self.use_dir(dir)

    def create_copy_dir(self, src, dest):
        """Remove a directory and recreate it as a copy from the given
        source."""
        self.add_command_dir('copy-rm', None, ['rm', '-rf', dest])
        parent = os.path.dirname(dest)
        self.add_command_dir('copy-mkdir', None, ['mkdir', '-p', parent])
        self.add_command_dir('copy', None, ['cp', '-a', src, dest])

    def add_command_dir(self, desc, dir, command, always_run=False):
        """Add a command to run in a given directory."""
        cmd = Command(self.desc_txt(desc), len(self.cmdlist), dir, self.path,
                      command, always_run)
        self.cmdlist.append(cmd)

    def add_command(self, desc, command, always_run=False):
        """Add a command to run in the default directory."""
        cmd = Command(self.desc_txt(desc), len(self.cmdlist), self.dir,
                      self.path, command, always_run)
        self.cmdlist.append(cmd)

    def cleanup_dir(self, desc='cleanup', dir=None):
        """Clean up a build directory.  If no directory is specified, the
        default directory is cleaned up and ceases to be the default
        directory."""
        if dir is None:
            dir = self.dir
            self.use_dir(None)
        if self.keep != 'all':
            self.add_command_dir(desc, None, ['rm', '-rf', dir],
                                 always_run=(self.keep == 'none'))

    def makefile_commands(self, wrapper, logsdir):
        """Return the sequence of commands in the form of text for a Makefile.
        The given wrapper script takes arguments: base of logs for
        previous command, or empty; base of logs for this command;
        description; directory; PATH addition; the command itself."""
        # prev_base is the base of the name for logs of the previous
        # command that is not always-run (that is, a build command,
        # whose failure should stop subsequent build commands from
        # being run, as opposed to a cleanup command, which is run
        # even if previous commands failed).
        prev_base = ''
        cmds = []
        for c in self.cmdlist:
            ctxt = c.shell_make_quote()
            if prev_base and not c.always_run:
                prev_log = os.path.join(logsdir, prev_base)
            else:
                prev_log = ''
            this_log = os.path.join(logsdir, c.logbase)
            if not c.always_run:
                prev_base = c.logbase
            if c.dir is None:
                dir = ''
            else:
                dir = c.dir
            if c.path is None:
                path = ''
            else:
                path = c.path
            prelims = [wrapper, prev_log, this_log, c.desc, dir, path]
            prelim_txt = Command.shell_make_quote_list(prelims, False)
            cmds.append('\t@%s %s' % (prelim_txt, ctxt))
        return '\n'.join(cmds)


def get_parser():
    """Return an argument parser for this module."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-j', dest='parallelism',
                        help='Run this number of jobs in parallel',
                        type=int, default=os.cpu_count())
    parser.add_argument('--keep', dest='keep',
                        help='Whether to keep all build directories, '
                        'none or only those from failed builds',
                        default='none', choices=('none', 'all', 'failed'))
    parser.add_argument('topdir',
                        help='Toplevel working directory')
    parser.add_argument('action',
                        help='What to do',
                        choices=('checkout', 'host-libraries', 'compilers',
                                 'glibcs'))
    parser.add_argument('configs',
                        help='Versions to check out or configurations to build',
                        nargs='*')
    return parser


def main(argv):
    """The main entry point."""
    parser = get_parser()
    opts = parser.parse_args(argv)
    topdir = os.path.abspath(opts.topdir)
    ctx = Context(topdir, opts.parallelism, opts.keep, opts.action)
    ctx.run_builds(opts.action, opts.configs)


if __name__ == '__main__':
    main(sys.argv[1:])
