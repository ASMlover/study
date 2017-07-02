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

import random
import async_timer
from Common.EntityUtils import EntityFactory, EntityManager
from Common.Codec import CodecDecoder
from Distributed.Game import GameGlobal
from Log.LogManager import LogManager

class Game2ClientProxy(object):
    def __init__(self, stub):
        super(Game2ClientProxy, self).__init__()
        self.logger = LogManager.get_logger('Game.Game2ClientProxy')
        self.client_stub = stub
        self.gate_id = 0

    def global_entity_message(self, msg):
        self.client_stub.global_entity_message(None, msg)

    def forward_entity_message(self, msg):
        self.client_stub.forward_entity_message(None, msg)

    def create_real_entity(self, create_info):
        self.client_stub.create_real_entity(None, create_info)

    def real_entity_created(self, entity_mb):
        self.client_stub.real_entity_created(None, entity_mb)

    def bind_client_to_game(self, bind_msg):
        self.client_stub.bind_client_to_game(None, bind_msg)

class Game2ClientProxyManager(object):
    def __init__(self):
        super(Game2ClientProxyManager, self).__init__()
        self.logger = LogManager.get_logger('Game.Game2ClientProxyManager')
        self.gate_proxies = {}
        self.gate_proxies_by_uuid = {}
        self.gate_index = 0
        GameGlobal.game_client_mgr = self

    def add_gate_proxy(self, proxy, gate_uuid=None):
        self.gate_index += 1
        proxy.gate_id = self.gate_index
        self.gate_proxies[self.gate_index] = proxy
        if gate_uuid is not None:
            self.gate_proxies_by_uuid[gate_uuid] = proxy
        return self.gate_index

    def del_gate_proxy(self, gate_id):
        self.gate_proxies.pop(gate_id, None)

    def get_random_gate_proxy(self):
        if len(self.gate_proxies) > 0:
            return random.choice(list(self.gate_proxies.itervalues()))
        return None

    def has_gate_proxy(self, gate_id):
        return gate_id in self.gate_proxies

    def get_gate_proxies(self):
        return self.gate_proxies
