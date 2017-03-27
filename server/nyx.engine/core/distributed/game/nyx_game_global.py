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

# 运用于game server的全局数据信息

nyx_gamemgr_proxy = None # 同game manager通信的proxy
nyx_dbmgr_proxy = None # db manager通信的proxy
nyx_dbmgr_proxy_set = set()
nyx_dbmgr_proxy_external = {}
nyx_online_games = [] # 当前所有在线game server的ip和port信息
nyx_game_client_mgr = None # game client
nyx_game_info = None # 本机game server的ip和port信息
nyx_game_event_callback = None # 全局的事件回调entity
nyx_entity_global = {} # 保存全局的entity的地址信息
nyx_game = None # game server
nyx_game_name = '' # game server的名字信息
nyx_dbname = 'nyxdb' # 数据库名字
nyx_dbname_external = {}
nyx_def_savetime = None # 默认存盘时间
nyx_game_service = None

_nyx_gamemgr_callbacks = {} # game调用game manager的callbacks
_nyx_dbmgr_callbacks = {} # game调用db manager的callbacks
_nyx_gate_callbacks = {} # game调用gate的callbacks

proto_encoder = None
gameid = None # 游戏id
hostnum = None # 服务器组id
platform_cfg = None # 平台相关配置
monitor_mgr = None # 属性监控接口

class _RotatedIdGenerator(object):
    """产生一个简单的自增整数id"""
    _maxid = 2 ** 20
    def __init__(self):
        self._id = 1

    def genid(self):
        if self._id != _RotatedIdGenerator._maxid:
            self._id += 1
        else:
            self._id =  1
        return (nyx_game_info.sid << 20) + self._id

_nyx_gamemgr_id_gen = _RotatedIdGenerator()
def reg_gamemgr_callback(callback):
    callback_id = _nyx_gamemgr_id_gen.genid()
    _nyx_gamemgr_callbacks[callback_id] = callback
    return callback_id

def unreg_gamemgr_callback(callback_id):
    return _nyx_gamemgr_callbacks.pop(callback_id, None)

_nyx_dbmgr_id_gen = _RotatedIdGenerator()
def reg_dbmgr_callback(callback):
    callback_id = _nyx_dbmgr_id_gen.genid()
    _nyx_dbmgr_callbacks[callback_id] = callback
    return callback_id

def unreg_dbmgr_callback(callback_id):
    return _nyx_dbmgr_callbacks.pop(callback_id, None)

_nyx_gate_id_gen = _RotatedIdGenerator()
def reg_gate_callback(callback):
    callback_id = _nyx_gate_id_gen.genid()
    _nyx_gate_callbacks[callback_id] = callback
    return callback_id

def unreg_gate_callback(callback_id):
    return _nyx_gate_callbacks.pop(callback_id, None)
