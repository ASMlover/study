// Copyright (c) 2017 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <iostream>
#include "color.h"

void show_color(void) {
#define CLRPRINT(c) color_printf((c), #c); printf("\n")
#define CLRBACK(c) color_printf(ColorType::FORE_BLACK, (c), #c); printf("\n")
#define CLRPRINT2(fc, bc) color_printf((fc), (bc), #fc " - " #bc); printf("\n")

  fprintf(stdout, "\n######################## show foreground color\n");
  color_printf(ColorType::FORE_BLACK,
      ColorType::BACK_WHITE, "ColorType::FORE_BLACK"); printf("\n");
  CLRPRINT(ColorType::FORE_BLUE);
  CLRPRINT(ColorType::FORE_LIGHTBLUE);
  CLRPRINT(ColorType::FORE_GREEN);
  CLRPRINT(ColorType::FORE_LIGHTGREEN);
  CLRPRINT(ColorType::FORE_CYAN);
  CLRPRINT(ColorType::FORE_LIGHTCYAN);
  CLRPRINT(ColorType::FORE_RED);
  CLRPRINT(ColorType::FORE_LIGHTRED);
  CLRPRINT(ColorType::FORE_MAGENTA);
  CLRPRINT(ColorType::FORE_LIGHTMAGENTA);
  CLRPRINT(ColorType::FORE_YELLOW);
  CLRPRINT(ColorType::FORE_LIGHTYELLOW);
  CLRPRINT(ColorType::FORE_WHITE);
  CLRPRINT(ColorType::FORE_LIGHTWHITE);
  CLRPRINT(ColorType::FORE_GRAY);

  fprintf(stdout, "\n######################## show background color\n");
  CLRPRINT(ColorType::BACK_BLACK);
  CLRBACK(ColorType::BACK_BLUE);
  CLRBACK(ColorType::BACK_LIGHTBLUE);
  CLRBACK(ColorType::BACK_GREEN);
  CLRBACK(ColorType::BACK_LIGHTGREEN);
  CLRBACK(ColorType::BACK_CYAN);
  CLRBACK(ColorType::BACK_LIGHTCYAN);
  CLRBACK(ColorType::BACK_RED);
  CLRBACK(ColorType::BACK_LIGHTRED);
  CLRBACK(ColorType::BACK_MAGENTA);
  CLRBACK(ColorType::BACK_LIGHTMAGENTA);
  CLRBACK(ColorType::BACK_YELLOW);
  CLRBACK(ColorType::BACK_LIGHTYELLOW);
  CLRBACK(ColorType::BACK_WHITE);
  CLRBACK(ColorType::BACK_LIGHTWHITE);
  CLRBACK(ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground black color\n");
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_BLACK, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground blue color\n");
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_BLUE, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light blue color\n");
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTBLUE, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground green color\n");
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_GREEN, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light green color\n");
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTGREEN, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground cyan color\n");
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_CYAN, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light cyan color\n");
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTCYAN, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground red color\n");
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_RED, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light red color\n");
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTRED, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground magenta color\n");
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_MAGENTA, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light magenta color\n");
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTMAGENTA, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground yellow color\n");
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_YELLOW, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light yellow color\n");
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_LIGHTYELLOW, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground white color\n");
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_LIGHTWHITE);
  CLRPRINT2(ColorType::FORE_WHITE, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground light white color\n");
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_LIGHTWHITE, ColorType::BACK_GRAY);

  fprintf(stdout, "\n######################## show foreground gray color\n");
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_BLACK);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_BLUE);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTBLUE);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_GREEN);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTGREEN);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_CYAN);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTCYAN);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_RED);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTRED);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_MAGENTA);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTMAGENTA);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_YELLOW);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTYELLOW);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_WHITE);
  CLRPRINT2(ColorType::FORE_GRAY, ColorType::BACK_LIGHTWHITE);
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  show_color();

  return 0;
}
