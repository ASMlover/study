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

import async_timer as _at
from common.nyx_entity import EntityManager
from log.nyx_log import LogManager
from rpc.nyx_service import IoService
from distributed.game import nyx_game_global as _gglobal

class NyxServer(IoService):
    def __init__(self, tick_time=0.1):
        super(NyxServer, self).__init__()
        self._logger = LogManager.get('NyxCore.Server.NyxServer')
        self._tick_timer = _at.add_cycle_timer(tick_time, self._on_tick)
        self.servers = []
        self.save_list = []
        self.save_index = 0

    def _on_tick(self):
        for entity in EntityManager.get_instance()._entities.values():
            try:
                entity.on_tick()
            except:
                if _gglobal.nyx_game_event_callback is not None:
                    import sys
                    t, v, tb = sys.exc_info()
                    _gglobal.nyx_game_event_callback.on_traceback(t, v, tb)
                self._logger.nyxlog_exception()
                return

    def add_server(self, rpc_server):
        self.servers.append(rpc_server)

    def run(self, timeout=None):
        super(NyxServer, self).run(timeout)

    def generate_save_list(self):
        self.save_list = []
        for entity in EntityManager.get_instance()._entities.itervalues():
            if entity.is_persistent():
                self.save_list.append(entity.id)
        self.save_index = 0

    def save_entities(self):
        # TODO:
        pass

    def stop(self):
        super(NyxServer, self).stop()
        self._tick_timer.cancel()
        for server in self.servers:
            server.close()
