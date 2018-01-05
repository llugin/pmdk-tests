#!/usr/bin/env python3
#
# Copyright 2018, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


import sys
import re
import argparse
from os import linesep, walk, path
from difflib import unified_diff
from pathlib import PurePath
from subprocess import check_output, STDOUT, CalledProcessError

DOXY_START = '/**'
DOXY_END = '*/'
DOXY_EXTENSIONS = ('.cc',)
LICENSE_EXTENSIONS = ('.cc', '.cpp', '.h', '.sh',
                      '.py', '.tpp', '.txt', '.cmake')
CPP_EXTENSIONS = ('.c', '.cc', '.cpp', '.h', '.tpp')

CLANG_STYLE_ARGS = ['BasedOnStyle: Google',
               'AllowShortFunctionsOnASingleLine: None',
               'AllowShortIfStatementsOnASingleLine: false',
               'AllowShortCaseLabelsOnASingleLine: false']
CLANG_STYLE = '"{' + ', '.join(CLANG_STYLE_ARGS) + '}"'

COPYRIGHT_HEADER_PATTERN = r'^(.*)Copyright (.*), Intel Corporation'

errors_log = []
missing_licence_files = []
clang_format_bin = ''


def check_file(filepath, lines):
    checking_results = []
    if has_extension(filepath, LICENSE_EXTENSIONS):
        checking_results.append(check_license_date(filepath, lines))
    return all(checking_results)


def get_license_boundary_lines(lines, license_filepath):
    license_lines = read_file_lines(license_filepath)
    get_first_license_line(license_lines)




    return None, None



def check_license()

def format_license()


def check_license_date(filepath, lines):
    '''Checks if year on license header contains year of last file \
     modification in repository.'''
    cmd = 'git log -1 --format="%ad" --date=format:"%Y" {}'.format(filepath)
    returncode, out = run(cmd)
    if returncode != 0:
        print(out)
        sys.exit('git command {} exited with {}.'.
                 format(cmd, returncode))
    last_modification_year = out.decode('utf-8').strip()

    copyright_header_pattern = r'^(.*)Copyright (.*), Intel Corporation'
    match = re.search(copyright_header_pattern, ''.join(lines))

    ok = True
    if match and not last_modification_year in match:
        errors_log.append('{}: incorrect license year'.format(filepath))
        ok = False
    elif not match:
        ok = False
    return ok


def get_formatted_lines(filepath, lines, license_file):
    formatted = lines[:]

    if has_extension(filepath, CPP_EXTENSIONS):
        formatted = clang_format(filepath)
    if has_extension(filepath, DOXY_EXTENSIONS):
        formatted = format_doxy(formatted)
    if has_extension(filepath, LICENSE_EXTENSIONS):
        formatted = format_licence(filepath, formatted, license_file)

    return formatted


def read_file_lines(filepath):
    with open(filepath, 'r', encoding='utf-8', newline=linesep) as f:
        return f.readlines()


def write_to_file(filepath, formatted):
    '''Write formatted lines to file.'''
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(''.join([line.replace('\r\n', '\n') for line in formatted]))


def format_doxy(lines):
    '''Format doxy-style test designs in given file lines.'''
    indices = get_doxy_line_indices(lines)
    if indices:
        return [format_doxy_line(
            line) if i in indices else line for i, line in enumerate(lines)]
    else:
        return lines


def get_doxy_line_indices(lines):
    '''Collect indices of lines in which \
     doxy-style test design headers occur.'''
    iterations = range(len(lines))

    def end_found(i):
        return lines[i].strip() == DOXY_END and i + 1 in iterations \
            and ('TEST_F' in lines[i + 1] or 'TEST_P' in lines[i + 1])

    start_found = False
    all_doxy_indices = []
    local_doxy_indices = []
    for i in iterations:
        if lines[i].strip() == DOXY_START:
            local_doxy_indices = [i]
            start_found = True
            continue
        if start_found and end_found(i):
            all_doxy_indices.extend(local_doxy_indices + [i])
            local_doxy_indices = []
            start_found = False
            continue
        if start_found and lines[i].strip().startswith('*'):
            local_doxy_indices.append(i)
        else:
            start_found = False

    return all_doxy_indices


