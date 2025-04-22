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
#include <boost/intrusive/list.hpp>
#include <string>
#include <utility>
#include <iostream>

using mode = boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

struct animal : public boost::intrusive::list_base_hook<> {
  std::string name;
  int legs;
  animal(std::string n, int l) noexcept : name{std::move(n)}, legs{l} {}
};

struct Animal : public boost::intrusive::list_base_hook<mode> {
  std::string name;
  int legs;
  Animal(std::string n, int l) noexcept : name{std::move(n)}, legs{l} {}
};

static void boost_intrusive_using_intrusive_list() noexcept {
  std::cout << "--------- [intrusive.using_intrusive_list] ---------" << std::endl;

  animal a1{"cat", 4};
  animal a2{"shark", 0};
  animal a3{"spider", 8};

  using animal_list = boost::intrusive::list<animal>;
  animal_list animals;

  animals.push_back(a1);
  animals.push_back(a2);
  animals.push_back(a3);
  a1.name = "dog";

  for (const animal& a : animals)
    std::cout << "[demo.intrusive] " << a.name << std::endl;
}

static void boost_intrusive_remove_and_destroy() noexcept {
  std::cout << "--------- [intrusive.remove_and_destroy] ---------" << std::endl;

  animal a1{"cat", 4};
  animal a2{"shark", 0};
  animal* a3 = new animal{"spider", 8};

  using animal_list = boost::intrusive::list<animal>;
  animal_list animals;

  animals.push_back(a1);
  animals.push_back(a2);
  animals.push_back(*a3);

  animals.pop_back();
  delete a3;

  for (const animal& a : animals)
    std::cout << "[demo.intrusive] " << a.name << std::endl;
}

static void boost_intrusive_pop_back_and_dispose() noexcept {
  std::cout << "--------- [intrusive.pop_back_and_dispose] ---------" << std::endl;

  animal a1{"cat", 4};
  animal a2{"shark", 0};
  animal* a3 = new animal{"spider", 8};

  using animal_list = boost::intrusive::list<animal>;
  animal_list animals;

  animals.push_back(a1);
  animals.push_back(a2);
  animals.push_back(*a3);

  animals.pop_back_and_dispose([](animal* a) { delete a; });
  for (const animal& a : animals)
    std::cout << "[demo.intrusive] " << a.name << std::endl;
}

static void boost_intrusive_auto_unlink_mode() noexcept {
  std::cout << "--------- [intrusive.auto_unlink_mode] ---------" << std::endl;

  Animal a1{"cat", 4};
  Animal a2{"shark", 0};
  Animal* a3 = new Animal{"spider", 8};

  using constant_time_size = boost::intrusive::constant_time_size<false>;
  using animal_list = boost::intrusive::list<Animal, constant_time_size>;
  animal_list animals;

  animals.push_back(a1);
  animals.push_back(a2);
  animals.push_back(*a3);

  delete a3;

  for (const Animal& a : animals)
    std::cout << "[demo.intrusive] " << a.name << std::endl;
}

void boost_intrusive() noexcept {
  std::cout << "========= [intrusive] =========" << std::endl;

  boost_intrusive_using_intrusive_list();
  boost_intrusive_remove_and_destroy();
  boost_intrusive_pop_back_and_dispose();
  boost_intrusive_auto_unlink_mode();
}
