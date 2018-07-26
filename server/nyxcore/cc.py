#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import base64
import codecs
import getopt
import os
import sys

def encrypt(key, string):
    enc = []
    key_len = len(key)
    for i in xrange(len(string)):
        c = key[i % key_len]
        enc_c = chr((ord(string[i]) + ord(c)) % 256)
        enc.append(enc_c)
    return base64.urlsafe_b64encode(''.join(enc))

def decrypt(key, string):
    dec = []
    string = base64.urlsafe_b64decode(string)
    key_len = len(key)
    for i in xrange(len(string)):
        c = key[i % key_len]
        dec_c = chr((256 + ord(string[i]) - ord(c)) % 256)
        dec.append(dec_c)
    return ''.join(dec)

_crypter_args = dict(
    root_dir = './',
    exclude_dirs = [],
    password = '111111',
    encrypt = False,
    quiet = False,
)

def _parse_arguments():
    opts, args = getopt.getopt(
            sys.argv[1:], '', ['root=', 'exclude=', 'password=', 'encrypt', 'quiet'])
    for opt, val in opts:
        if opt == '--root':
            _crypter_args['root_dir'] = val
        elif opt == '--exclude':
            _crypter_args['exclude_dirs'] = val.split(',')
        elif opt == '--password':
            _crypter_args['password'] = val
        elif opt == '--encrypt':
            _crypter_args['encrypt'] = True
        elif opt == '--quiet':
            _crypter_args['quiet'] = True

def _is_inclusive(child, parent):
    relpath = os.path.relpath(child, parent)
    return not relpath.startswith('..')

def _crypter_codes(password, root_dir, exclude_dirs, is_encrypt=True):
    exclude_dirs = set(exclude_dirs + ['.svn', '.git', '.idea'])
    exclude_dirs = set(os.path.join(root_dir, dir) for dir in exclude_dirs)

    for path, dirs, files in os.walk(root_dir):
        if '.svn' in dirs:
            dirs.remove('.svn')
        if '.git' in dirs:
            dirs.remove('.git')

        exclude = False
        for exclude_dir in exclude_dirs:
            if _is_inclusive(path, exclude_dir):
                exclude = True
        if exclude:
            continue

        if is_encrypt:
            crypter_fun = encrypt
            valid_ext = ('.h', '.c', '.cc', '.cpp')
        else:
            crypter_fun = decrypt
            valid_ext = ('.enc',)

        for f in files:
            fname = os.path.join(path, f)
            fnext = os.path.splitext(f)[1].lower()

            if fnext in valid_ext:
                outfile = '%s.enc' % fname if is_encrypt else os.path.splitext(fname)[0]
                with open(fname, 'rb') as fp:
                    outstr = crypter_fun(password, fp.read())
                with codecs.open(outfile, 'wb', 'utf-8') as fp:
                    fp.write(outstr)
                os.remove(fname)

def main():
    _parse_arguments()
    print _crypter_args
    _crypter_codes(
            _crypter_args['password'],
            _crypter_args['root_dir'],
            _crypter_args['exclude_dirs'],
            _crypter_args['encrypt'])

if __name__ == '__main__':
    main()
