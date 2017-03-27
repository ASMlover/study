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

from bson import objectid
from common.nyx_id import IdCreator
from distributed.gate import nyx_gate_global as _gglobal
from log.nyx_log import LogManager
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto import nyx_gate_game_pb2 as _gg_pb2
from proto.pyproto import nyx_client_gate_pb2 as _cg_pb2
from rpc.nyx_compressor import Compressor

class GateClientProxy(object):
    def __init__(self, stub, client_info):
        super(GateClientProxy, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Gate.GateClientProxy')
        # TODO:
        pass

    def get_session_id(self):
        # TODO:
        pass

    def get_client_id(self):
        # TODO:
        pass

    def destroy(self):
        # TODO:
        pass

    def disconnect(self):
        # TODO:
        pass

    def connect_response(self, response):
        # TODO:
        pass

    def create_entity(self, entity_info):
        # TODO:
        pass

    def destroy_entity(self, entity_info):
        # TODO:
        pass

    def entity_message(self, entity_msg):
        # TODO:
        pass

    def chat_to_client(self, outband_info):
        # TODO:
        pass

    def reg_md5index(self, md5_index):
        # TODO:
        pass

class GateService(_cg_pb2.SClientToGate):
    def __init__(self, proxy_mgr, gate_config, gate_id):
        super(GateService, self).__init__(self)
        self.logger = LogManager.get_logger('NyxCore.Gate.GateService')
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def _remove_client_proxy(self, client_id, unreg_rpc_channel=True):
        # TODO:
        pass

    def seed_request(self, controller, request, done):
        # TODO:
        pass

    def session_key(self, controller, key, done):
        # TODO:
        pass

    def connect_server(self, controller, request, done):
        # TODO:
        pass

    def entity_message(self, controller, entity_msg, done):
        # TODO:
        pass

    def reg_md5index(self, controller, md5_index, done):
        # TODO:
        pass
