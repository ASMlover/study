#pragma once

#include <functional>
#include <vector>
#include <Tadpole/Value.hh>

namespace _mevo::tadpole {

enum class Code {
  CONSTANT,   // load the constant at index [arg]

  NIL,        // push `nil` onto the stack
  FALSE,      // push `false` onto the stack
  TRUE,       // push `true` onto the stack
  POP,        // pop and discard the top of stack

  DEF_GLOBAL, // define the top of stack in global slot [arg]
  GET_GLOBAL, // pushes the value in global slot [arg]
  SET_GLOBAL, // stores the top of stack in global slot [arg], do not pop it
  GET_LOCAL,  // pushes the value in local slot [arg]
  SET_LOCAL,  // stores the top of stack in local slot [arg], do not pop it
  GET_UPVALUE,// pushes the value in upvalue slot [arg]
  SET_UPVALUE,// stores the top of stack in upvalue slot [arg], do not pop it

  // calculate peek [0] and peek [1], pushes the result value onto the stack
  ADD,
  SUB,
  MUL,
  DIV,

  // call function, the number indicates the number of arguments (not
  // including the function object)
  CALL_0,
  CALL_1,
  CALL_2,
  CALL_3,
  CALL_4,
  CALL_5,
  CALL_6,
  CALL_7,
  CALL_8,

  // creares a closure for the function stored at [arg] in the constant table,
  // pushes the created closure onto the stack
  CLOSURE,

  // close the upvalue for the local on the top of the stack, then pop it
  CLOSE_UPVALUE,

  // exit from the current function and return the value on the top of the stack
  RETURN,
};

class Chunk final : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<int> lines_;
  std::vector<Value> constants_;
public:
  template <typename T> inline u8_t write(T code, int line) noexcept {
    codes_.push_back(as_type<u8_t>(code));
    lines_.push_back(line);
    return as_type<u8_t>(codes_.size() - 1);
  }

  inline u8_t add_constant(Value value) noexcept {
    constants_.push_back(value);
    return as_type<u8_t>(constants_.size());
  }

  inline int count() const noexcept { return as_type<int>(codes_.size()); }
  inline const u8_t* codes() const noexcept { return codes_.data(); }
  inline u8_t get_code(sz_t i) const noexcept { return codes_[i]; }
  template <typename T>
  inline void set_code(sz_t i, T c) noexcept { codes_[i] = as_type<u8_t>(c); }
  inline int get_line(sz_t i) const noexcept { return lines_[i]; }
  inline const Value& get_constant(sz_t i) const noexcept { return constants_[i]; }
  inline int codes_offset(const u8_t* ip) const { return as_type<int>(ip - codes()); }

  template <typename Fn> inline void iter_constants(Fn&& fn) {
    for (auto& c : constants_)
      fn(c);
  }

  void dis(const str_t& s);
  int dis_code(int i);
};

}
