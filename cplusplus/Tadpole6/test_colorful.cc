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
#include "harness.hh"
#include "colorful.hh"

TADPOLE_TEST(TadpoleColorful) {
  using namespace tadpole::colorful;

  std::cout << fg::black << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::red << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::green << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::yellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::blue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::magenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::cyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::white << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::gray << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightred << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightgreen << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightyellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightblue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightmagenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightcyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << fg::lightwhite << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << reset;

  std::cout << bg::black << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::red << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::green << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::yellow << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::blue << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::magenta << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::cyan << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::white << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::gray << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightred << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightgreen << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightyellow << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightblue << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightmagenta << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightcyan << "Hello, Tadpole Colorful !!!" << reset << std::endl;
  std::cout << bg::lightwhite << "Hello, Tadpole Colorful !!!" << reset << std::endl;
}
