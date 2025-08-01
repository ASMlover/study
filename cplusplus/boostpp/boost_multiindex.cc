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
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

struct animal {
  std::string name;
  int legs;
  bool has_tail;
};

class animals_container {
public:
  void add(animal a) noexcept {}
  const animal* find_by_name(const std::string& name) const noexcept { return nullptr; }
  std::vector<animal> find_by_legs(int from, int to) const noexcept { return {}; }
  std::vector<animal> find_by_tail(bool has_tail) const noexcept { return {}; }
};

class Animal {
  std::string name_;
  int legs_;
public:
  Animal(std::string name, int legs) noexcept : name_{std::move(name)}, legs_{legs} {}
  bool operator<(const Animal& a) const noexcept { return legs_ < a.legs_; }
  const std::string& name() const noexcept { return name_; }
  int legs() const noexcept { return legs_; }
};
static void boost_multiindex_multi_index_container() noexcept {
  std::cout << "--------- [multiindex.multi_index_container] ---------" << std::endl;
  using namespace boost::multi_index;

  using animal_multi = multi_index_container<
    animal, indexed_by<hashed_non_unique<member<animal, std::string, &animal::name>>,
    hashed_non_unique<member<animal, int, &animal::legs>>>>;

  animal_multi animals;
  animals.insert({"cat", 4});
  animals.insert({"shark", 0});
  animals.insert({"spider", 8});
  std::cout << "[demo.multiindex] " << animals.count("cat") << std::endl;

  const animal_multi::nth_index<1>::type& legs_index = animals.get<1>();
  std::cout << "[demo.multiindex] " << legs_index.count(8) << std::endl;
}

static void boost_multiindex_changing_elements() noexcept {
  std::cout << "-------- [multiindex.changing_elements] ---------" << std::endl;
  using namespace boost::multi_index;

  using animal_multi = multi_index_container<
    animal, indexed_by<hashed_non_unique<member<animal, std::string, &animal::name>>,
    hashed_non_unique<member<animal, int, &animal::legs>>>>;

  animal_multi animals;
  animals.insert({"cat", 4});
  animals.insert({"shark", 0});
  animals.insert({"spider", 8});

  auto& legs_index = animals.get<1>();
  auto it = legs_index.find(4);
  legs_index.modify(it, [](animal& a) { a.name = "dog"; });
  std::cout << "[demo.multiindex] " << animals.count("dog") << std::endl;
}

static void boost_multiindex_container_hashed_unique() noexcept {
  std::cout << "--------- [multiindex.container_hashed_unique] ---------" << std::endl;
  using namespace boost::multi_index;

  using animal_multi = multi_index_container<
    animal, indexed_by<hashed_non_unique<member<animal, std::string, &animal::name>>,
    hashed_unique<member<animal, int, &animal::legs>>>>;

  animal_multi animals;
  animals.insert({"cat", 4});
  animals.insert({"shark", 0});
  animals.insert({"dog", 4});

  auto& legs_index = animals.get<1>();
  std::cout << "[demo,multiindex] " << legs_index.count(4) << std::endl;
}

static void boost_multiindex_interfaces() noexcept {
  std::cout << "--------- [multiindex.interfaces] ---------" << std::endl;
  using namespace boost::multi_index;

  using animal_multi = multi_index_container<
    animal, indexed_by<sequenced<>, ordered_non_unique<member<animal, int, &animal::legs>>,
    random_access<>>>;

  animal_multi animals;
  animals.push_back({"cat", 4});
  animals.push_back({"shark", 0});
  animals.push_back({"spider", 8});

  auto& legs_index = animals.get<1>();
  auto it = legs_index.lower_bound(4);
  auto end = legs_index.upper_bound(8);
  for (; it != end; ++it)
    std::cout << "[demo.multiindex] " << it->name << std::endl;

  const auto& rand_index = animals.get<2>();
  std::cout << "[demo.multiindex] " << rand_index[0].name << std::endl;
}

static void boost_multiindex_extractors() noexcept {
  std::cout << "--------- [multiindex.extractors] ---------" << std::endl;
  using namespace boost::multi_index;

  using animal_multi = multi_index_container<
    Animal, indexed_by<ordered_unique<identity<Animal>>,
    hashed_unique<const_mem_fun<Animal, const std::string&, &Animal::name>>>>;

  animal_multi animals;
  animals.emplace("cat", 4);
  animals.emplace("shark", 0);
  animals.emplace("spider", 8);
  std::cout << "[demo.multiindex] " << animals.begin()->name() << std::endl;

  const auto& name_index = animals.get<1>();
  std::cout << "[demo.multiindex] " << name_index.count("shark") << std::endl;
}

static void boost_multiindex_animals_container() noexcept {
  std::cout << "--------- [multiindex.animals_container] ---------" << std::endl;

  animals_container animals;
  animals.add({"cat", 4, true});
  animals.add({"ant", 6, false});
  animals.add({"spider", 8, false});
  animals.add({"shark", 0, false});

  const animal* a = animals.find_by_name("cat");
  if (a)
    std::cout << "[demo.multiindex] cat has " << a->legs << " legs" << std::endl;

  auto animals_with_6_to_8_legs = animals.find_by_legs(6, 9);
  for (auto a : animals_with_6_to_8_legs)
    std::cout << "[demo.multiindex] " << a.name << " has " << a.legs << " legs" << std::endl;

  auto animals_without_tail = animals.find_by_tail(false);
  for (auto a : animals_without_tail)
    std::cout << "[demo.multiindex] " << a.name << " has no tail" << std::endl;
}

void boost_multiindex() noexcept {
  std::cout << "========= [multiindex] =========" << std::endl;

  boost_multiindex_multi_index_container();
  boost_multiindex_changing_elements();
  boost_multiindex_container_hashed_unique();
  boost_multiindex_interfaces();
  boost_multiindex_extractors();
  boost_multiindex_animals_container();
}
