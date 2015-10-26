#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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

from email.header import Header
from email.mime.text import MIMEText
from email.utils import parseaddr, formataddr
import getpass
import smtplib

SMTP_HOST = 'smtp.163.com'

def _format_addr(s):
    name, addr = parseaddr(s)
    return formataddr((Header(name, 'utf-8').encode(), addr))

def _get_mail_user():
    user = input('User: ')
    passwd = getpass.getpass('Password: ')
    receiver = input('To: ')

    return user, passwd, receiver

def _genrate_message(sender, receiver, text='Hello, world!'):
    msg = MIMEText(text, 'plain', 'utf-8')
    msg['From'] = _format_addr('Sender <%s>' % sender)
    msg['To'] = _format_addr('Receiver <%s>' % receiver)
    msg['Subject'] = Header('Sender -> Receiver', 'utf-8').encode()
    return msg

if __name__ == '__main__':
    user, passwd, receiver = _get_mail_user()
    msg = _genrate_message(user, receiver)

    server = smtplib.SMTP(SMTP_HOST, 25)
    server.set_debuglevel(1)
    server.login(user, passwd)
    server.sendmail(user, [receiver], msg.as_string())
    server.quit()
