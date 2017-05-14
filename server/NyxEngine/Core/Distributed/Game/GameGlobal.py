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

# GameServer中用到的全局数据信息
gamemgr_proxy = None # 同GameManager通信的Proxy
dbmgr_proxy = None # 同DBManager通信的Proxy
dbmgr_proxy_set = []
dbmgr_proxy_external = {}
online_games = [] # 当前在线的GameServer的ip和端口信息
game_client_mgr = None # GameServer的客户的Proxy信息
game_info = None # 当前GameServer的ip和端口信息
game_event_callback = None # 全局事件的回调
global_entities = {} # 分布式全局entity的地址信息
game = None # GameServer游戏服务器
game_name = '' # 游戏服务器的名字
dbname = 'nyxdb' # 默认数据库名字
dbname_external = {}
def_savetime = None # 对Entity对象的默认存储时间
game_service = None
proto_encoder = None # 默认协议Encoder
gameid = None # 游戏id
hostnum = None # 服务器组id
platform_conf = None # 平台相关配置信息
monitor_mgr = None # 远程监控接口

_gamemgr_callbacks = {} # GameServer调用GameManager的回调函数
_dbmgr_callbacks = {} # GameServer调用DBManager的回调函数
_gate_callbacks = {} # GameServer调用Gate的回调函数

class _RotatedIdGenerator(object):
    """自增id生成器"""
    _maxid = 2 ** 20
    def __init__(self):
        self._id = 1

    def gen_id(self):
        if self._id != _RotatedIdGenerator._maxid:
            self._id += 1
        else:
            self._id = 1
        return (game_info.sid << 20) + self._id

_gamemgr_id_generator = _RotatedIdGenerator()
def reg_gamemgr_callback(callback):
    callback_id = _gamemgr_id_generator.gen_id()
    _gamemgr_callbacks[callback_id] = callback
    return callback_id

def unreg_gamemgr_callback(callback_id):
    return _gamemgr_callbacks.pop(callback_id, None)

_dbmgr_id_generator = _RotatedIdGenerator()
def reg_dbmgr_callback(callback):
    callback_id = _dbmgr_id_generator.gen_id()
    _dbmgr_callbacks[callback_id] = callback
    return callback_id

def unreg_dbmgr_callback(callback_id):
    return _dbmgr_callbacks.pop(callback_id, None)

_gate_id_generator = _RotatedIdGenerator()
def reg_gate_callback(callback):
    callback_id = _gate_callbacks.gen_id()
    _gate_callbacks[callback_id] = callback
    return callback_id

def unreg_gate_callback(callback_id):
    return _gate_callbacks.pop(callback_id, None)
