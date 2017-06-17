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

import sys
import async_timer
from Common.EntityUtils import EntityManager
from Distributed.Game import GameGlobal
from Log.LogManager import LogManager
from Rpc.IoService import IoService

class NyxServer(IoService):
    """运行一个tick以及管理TcpServer"""
    def __init__(self, interval=0.1):
        super(NyxServer, self).__init__()
        self.logger = LogManager.get_logger('Server.NyxServer')
        self.servers = []
        self.save_list = []
        self.save_index = 0
        self.tick_timer = async_timer.add_cycle_timer(interval, self.on_tick)

    def on_tick(self):
        """调用所有Entity的tick"""
        for entity in EntityManager._entities.values():
            # 需要处理某些Entity销毁的情况
            try:
                entity.tick()
            except:
                if GameGlobal.game_event_callback is not None:
                    t, v, tb = sys.exc_info()
                    GameGlobal.game_event_callback.on_traceback(t, v, tb)
                self.logger._log_exception()
                self.logger.error('NyxServer.on_tick: get exception')
                return

    def run(self, timeout=None, process_large_buf=True):
        super(NyxServer, self).run(timeout, process_large_buf)

    def stop(self):
        self.logger.info('NyxServer.stop: NyxServer stop')
        super(NyxServer, self).stop()
        self.tick_timer.cancel()
        for server in self.servers:
            server.close()

    def add_server(self, rpc_server):
        self.servers.append(rpc_server)

    def generate_save_list(self):
        self.save_list = []
        for entity in EntityManager._entities.itervalues():
            if entity.is_persistent():
                self.save_list.append(entity.entity_id)
        self.save_index = 0

    def save_entities(self):
        if self.save_index > len(self.save_list) - 1:
            self.generate_save_list()
        list_len = len(self.save_list)
        if list_len == 0:
            return

        while self.save_index < len(self.save_list):
            entity = EntityManager.get_entity(self.save_list[self.save_index])
            # 需要处理有些Entity已经销毁的情况
            if entity is not None:
                entity.save()
                self.save_index += 1
                break
            self.save_index += 1
