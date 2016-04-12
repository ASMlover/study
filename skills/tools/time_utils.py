#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

DEFAULT_REFRESH_TIME = 5 * 3600 # 默认刷新时间
DAY_SECONDS = 24 * 3600         # 每天的秒数
WEEKLY_REFRESH_WDAY = 0         # 0 ~ 6, 周一 ~ 周日

def getdaytime(t, refresh_time=None):
    if refresh_time is None:
        refresh_time = DEFAULT_REFRESH_TIME
    return (int(t - time.timezone - refresh_time) // DAY_SECONDS) * DAY_SECONDS + time.timezone + refresh_time

def gettimeofday(timestamp=None):
    t = timestamp and timestamp or time.time()
    return t - getdaytime(t, 0)

def issameday(t1, t2, refresh_time=None):
    if refresh_time is None:
        refresh_time = DEFAULT_REFRESH_TIME

    return getdaytime(t1, refresh_time) == getdaytime(t2, refresh_time)

def getweektime(t, refresh_wday=None, refresh_time=None):
    if refresh_wday is None:
        refresh_wday = WEEKLY_REFRESH_WDAY
    if refresh_time is None:
        refresh_time = DEFAULT_REFRESH_TIME

    weekly_offset = refresh_wday * DAY_SECONDS + refresh_time
    # utc0是周四也就是wday=3
    utc_weekly_offset = 3 * DAY_SECONDS
    week_time = 7 * DAY_SECONDS
    return int(t - time.timezone - weekly_offset + utc_weekly_offset) // week_time * week_time + time.timezone + weekly_offset - utc_weekly_offset

def issameweek(t1, t2, refresh_wday=None, refresh_time=None):
    return getweektime(t1, refresh_wday, refresh_time) == getweektime(t2, refresh_wday, refresh_time)

def getweekday(t=None, refresh_time=None):
    day_time = getdaytime(t or time.time(), refresh_time)
    return time.localtime(day_time).tm_wday + 1
