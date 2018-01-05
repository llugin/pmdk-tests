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


import os
import sys
from os import path
from subprocess import check_output, STDOUT, CalledProcessError
from pathlib import PurePath
from difflib import unified_diff


def read_file_lines(filepath):
    with open(filepath, 'r', encoding='utf-8', newline=os.linesep) as f:
        return f.readlines()


def has_extension(filepath, extensions):
    '''Check if files extension is in given extensions.'''
    return any(filepath.endswith(ext) for ext in extensions)


def get_files_to_process(root_dir, ignored, extensions):
    root_dir = path.abspath(root_dir)
    if isinstance(ignored, str):
        ignored = [ignored]

    def is_ignored(file):
        parts = PurePath(path.abspath(file)).parts
        return any(i in PurePath(path.abspath(file)).parts for i in ignored)

    to_format = []
    for root, _, files in os.walk(root_dir):
        to_format.extend(path.join(root, file) for file in files if has_extension(
            file, extensions) and not is_ignored(path.join(root, file)))

    return to_format


def run(cmd, shell=False, cwd=None):
    try:
        out = check_output(cmd, shell=shell, cwd=cwd, stderr=STDOUT)
    except CalledProcessError as e:
        return e.returncode, e.output
    else:
        return 0, out


def check_diff(filepath, lines, formatted):
    diff = list(unified_diff(lines, formatted))
    if diff:
        print('{} diff:'.format(filepath))
        sys.stdout.writelines(diff)
        print()

    return bool(diff)
