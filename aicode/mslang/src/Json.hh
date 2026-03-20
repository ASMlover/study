// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <cmath>
#include <format>
#include <string>
#include <variant>
#include <vector>
#include "Types.hh"

namespace ms::json {

class Value;
using Array = std::vector<Value>;
using Object = std::vector<std::pair<str_t, Value>>;

class Value {
  std::variant<nil_t, bool, double, i64_t, str_t, Array, Object> data_;

public:
  Value() noexcept : data_(nullptr) {}
  Value(nil_t) noexcept : data_(nullptr) {}
  Value(bool v) noexcept : data_(v) {}
  Value(double v) noexcept : data_(v) {}
  Value(int v) noexcept : data_(static_cast<i64_t>(v)) {}
  Value(i64_t v) noexcept : data_(v) {}
  Value(str_t v) noexcept : data_(std::move(v)) {}
  Value(cstr_t v) noexcept : data_(str_t(v)) {}
  Value(Array v) noexcept : data_(std::move(v)) {}
  Value(Object v) noexcept : data_(std::move(v)) {}

  static inline Value array() { return Value(Array{}); }
  static inline Value object() { return Value(Object{}); }

  inline bool is_null() const noexcept { return std::holds_alternative<nil_t>(data_); }
  inline bool is_bool() const noexcept { return std::holds_alternative<bool>(data_); }
  inline bool is_double() const noexcept { return std::holds_alternative<double>(data_); }
  inline bool is_int() const noexcept { return std::holds_alternative<i64_t>(data_); }
  inline bool is_number() const noexcept { return is_double() || is_int(); }
  inline bool is_string() const noexcept { return std::holds_alternative<str_t>(data_); }
  inline bool is_array() const noexcept { return std::holds_alternative<Array>(data_); }
  inline bool is_object() const noexcept { return std::holds_alternative<Object>(data_); }

  inline bool as_bool() const { return std::get<bool>(data_); }
  inline double as_double() const {
    if (is_int()) return static_cast<double>(std::get<i64_t>(data_));
    return std::get<double>(data_);
  }
  inline i64_t as_int() const {
    if (is_double()) return static_cast<i64_t>(std::get<double>(data_));
    return std::get<i64_t>(data_);
  }
  inline const str_t& as_string() const { return std::get<str_t>(data_); }
  inline const Array& as_array() const { return std::get<Array>(data_); }
  inline const Object& as_object() const { return std::get<Object>(data_); }

  inline void push(Value v) { std::get<Array>(data_).push_back(std::move(v)); }
  inline sz_t size() const {
    if (is_array()) return std::get<Array>(data_).size();
    if (is_object()) return std::get<Object>(data_).size();
    return 0;
  }
  inline const Value& at(sz_t i) const { return std::get<Array>(data_)[i]; }

  inline Value& operator[](const str_t& key) {
    auto& obj = std::get<Object>(data_);
    for (auto& [k, v] : obj) {
      if (k == key) return v;
    }
    obj.emplace_back(key, Value{});
    return obj.back().second;
  }

  inline const Value& operator[](const str_t& key) const {
    auto& obj = std::get<Object>(data_);
    for (auto& [k, v] : obj) {
      if (k == key) return v;
    }
    static Value null_value;
    return null_value;
  }

  inline bool has(const str_t& key) const {
    if (!is_object()) return false;
    auto& obj = std::get<Object>(data_);
    for (auto& [k, v] : obj) {
      if (k == key) return true;
    }
    return false;
  }

  static inline str_t escape(const str_t& s) {
    str_t r;
    r.reserve(s.size() + 2);
    r += '"';
    for (char c : s) {
      switch (c) {
      case '"':  r += "\\\""; break;
      case '\\': r += "\\\\"; break;
      case '\b': r += "\\b"; break;
      case '\f': r += "\\f"; break;
      case '\n': r += "\\n"; break;
      case '\r': r += "\\r"; break;
      case '\t': r += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20)
          r += std::format("\\u{:04x}", static_cast<unsigned>(c));
        else
          r += c;
      }
    }
    r += '"';
    return r;
  }

  inline str_t serialize() const {
    if (is_null()) return "null";
    if (is_bool()) return as_bool() ? "true" : "false";
    if (is_int()) return std::to_string(as_int());
    if (is_double()) {
      double d = std::get<double>(data_);
      if (std::floor(d) == d && std::abs(d) < 1e15)
        return std::to_string(static_cast<i64_t>(d));
      return std::format("{}", d);
    }
    if (is_string()) return escape(as_string());
    if (is_array()) {
      str_t r = "[";
      bool first = true;
      for (auto& v : as_array()) {
        if (!first) r += ',';
        first = false;
        r += v.serialize();
      }
      r += ']';
      return r;
    }
    if (is_object()) {
      str_t r = "{";
      bool first = true;
      for (auto& [k, v] : as_object()) {
        if (!first) r += ',';
        first = false;
        r += escape(k);
        r += ':';
        r += v.serialize();
      }
      r += '}';
      return r;
    }
    return "null";
  }

