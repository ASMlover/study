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
#pragma once

#include <functional>
#include <map>
#include <sstream>

namespace consistent_hash {

class HashNode {
  int replicas_{};
  std::string name_;
public:
  HashNode(void) {
  }

  HashNode(int replicas, const std::string& name)
    : replicas_(replicas), name_(name) {
  }

  std::string get_name(void) const {
    return name_;
  }

  int get_replicas(void) const {
    return replicas_;
  }

  bool operator==(const HashNode& other) const {
    return name_ == other.name_ && replicas_ == other.replicas_;
  }
};

inline std::size_t __get_hash(const HashNode& n, int i) {
  return std::hash<std::string>{}(n.get_name() + std::to_string(i));
}

inline std::size_t __get_hash(const std::string& s, int i) {
  return std::hash<std::string>{}(s + std::to_string(i));
}

class HashRing {
  std::map<std::int64_t, HashNode> ring_;

  HashRing(const HashRing&) = delete;
  HashRing& operator=(const HashRing&) = delete;
public:
  HashRing(void) {
  }

  void insert_node(const HashNode& node) {
    auto name = node.get_name();
    for (auto i = 0; i < node.get_replicas(); ++i) {
      auto hash_code = __get_hash(name, i);
      ring_[hash_code] = node;
    }
  }

  void remove_node(const HashNode& node) {
    auto name = node.get_name();
    for (auto i = 0; i < node.get_replicas(); ++i) {
      auto hash_code = __get_hash(name, i);
      ring_.erase(hash_code);
    }
  }

  HashNode get_node(const std::string& k) const {
    auto hash_code = std::hash<std::string>{}(k);
    if (ring_.empty())
      return HashNode();

    auto it = ring_.lower_bound(hash_code);
    if (it == ring_.end())
      it = ring_.begin();
    return it->second;
  }
};

}