def format_doxy_line(line):
    '''Apply formatting to test design line depending on its recognized type.'''
    if line.strip() == DOXY_START and not line.startswith(DOXY_START):
        return line.strip() + linesep
    if line.strip() == DOXY_END and not line.startswith(' ' + DOXY_END):
        return ' ' + DOXY_END + line.split(DOXY_END)[1]
    if '*' in line and r'\li' in line and not line.startswith(
            r' *          \li'):
        return r' *          \li' + line.split(r'\li')[1]
    if r'\test' in line and not line.startswith(r' * \test'):
        return r' * \test' + line.split(r'\test')[1]
    if line.strip().startswith('*') and not line.startswith(' *'):
        return ' *' + line.split('*')[1]

    return line


def format_licence(filepath, lines, license_file):
    '''Format license.'''
    shebang = '#!'
    comments = ['/*', ' *', '#', ' */']

    if has_extension(filepath, CPP_EXTENSIONS):
        comments.remove('#')

    def get_comment_in(line):
        return next(
            (c for c in comments if line.lstrip().startswith(c.strip())), None)

    licence_found = False
    formatted = lines[:]
    for i, line in enumerate(formatted):
        if i == 0 and line.startswith(shebang):
            continue
        if not get_comment_in(line) and not line.strip():
            break
        used_comment = get_comment_in(line)
        if used_comment:
            licence_found = True
            formatted[i] = used_comment + \
                line.split(used_comment.strip(), 1)[1]

    if not licence_found:
        missing_licence_files.append(filepath)

    return formatted


def check_prerequisites():
    if not find_clang_format():
        sys.exit('No clang-format in version 3.9 found.')


def find_clang_format():
    global clang_format_bin

    returncode, out = run('clang-format --version', shell=True)
    if returncode == 0 and b'version 3.9' in out:
        clang_format_bin = 'clang-format'
        return True

    returncode, _ = run('clang-format-3.9 --version', shell=True)
    if returncode == 0:
        clang_format_bin = 'clang-format-3.9'
        return True
    return False


def clang_format(filepath):
    cmd = clang_format_bin + ' ' + filepath + ' -style={}'.format(CLANG_STYLE)
    returncode, out = run(cmd, shell=True)
    return out.decode('utf-8').splitlines(keepends=True)


def check_diff(filepath, lines, formatted):
    diff = list(unified_diff(lines, formatted))
    if diff:
        if not args.quiet:
            print('{} diff:'.format(filepath))
            sys.stdout.writelines(diff)
            print()
        if args.in_place:
            write_to_file(filepath, formatted)

    return bool(diff)

if __name__ == '__main__':
    class CheckPrerequisitesAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            check_prerequisites()
            parser.exit()

    parser = argparse.ArgumentParser(
        description='Autoformat doxy-style test designs'
                    ' in comments and licences.')
    parser.add_argument('-p', '--path',
                        help='Path to the file to be formatted or directory'
                             ' if in recursive mode.', required=True)
    parser.add_argument(
        '-r', '--recursive', action='store_true', default=False,
        help='Recursive mode with root directory provided by'
             ' -p/--path argument')
    parser.add_argument('-i', '--in-place', action='store_true', default=False,
                        help="Format files in place,"
                             " don't end with failure in case of diffs.")
    parser.add_argument('--ignore-dir', default='build',
                        help='In recursive mode ignore elements under '
                             'directory with given name. Default: build')
    default_license_path = path.abspath(path.join(path.dirname(__file__), '..', '..', 'LICENSE'))
    parser.add_argument('-l,', '--license', default=default_license_path,
                        help='Path to LICENSE file')
    parser.add_argument('--check-prerequisites',  nargs=0,
                        help='Check prerequisites only.',
                        action=CheckPrerequisitesAction)
    parser.add_argument(
        '-q', '--quiet', action='store_true',
        default=False, help='Do not print diff to output.')
    args = parser.parse_args()
    check_prerequisites()

    files_to_process = []
    if args.recursive:
        if not path.isdir(args.path):
            sys.exit('{} is not a directory'.format(path.abspath(args.path)))
        files_to_process = get_files_to_process(args.path, args.ignore_dir)
    else:
        if not path.isfile(args.path):
            sys.exit('{} is not a regular file'.format(path.abspath(args.path)))
        files_to_process.append(args.path)

    diffs_after_formatting = False
    for filepath in files_to_process:
        lines = read_file_lines(filepath)
        check_file(filepath, lines)
        formatted = get_formatted_lines(filepath, lines, args.license)
        diff_occurs = check_diff(filepath, lines, formatted)
        if diff_occurs:
            diff_after_formatting = True
            if args.in_place:
                write_to_file(filepath, formatted)

    if errors_log:
        for line in errors_log:
            print(line)

    if not args.in_place and (diffs_after_formatting or errors_log):
        sys.exit(1)
