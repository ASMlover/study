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
from bson import BSON
from log.nyx_log import LogManager
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto import nyx_db_manager_pb2 as _dm_pb2
from rpc.nyx_channel_client import ChannelClient
from distributed.game import nyx_game_global as _gglobal

class DBManagerProxy(object):
    """game和db通信的rpc client代理"""
    def __init__(self, stub):
        super(DBManagerProxy, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Game.DBManagerProxy')
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def is_dbmgr_connected(self):
        # TODO:
        pass

    def reg_to_dbmgr(self, server_info):
        # TODO:
        pass

    def db_find_doc(self, db, collection, query, fields=None,
            limit=1, callback=None, seq_flag=False, sort=None,
            seq_key=None, read_pref=None, hint=None, skip=None):
        # TODO:
        pass

    def db_update_doc(self, db, collection, query, doc,
            callback=None, upsert=True, multi=False, seq_flag=False, seq_key=None):
        # TODO:
        pass

    def db_operation_index(self, db, collection, op_type, index, desc, callback=None):
        # TODO:
        pass

    def db_delete_doc(self, db, collection, query, callback=None, seq_flag=False, seq_key=None):
        # TODO:
        pass

    def db_insert_doc(self, db, collection, doc, callback=None, seq_flag=False, seq_key=None):
        # TODO:
        pass

    def db_count_doc(self, db, collection, query={}, callback=None):
        # TODO:
        pass

    def db_find_and_modify_doc(self, db, collection, query, update=None, fields=None,
            upsert=False, new=False, callback=None, seq_flag=False, seq_key=None):
        # TODO:
        pass

    def db_run_procedure(self, db, proc, callback, *args, **kwargs):
        # TODO:
        pass

    def db_create_collection(self, db, collection, opts, callback):
        # TODO:
        pass

    def db_reload_procedure(self):
        # TODO:
        pass

class DBManagerClient(_dm_pb2.SDBManagerToClient):
    _STATUS_NOT_CONNECTED = 1
    _STATUS_CONNECTING = 2
    _STATUS_CONNECTED = 3

    def __init__(self, config, config_sections, name=None):
        super(DBManagerClient, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Game.DBManagerClient')
        # TODO:
        pass
