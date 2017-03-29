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

from collections import deque
import os
import sys
from bson import BSON
import async_time as _at
from pymongo import MongoClient
from pymongo.errors import DuplicateKeyError, CollectionInvalid
from distributed.dbmgr import nyx_dbmgr_global as _dglobal
from log.nyx_log import LogManager
from proto.pyproto import nyx_db_manager_pb2 as _dm_pb2

_OP_FIND_DOC = 1
_OP_UPDATE_DOC = 2
_OP_DELETE_DOC = 3
_OP_INSERT_DOC = 4
_OP_COUNT_DOC = 5
_OP_FIND_AND_MODIFY_DOC = 6
_OP_RUN_PROCEDURE = 7
_OP_ENSURE_INDEX = 8
_OP_DROP_INDEX = 9
_OP_RESET_INDEX = 10
_OP_CREATE_COLLECTION = 11

class DBClientProxy(object):
    def __init__(self, stub, server_holder):
        super(DBClientProxy, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.DBMgr.DBClientProxy')
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def on_db_operation_index(self, callback_id, status):
        # TODO:
        pass

    def on_db_find_doc(self, callback_id, status, doc=None):
        # TODO:
        pass

    def on_db_count_doc(self, callback_id, status, count):
        # TODO:
        pass

    def on_db_update_doc(self, callback_id, status):
        # TODO:
        pass

    def on_db_delete_doc(self, callback_id, status):
        # TODO:
        pass

    def on_db_insert_doc(self, callback_id, status, insert_id=None):
        # TODO:
        pass

    def on_db_find_and_modify_doc(self, callback_id, status, doc=None):
        # TODO:
        pass

    def on_db_run_procedure(self, callback_id, status, doc):
        # TODO:
        pass

    def on_db_status(self, status):
        # TODO:
        pass

    def on_db_create_collection(self, callback_id, status):
        # TODO:
        pass

class MongoClientDriver(object):
    def __init__(self, mongo_config, db_config):
        super(MongoClientDriver, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.DBMgr.MongoClientDriver')
        # TODO:
        pass

    def connect_mongo(self):
        # TODO:
        pass

    def request_callback(self):
        # TODO:
        pass

    def drop_collection(self, db, collection):
        # TODO:
        pass

    def _init_procedure(self):
        # TODO:
        pass

    def get_db_status(self):
        # TODO:
        pass

    def _db_operation_response(self, request, result):
        # TODO:
        pass

    def _db_operation_except(self, request, exc_info):
        # TODO:
        pass

    def _db_operation_callback(self, request, result, callback):
        # TODO:
        pass

    def _do_db_operation(self, op_type, op_request, client_proxy):
        # TODO:
        pass

    def db_operation(self, op_type, op_request, client_proxy, callback):
        # TODO:
        pass

    def get_db_result(self):
        # TODO:
        pass

    def report_failure(self, op_type, op_request, client_proxy):
        # TODO:
        pass
