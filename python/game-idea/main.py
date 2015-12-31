#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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
import trigger

class Space(object):
    def __init__(self):
        pass

    def monster_count(self):
        return 0

class TriggerManager(object):
    def __init__(self, space=None):
        self.space = space
        self.triggers = {}

    def register(self, trigger_no, infos):
        trigger_name = 'Trigger%d' % trigger_no
        trigger_type = getattr(trigger, trigger_name)
        if trigger_type:
            self.triggers[trigger_no] = trigger_type(self.space, infos)

    def unregister(self, trigger_no):
        self.triggers.pop(trigger_no, None)

    def on_event_notify(self, notify, *args):
        for _, trigger in self.triggers.items():
            on_event = getattr(trigger, notify, None)
            if on_event:
                on_event(*args)

if __name__ == '__main__':
    space = Space()
    trigger_mgr = TriggerManager(space)
    trigger_mgr.register(1101, {'cond': 0, 'action': 'all monsters dead !!!'})

    trigger_mgr.on_event_notify('on_monster_die')
