#!/usr/bin/env python
# -*- coding: UTF-8 -*-
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
import sys
sys.path.append('..')

import pygame
from engine.entity import Entity

class Game(Entity):
    def on_init(self):
        self._font = pygame.font.SysFont('Courier New', 13)
        self._font_height = self._font.get_linesize()
        self._event_text = []

    def on_update(self, event):
        self._event_text.append(str(event))
        index = int(-self.get_height() / self._font_height)
        self._event_text = self._event_text[index:]

        self._screen.fill((0, 128, 128))
        y = self.get_height() - self._font_height
        for text in reversed(self._event_text):
            self._screen.blit(self._font.render(text, True, (0, 0, 0)), (0, y))
            y -= self._font_height

_game = Game(caption='DEMO')
def main():
    _game.setup()
    _game.loop()

if __name__ == '__main__':
    main()
