#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2021 ASMlover. All rights reserved.
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

import colorful as _C

def main():
    _C.xprint(_C.Foreground.BLUE, f"Colorful.Foreground.BLUE")
    _C.xprint(_C.Foreground.GREEN, f"Colorful.Foreground.GREEN")
    _C.xprint(_C.Foreground.CYAN, f"Colorful.Foreground.CYAN")
    _C.xprint(_C.Foreground.RED, f"Colorful.Foreground.RED")
    _C.xprint(_C.Foreground.MAGENTA, f"Colorful.Foreground.MAGENTA")
    _C.xprint(_C.Foreground.YELLOW, f"Colorful.Foreground.YELLOW")
    _C.xprint(_C.Foreground.WHITE, f"Colorful.Foreground.WHITE")
    _C.xprint(_C.Foreground.GRAY, f"Colorful.Foreground.GRAY")

    _C.xprint(_C.Foreground.LIGHTBLUE, f"Colorful.Foreground.LIGHTBLUE")
    _C.xprint(_C.Foreground.LIGHTGREEN, f"Colorful.Foreground.LIGHTGRAY")
    _C.xprint(_C.Foreground.LIGHTCYAN, f"Colorful.Foreground.LIGHTCYAN")
    _C.xprint(_C.Foreground.LIGHTRED, f"Colorful.Foreground.LIGHTRED")
    _C.xprint(_C.Foreground.LIGHTMAGENTA, f"Colorful.Foreground.LIGHTMAGENTA")
    _C.xprint(_C.Foreground.LIGHTYELLOW, f"Colorful.Foreground.LIGHTYELLOW")
    _C.xprint(_C.Foreground.LIGHTWHITE, f"Colorful.Foreground.LIGHTWHITE")

if __name__ == '__main__':
    main()
