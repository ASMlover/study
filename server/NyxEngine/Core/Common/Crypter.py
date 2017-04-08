#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

import os
from Crypto import Random
from Crypto.Cipher import ARC4, PKCS1_OAEP
from Crypto.Hash import SHA
from Crypto.PublicKey import RSA
from keyczar.keyczar import Crypter, Encrypter

class LoginKeyCrypterNoKeyczar(object):
    def __init__(self, keypath=None, keycontent=None):
        super(LoginKeyCrypterNoKeyczar, self).__init__()
        assert keypath or keycontent
        if keypath:
            keycontent = open(keypath).read()
        key = RSA.importKey(keycontent)
        self.crypter = PKCS1_OAEP.new(key)

    def encrypt(self, data):
        return self.crypter.encrypt(data)

    def decrypt(self, data):
        return self.crypter.decrypt(data)

class LoginKeyCrypter(object):
    def __init__(self, keypath):
        super(LoginKeyCrypter, self).__init__()
        self.encrypter = Encrypter.Read(keypath)
        self.decrypter = Crypter.Read(keypath)

    def encrypt(self, data):
        return self.encrypter.Encrypt(data, None)

    def decrypt(self, data):
        return self.decrypter.Decrypt(data, None)

class ARC4Crypter(object):
    def __init__(self, key=None):
        super(ARC4Crypter, self).__init__()
        if key is None:
            seed = os.urandom(256)
            nonce = Random.new().read(256)
            key = SHA.new(seed + nonce).digest()
        self.cipher = ARC4.new(key)

    def encrypt(self, data):
        return self.cipher.encrypt(data)

    def decrypt(self, data):
        return self.cipher.decrypt(data)
