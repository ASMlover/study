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

import time
import datetime

_DAILY_REFRESH_TIME = 0

def parse_date(s):
    """parse string mode date

    date format must be `YYYY-mm-dd`, return the start seconds of day
    """
    if '-' in s:
        sd = s.split('-')
    else:
        raise ValueError('invalid date: %s' % s)

    if len(sd) != 3:
        raise ValueError('invalid date: %s' % s)

    y, m, d = map(int, sd)
    return time.mktime(datetime.datetime(y, m, d).timetuple())

def parse_time(s):
    """parse string mode time

    time format must be `HH:MM` or `HH:MM:SS`, return the seconds in day
    """
    if ':' in s:
        st = map(int, s.split(':'))
    else:
        raise ValueError('invalid time: %s' % s)

    if len(st) == 3:
        h, m, s = st
    elif len(st) == 2:
        h, m = st
        s = 0
    else:
        raise ValueError('invalid time: %s' % s)

    return h * 3600 + m * 60 + s

def _get_time():
    """get current time(seconds)

    it's should be add diff-time of server network delay
    """
    return time.time()

def get_day_time(t=None, refresh_time=None):
    """get start seconds of day"""
    if t is None:
        t = _get_time()
    if refresh_time is None:
        refresh_time = _DAILY_REFRESH_TIME

    day_time = (int(t - time.timezone - refresh_time) // 86400) * 86400 + time.timezone + refresh_time
    return day_time

def get_time_of_day(t=None, refresh_time=None):
    """get the seconds in day"""
    if t is None:
        t = _get_time()
    return t - get_day_time(t, refresh_time)

def is_same_day(t1, t2, refresh_time=None):
    """check two times in same day"""
    return get_day_time(t1, refresh_time) == get_day_time(t2, refresh_time)
