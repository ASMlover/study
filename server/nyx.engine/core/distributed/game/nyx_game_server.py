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

import argparse
import async_timer as _at
from common import nyx_common as _nc
from common.nyx_entity import EntityFactory, EntityManager, EntityScanner
from common.nyx_codec import ProtoEncoder
from distributed.game import nyx_game_global as _gglobal
from distributed.game.nyx_dbmgr_client import DBManagerClient
from distributed.game.nyx_gamemgr_client import GameManagerClient
from distributed.game.nyx_game_service import GameService, GameClientProxyManager
from log.nyx_log import LogManager
from rpc.nyx_rpc_channel import RpcChannelCreator, RpcChannelManager
from rpc.nyx_service import IoService
from rpc.nyx_tcp_server import TcpServer
from proto.pyproto import nyx_common_pb2 as _c_pb2
from server.nyx_server import NyxServer
from server.nyx_server_entity import ServerEntity

class GameServer(object):
    def __init__(self, config, config_sections):
        super(GameServer, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Game.GameServer')
        # TODO:
        pass

    def _merge_common_config(self):
        # TODO:
        pass

    def _create_game_service(self):
        # TODO:
        pass

    def call_init_function(self):
        # TODO:
        pass

    def setup_logic_path(self):
        # TODO:
        pass

    def register_entities(self):
        # TODO:
        pass

    def _start_console(self):
        # TODO:
        pass

    def _check_game_started(self):
        # TODO:
        pass

    def run(self):
        # TODO:
        pass

    def stop(self):
        # TODO:
        pass
