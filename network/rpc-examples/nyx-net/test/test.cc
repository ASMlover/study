// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <mutex>
#include "../core/utils/ThreadPool.hh"

std::mutex _mutex;

void run_task(const std::string& prompt, int job, int t) {
  std::unique_lock<std::mutex> g(_mutex);
  std::cout
    << "[" << std::this_thread::get_id() << "] - "
    << prompt << " : " << job << ", sleep " << t << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(t));
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  nyx::utils::ThreadPool p;
  std::srand((unsigned int)std::time(nullptr));
  for (auto i = 0; i < 10; ++i) {
    auto n = std::rand() % 100;
    p.post([i, n]{ run_task("task job", i, n); });
  }

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line == "exit")
      break;
  }
  p.shutoff();

  return 0;
}
