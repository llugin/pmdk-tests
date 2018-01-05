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
from os import path
import check_utils

COPYRIGHT_HEADER_PATTERN = r'^(.*)Copyright (.*), Intel Corporation'
LICENSE_EXTENSIONS = ('.cc', '.cpp', '.h', '.sh',
                      '.py', '.tpp', '.txt', '.cmake')
IGNORED_FILES = '__init__.py'

errors = []


def validate_found_license_file(license_filepath):
    lines = check_utils.read_file_lines(license_filepath)
    if not re.search(COPYRIGHT_HEADER_PATTERN, lines[0]):
        sys.exit('Copyright header not found in the first line of {}'.format(license_filepath))


def check_license_date(filepath, license_header_line):
    cmd = 'git log -1 --format="%ad" --date=format:"%Y" {}'.format(filepath)
    returncode, out = check_utils.run(cmd, cwd=path.dirname(path.abspath(filepath)))

    if returncode != 0:
        print(out.decode('utf-8'))
        sys.exit('{} exited with {}.'.
                 format(cmd, returncode))

    last_modification_year = out.decode('utf-8').strip()
    if not last_modification_year in license_header_line:
        msg = '{}: last modification year({}) not found in license header'
        errors.append(msg.format(filepath, last_modification_year))


def check_license(filepath, license_lines):
    license_start = -1

    lines = check_utils.read_file_lines(filepath)
    for i, line in enumerate(lines):
        if re.search(COPYRIGHT_HEADER_PATTERN, line):
            check_license_date(filepath, line)
            license_start = i
            break

    if license_start == -1:
        errors.append('{}: license could not be found'.format(filepath))
        return

    # index + 1 to omit date-dependent copyright header
    for file_line, license_line in zip(lines[license_start + 1:], license_lines[1:]):
        if license_line.strip() not in file_line:
            errors.append('{}: license text mismatch with line "{}"'.format(filepath, license_line.strip()))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Check license in repository.')

    parser.add_argument('-r', '--root-dir',
                        help='Path to repository root dir', required=True)
    parser.add_argument('--ignore-dir', help='Ignore elements under directory with given name. Default: build', default='build')
    args = parser.parse_args()
    license_filepath = path.join(args.root_dir, 'LICENSE')
    validate_found_license_file(license_filepath)

    if not path.isdir(args.root_dir):
            sys.exit('{} is not a directory'.format(path.abspath(args.path)))
    files_to_process = check_utils.get_files_to_process(
        args.root_dir, [args.ignore_dir, IGNORED_FILES], LICENSE_EXTENSIONS)

    for filepath in files_to_process:
        license_lines = check_utils.read_file_lines(license_filepath)
        check_license(filepath, license_lines)

    if errors:
        for line in errors:
            print(line)
        sys.exit(1)


