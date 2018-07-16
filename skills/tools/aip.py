#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
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

# 基金定投计算根据

from __future__ import print_function
import sys

def automatic_investment_plan(rate, year=10, month_money=1000):
    money = 12 * month_money
    all_monry = money * (1 + rate) * (-1 + (1 + rate) ** year) / rate
    return money * year / 10000.0, all_monry / 10000.0

if __name__ == '__main__':
    if len(sys.argv) == 2:
        rate = float(sys.argv[1])
        print (automatic_investment_plan(rate))
    elif len(sys.argv) == 3:
        rate = float(sys.argv[1])
        year = int(sys.argv[2])
        print (automatic_investment_plan(rate, year))
    elif len(sys.argv) == 4:
        rate = float(sys.argv[1])
        year = int(sys.argv[2])
        month_money = int(sys.argv[3])
        print (automatic_investment_plan(rate, year, month_money))
    else:
        print ('ERROR')
