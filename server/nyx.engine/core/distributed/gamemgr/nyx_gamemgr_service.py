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
import async_time as _at
from common.nyx_id import IdCreator
from distributed.gamemgr import nyx_gamemgr_global as _gglobal
from log.nyx_log import LogManager
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto.nyx_common_pb2 import EntityMailbox
from proto.pyproto import nyx_game_manager_pb2 as _gm_pb2

_SERVER_TYPE_GATE = 0
_SERVER_TYPE_GAME = 1
_SERVER_TYPE_DBMGR = 2
_SERVER_TYPE_GAMEMGR = 3

class ServerClientProxy(object):
    def __init__(self, stub, server_info_holder, server_type=_SERVER_TYPE_GATE):
        super(ServerClientProxy, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.GameMgr.ServerClientProxy')
        # TODO:
        pass

    def on_forward_entity_message(self, entity_msg):
        # TODO:
        pass

    def on_get_game_server_list(self, infos):
        # TODO:
        pass

    def on_global_entity_message(self, entity_msg):
        # TODO:
        pass

    def on_global_message(self, global_msg):
        # TODO:
        pass

    def on_global_data(self, global_data):
        # TODO:
        pass

    def on_global_data_list(self, global_data_list):
        # TODO:
        pass

    def on_del_global_data(self, global_data):
        # TODO:
        pass

    def on_reg_global_entity_mailbox(self, reg_msg):
        # TODO:
        pass

    def on_unreg_global_entity_mailbox(self, reg_msg):
        # TODO:
        pass

    def on_global_entities(self, global_entities):
        # TODO:
        pass

    def create_entity(self, entity_info):
        # TODO:
        pass

    def run_script(self, script):
        # TODO:
        pass

    def on_server_control(self, control_op):
        # TODO:
        pass

    def game_manager_callback(self, callback_type, callback_id, return_value):
        # TODO:
        pass
