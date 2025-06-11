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
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <iostream>

static void boost_property_tree_accessing_data() noexcept {
  std::cout << "--------- [property_tree.accessing_data] ---------" << std::endl;
  using boost::property_tree::ptree;

  ptree pt;
  pt.put("C:.Windows.System", "20 files");

  ptree& c = pt.get_child("C:");
  ptree& windows = c.get_child("Windows");
  ptree& system = windows.get_child("System");
  std::cout << "[demo.property_tree] " << system.get_value<std::string>() << std::endl;
}

static void boost_property_tree_accessing_data_in_basic_ptree() noexcept {
  std::cout << "--------- [property_tree.accessing_data_in_basic_ptree] ---------" << std::endl;
  using ptree = boost::property_tree::basic_ptree<std::string, int>;

  ptree pt;
  pt.put(ptree::path_type{"C:\\Windows\\System", '\\'}, 20);
  pt.put(ptree::path_type{"C:\\Windows\\Cursors", '\\'}, 50);

  ptree& windows = pt.get_child(ptree::path_type{"C:\\Windows", '\\'});
  int files = 0;
  for (const std::pair<std::string, ptree>& p : windows)
    files += p.second.get_value<int>();
  std::cout << "[demo.property_tree] " << files << std::endl;
}

void boost_property_tree() noexcept {
  std::cout << "========= [property_tree] =========" << std::endl;

  boost_property_tree_accessing_data();
  boost_property_tree_accessing_data_in_basic_ptree();
}
