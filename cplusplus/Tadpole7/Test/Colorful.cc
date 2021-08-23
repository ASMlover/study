// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Common/Harness.hh>
#include <Common/Colorful.hh>

TADPOLE_TEST(TadpoleColorful) {
  using namespace Tadpole::Common::Colorful;

#define CLR(c)\
  std::cout << c << "Hello, Tadpole Colorful - with `" #c "` !!!" << reset << std::endl
#define CLRF(c)\
  std::cout << c << bg::black << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::red << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::green << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::yellow << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::blue << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::magenta << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::cyan << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::white << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::gray << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightred << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightgreen << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightyellow << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightblue << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightmagenta << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightcyan << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << c << bg::lightwhite << "Hello, Tadpole Colorful - foreground with `" #c "` !!!" << std::endl;\
  std::cout << reset

  CLR(fg::black);
  CLR(fg::red);
  CLR(fg::green);
  CLR(fg::yellow);
  CLR(fg::blue);
  CLR(fg::magenta);
  CLR(fg::cyan);
  CLR(fg::white);
  CLR(fg::gray);
  CLR(fg::lightred);
  CLR(fg::lightgreen);
  CLR(fg::lightyellow);
  CLR(fg::lightblue);
  CLR(fg::lightmagenta);
  CLR(fg::lightcyan);
  CLR(fg::lightwhite);

  CLR(bg::black);
  CLR(bg::red);
  CLR(bg::green);
  CLR(bg::yellow);
  CLR(bg::blue);
  CLR(bg::magenta);
  CLR(bg::cyan);
  CLR(bg::white);
  CLR(bg::gray);
  CLR(bg::lightred);
  CLR(bg::lightgreen);
  CLR(bg::lightyellow);
  CLR(bg::lightblue);
  CLR(bg::lightmagenta);
  CLR(bg::lightcyan);
  CLR(bg::lightwhite);

  CLRF(fg::black);
  CLRF(fg::red);
  CLRF(fg::green);
  CLRF(fg::yellow);
  CLRF(fg::blue);
  CLRF(fg::magenta);
  CLRF(fg::cyan);
  CLRF(fg::white);
  CLRF(fg::gray);
  CLRF(fg::lightred);
  CLRF(fg::lightgreen);
  CLRF(fg::lightyellow);
  CLRF(fg::lightblue);
  CLRF(fg::lightmagenta);
  CLRF(fg::lightcyan);
  CLRF(fg::lightwhite);
}