  // --- JSON Parser ---
  static inline Value parse(strv_t input) {
    sz_t pos = 0;
    return parse_value(input, pos);
  }

private:
  static inline void skip_ws(strv_t s, sz_t& p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r')) ++p;
  }

  static inline Value parse_value(strv_t s, sz_t& p) {
    skip_ws(s, p);
    if (p >= s.size()) return Value{};
    char c = s[p];
    if (c == '"') return parse_string(s, p);
    if (c == '{') return parse_object(s, p);
    if (c == '[') return parse_array(s, p);
    if (c == 't' && s.substr(p, 4) == "true") { p += 4; return Value(true); }
    if (c == 'f' && s.substr(p, 5) == "false") { p += 5; return Value(false); }
    if (c == 'n' && s.substr(p, 4) == "null") { p += 4; return Value{}; }
    if (c == '-' || (c >= '0' && c <= '9')) return parse_number(s, p);
    return Value{};
  }

  static inline Value parse_string(strv_t s, sz_t& p) {
    ++p; // skip opening "
    str_t result;
    while (p < s.size() && s[p] != '"') {
      if (s[p] == '\\') {
        ++p;
        if (p >= s.size()) break;
        switch (s[p]) {
        case '"':  result += '"'; break;
        case '\\': result += '\\'; break;
        case '/':  result += '/'; break;
        case 'b':  result += '\b'; break;
        case 'f':  result += '\f'; break;
        case 'n':  result += '\n'; break;
        case 'r':  result += '\r'; break;
        case 't':  result += '\t'; break;
        case 'u': {
          if (p + 4 < s.size()) {
            auto hex = str_t(s.substr(p + 1, 4));
            auto cp = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += cp;
            p += 4;
          }
          break;
        }
        default: result += s[p];
        }
      } else {
        result += s[p];
      }
      ++p;
    }
    if (p < s.size()) ++p; // skip closing "
    return Value(std::move(result));
  }

  static inline Value parse_number(strv_t s, sz_t& p) {
    sz_t start = p;
    if (s[p] == '-') ++p;
    while (p < s.size() && s[p] >= '0' && s[p] <= '9') ++p;
    bool is_float = false;
    if (p < s.size() && s[p] == '.') { is_float = true; ++p; while (p < s.size() && s[p] >= '0' && s[p] <= '9') ++p; }
    if (p < s.size() && (s[p] == 'e' || s[p] == 'E')) { is_float = true; ++p; if (p < s.size() && (s[p] == '+' || s[p] == '-')) ++p; while (p < s.size() && s[p] >= '0' && s[p] <= '9') ++p; }
    auto num_str = str_t(s.substr(start, p - start));
    if (is_float) return Value(std::stod(num_str));
    return Value(static_cast<i64_t>(std::stoll(num_str)));
  }

  static inline Value parse_array(strv_t s, sz_t& p) {
    ++p; // skip [
    Array arr;
    skip_ws(s, p);
    if (p < s.size() && s[p] == ']') { ++p; return Value(std::move(arr)); }
    for (;;) {
      arr.push_back(parse_value(s, p));
      skip_ws(s, p);
      if (p >= s.size() || s[p] != ',') break;
      ++p;
    }
    if (p < s.size() && s[p] == ']') ++p;
    return Value(std::move(arr));
  }

  static inline Value parse_object(strv_t s, sz_t& p) {
    ++p; // skip {
    Object obj;
    skip_ws(s, p);
    if (p < s.size() && s[p] == '}') { ++p; return Value(std::move(obj)); }
    for (;;) {
      skip_ws(s, p);
      if (p >= s.size() || s[p] != '"') break;
      auto key = parse_string(s, p);
      skip_ws(s, p);
      if (p < s.size() && s[p] == ':') ++p;
      auto val = parse_value(s, p);
      obj.emplace_back(key.as_string(), std::move(val));
      skip_ws(s, p);
      if (p >= s.size() || s[p] != ',') break;
      ++p;
    }
    if (p < s.size() && s[p] == '}') ++p;
    return Value(std::move(obj));
  }
};

} // namespace ms::json
