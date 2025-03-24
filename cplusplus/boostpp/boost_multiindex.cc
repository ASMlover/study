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
#include <string>
#include <iostream>

struct animal {
  std::string name;
  int legs;
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

void boost_multiindex() noexcept {
  std::cout << "========= [multiindex] =========" << std::endl;

  boost_multiindex_multi_index_container();
  boost_multiindex_changing_elements();
  boost_multiindex_container_hashed_unique();
}
