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

# Just for Python3

from email import encoders
from email.header import Header
from email.mime.base import MIMEBase
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.utils import formataddr
from email.utils import parseaddr
import getpass
import smtplib
import os

SMTP_HOST = 'smtp.163.com'

def _format_addr(s):
    name, addr = parseaddr(s)
    return formataddr((Header(name, 'utf-8').encode(), addr))

def _get_mail_user():
    user = input('User: ')
    passwd = getpass.getpass('Password: ')
    receiver = input('To: ')
    attachment = input('Attachment: ')

    return user, passwd, receiver, attachment

def _genrate_message(sender, receiver, attachment, text='Hello, world!'):
    msg = MIMEMultipart()
    msg['From'] = _format_addr('Sender <%s>' % sender)
    msg['To'] = _format_addr('Receiver <%s>' % receiver)
    msg['Subject'] = Header('Sender -> Receiver', 'utf-8').encode()
    msg.attach(MIMEText(text, 'plain', 'utf-8'))
    with open(attachment, 'rb') as f:
        filename = os.path.split(attachment)[-1]
        ext = os.path.splitext(filename)[-1]
        mime = MIMEBase('file', ext, filename=filename)
        mime.add_header(
                'Content-Disposition', 'attachment', filename=filename)
        mime.add_header('Content', '<0>')
        mime.add_header('X-Attachment-Id', '0')
        mime.set_payload(f.read())
        encoders.encode_base64(mime)
        msg.attach(mime)

    return msg

if __name__ == '__main__':
    user, passwd, receiver, attachment = _get_mail_user()
    msg = _genrate_message(user, receiver, attachment)

    server = smtplib.SMTP(SMTP_HOST, 25)
    server.set_debuglevel(1)
    server.login(user, passwd)
    server.sendmail(user, [receiver], msg.as_string())
    server.quit()
