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
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <cstddef>
#include <string>
#include <iostream>

template <typename T> inline void hash_combine(std::size_t& seed, const T& v) noexcept {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

static void boost_unordered_using_unordered_set() noexcept {
  std::cout << "--------- [unordered.using_unordered_set] ---------" << std::endl;
  using unordered_set = boost::unordered_set<std::string>;

  unordered_set set;
  set.emplace("cat");
  set.emplace("shark");
  set.emplace("spider");

  for (const std::string& s : set)
    std::cout << "[demo.unordered] " << s << std::endl;

  std::cout << "[demo.unordered] " << set.size() << std::endl;
  std::cout << "[demo.unordered] " << set.max_size() << std::endl;

  std::cout << "[demo.unordered] " << std::boolalpha << (set.find("cat") != set.end()) << std::endl;
  std::cout << "[demo.unordered] " << set.count("shark") << std::endl;
}

static void boost_unordered_using_unordered_map() noexcept {
  std::cout << "--------- [unordered.using_unordered_map] ---------" << std::endl;
  using unordered_map = boost::unordered_map<std::string, int>;

  unordered_map map;
  map.emplace("cat", 4);
  map.emplace("shark", 0);
  map.emplace("spider", 8);

  for (const auto& p : map)
    std::cout << "[demo.unordered] " << p.first << ";" << p.second << std::endl;

  std::cout << "[demo.unordered] " << map.size() << std::endl;
  std::cout << "[demo.unordered] " << map.max_size() << std::endl;

  std::cout << "[demo.unordered] " << std::boolalpha << (map.find("cat") != map.end()) << std::endl;
  std::cout << "[demo.unordered] " << map.count("shark") << std::endl;
}

struct animal {
  std::string name;
  int legs;
};
inline bool operator==(const animal& lhs, const animal& rhs) noexcept {
  return lhs.name == rhs.name && lhs.legs == rhs.legs;
}
inline std::size_t hash_value(const animal& a) {
  std::size_t seed = 0;
  hash_combine(seed, a.name);
  hash_combine(seed, a.legs);
  return seed;
}
static void boost_unordered_user_defined() noexcept {
  std::cout << "--------- [unordered.user_defined] ---------" << std::endl;
  using unordered_set = boost::unordered_set<animal>;

  unordered_set animals;
  animals.insert({"cat", 4});
  animals.insert({"shark", 0});
  animals.insert({"spider", 8});
}

void boost_unordered() noexcept {
  std::cout << "========= [unordered] =========" << std::endl;

  boost_unordered_using_unordered_set();
  boost_unordered_using_unordered_map();
  boost_unordered_user_defined();
}
