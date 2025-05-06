// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include <boost/container/vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <iostream>

struct animal {
  boost::container::vector<animal> children;
};

static void boost_container_recursive_containers() noexcept {
  std::cout << "--------- [container.recursive_containers] ---------" << std::endl;

  animal parent, child1, child2;
  parent.children.push_back(child1);
  parent.children.push_back(child2);
}

static void boost_container_stable_vector() noexcept {
  std::cout << "--------- [container.stable_vector] ---------" << std::endl;
  using namespace boost::container;

  stable_vector<int> v(2, 1);
  int& i = v[1];
  v.erase(v.begin());
  std::cout << "[demo.container] " << i << std::endl;
}

void boost_container() noexcept {
  std::cout << "========= [container] =========" << std::endl;

  boost_container_recursive_containers();
  boost_container_stable_vector();
}
