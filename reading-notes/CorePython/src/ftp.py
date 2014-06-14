# Copyright (c) 2014 ASMlover. All rights reserved.
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
#
#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import ftplib
import os
import socket


HOST = 'ftp.mozilla.org'
DIRN = 'pub/mozilla.org/webtools'
FILE = 'bugzilla-LATEST.tar.gz'



def main():
  try:
    f = ftplib.FTP(HOST)
  except (socket.error, socket.gaierror), e:
    print 'ERROR: cannot reach "%s"' % HOST
    return
  print '*** Connected to host "%s"' % HOST

  try:
    f.login()
  except ftplib.error_perm:
    print 'ERROR: cannot login anonymously'
    f.quit()
    return
  print '*** Logined in as "anonymously"'

  try:
    f.cwd(DIRN)
  except ftplib.error_perm:
    print 'ERROR: cannot cd to "%s"' % DIRN
    f.quit()
    return
  print '*** Changed to "%s" folder' % DIRN

  try:
    f.retrbinary('RETR %s' % FILE, open(FILE, 'wb').write)
  except ftplib.error_perm:
    print 'ERROR: cannot read file "%s"' % FILE
    os.unlink(FILE)
  else:
    print '*** Downloaded "%s" to cwd' % FILE
  f.quit()
  return



if __name__ == '__main__':
  main()
