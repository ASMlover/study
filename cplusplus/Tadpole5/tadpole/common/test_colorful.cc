// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
  using namespace tadpole;

  std::cout << colorful::fg::black << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::red << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::green << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::yellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::blue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::magenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::cyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::white << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::gray << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightred << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightgreen << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightyellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightblue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightmagenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightcyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::fg::lightwhite << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::reset;

  std::cout << colorful::bg::black << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::red << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::green << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::yellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::blue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::magenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::cyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::white << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::gray << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightred << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightgreen << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightyellow << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightblue << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightmagenta << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightcyan << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::bg::lightwhite << "Hello, Tadpole Colorful !!!" << std::endl;
  std::cout << colorful::reset;
}
