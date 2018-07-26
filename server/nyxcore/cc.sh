#!/usr/bin/env bash
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

if [ ! -n "$1" ]; then
  echo "Usage: crypter_codes.sh <password> <encrypt>: miss password"
  echo "  passord -> setting encoding password"
  echo "  encrypt -> true(for encode)/false(for decode)"
  exit
fi
if [ ! -n "$2" ]; then
  echo "Usage: crypter_codes.sh <password> <encrypt>: miss encrypt flag"
  echo "  passord -> setting encoding password"
  echo "  encrypt -> true(for encode)/false(for decode)"
  exit
fi

CRYPTER_EXCLUDE=".svn,.git,.idea,3rdparty,cmake-build"
if [ "$2" == "true" ]; then
  python crypter_codes.py --root=./ --exclude=$CRYPTER_EXCLUDE --password=$1 --encrypt
else
  python crypter_codes.py --root=./ --exclude=$CRYPTER_EXCLUDE --password=$1
fi
