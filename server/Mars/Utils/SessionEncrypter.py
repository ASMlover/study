#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

class LoginKeyDecrypterNoKeyczar(object):
    def __init__(self, loginKeyPath):
        super(LoginKeyDecrypterNoKeyczar, self).__init__()
        keyContent = open(loginKeyPath).read()
        key = RSA.importKey(keyContent)
        self.decrypter = PKCS1_OAEP.new(key)

    def decrypt(self, encryptedText):
        return self.decrypter.decrypt(encryptedText)

class LoginKeyEncrypterNoKeyczar(object):
    def __init__(self, loginKeyPath=None, keyContent=None):
        super(LoginKeyEncrypterNoKeyczar, self).__init__()
        assert loginKeyPath or keyContent

        if loginKeyPath:
            keyContent = open(loginKeyPath).read()
        key = RSA.importKey(keyContent)
        self.encrypter = PKCS1_OAEP.new(key)

    def encrypt(self, data):
        return self.encrypter.encrypt(data)

class LoginKeyDecrypter(object):
    def __init__(self, loginKeyPath):
        super(LoginKeyDecrypter, self).__init__()
        self.decrypter = Crypter.Read(loginKeyPath)

    def decrypt(self, encryptedText):
        return self.decrypter.Decrypt(encryptedText, None)

class LoginKeyEncrypter(object):
    def __init__(self, loginKeyPath):
        super(LoginKeyEncrypter, self).__init__()
        self.encrypter = Encrypter.Read(loginKeyPath)

    def encrypt(self, data):
        return self.encrypter.Encrypt(data, None)

class ARC4Crypter(object):
    def __init__(self, key=None):
        super(ARC4Crypter, self).__init__()
        if key == None:
            seed = os.urandom(256)
            nonce = Random.new().read(256)
            key = SHA.new(seed + nonce).digest()
        self.cipher = ARC4.new(key)

    def encrypt(self, data):
        return self.cipher.encrypt(data)

    def decrypt(self, encryptedText):
        return self.cipher.decrypt(encryptedText)
