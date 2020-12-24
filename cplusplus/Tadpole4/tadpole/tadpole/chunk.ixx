module;
#include <iostream>
#include <vector>
#include "common.hh"
#include "value.hh"

export module chunk;

export namespace tadpole {

enum class Code : u8_t {
    CONSTANT,

    NIL,
    FALSE,
    TRUE,
    POP,

    DEF_GLOBAL,
    GET_GLOBAL,
    SET_GLOBAL,
    GET_LOCAL,
    SET_LOCAL,
    GET_UPVALUE,
    SET_UPVALUE,

    ADD,
    SUB,
    MUL,
    DIV,

    CALL_0,
    CALL_1,
    CALL_2,
    CALL_3,
    CALL_4,
    CALL_5,
    CALL_6,
    CALL_7,
    CALL_8,

    CLOSURE,
    CLOSE_UPVALUE,

    RETURN,
};

inline int operator-(Code a, Code b) noexcept {
  return as_type<int>(a) - as_type<int>(b);
}

template <typename T> inline Code operator+(Code a, T b) noexcept {
  return as_type<Code>(as_type<int>(a) + as_type<int>(b));
}

class Chunk final : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<int> lines_;
  std::vector<Value> constants_;

  inline sz_t dis_compound(
    const char* msg, sz_t i, bool with_constant = false) noexcept {
    auto c = get_code(i + 1);
    std::fprintf(stdout, "%-16s %4d", msg, c);
    if (with_constant)
      std::cout << " `" << get_constant(c) << "`";
    std::cout << std::endl;

    return i + 2;
  }

  inline sz_t dis_simple(const char* msg, sz_t i, int n = 0) noexcept {
    std::cout << msg;
    if (n > 0)
      std::cout << "_" << n;
    std::cout << std::endl;

    return i + 1;
  }
public:
  template <typename T> inline u8_t write(T c, int lineno) noexcept {
    codes_.push_back(as_type<u8_t>(c));
    lines_.push_back(lineno);
    return as_type<u8_t>(codes_.size() - 1);
  }

  inline u8_t add_constant(Value value) noexcept {
    constants_.push_back(value);
    return as_type<u8_t>(constants_.size() - 1);
  }

  inline void write_constant(Value value, int lineno) noexcept {
    write(Code::CONSTANT, lineno);
    write(add_constant(value), lineno);
  }

  inline sz_t codes_count() const noexcept { return codes_.size(); }
  inline const u8_t* codes() const noexcept { return codes_.data(); }
  inline u8_t get_code(sz_t i) const noexcept { return codes_[i]; }
  template <typename T>
  inline void set_code(sz_t i, T c) noexcept { codes_[i] = as_type<u8_t>(c); }
  inline int get_line(sz_t i) const noexcept { return lines_[i]; }
  inline const Value& get_constant(sz_t i) const noexcept { return constants_[i]; }
  inline sz_t offset(const u8_t* ip) const noexcept { return as_type<sz_t>(ip - codes()); }

  template <typename Fn> inline void iter_constants(Fn&& fn) {
    for (auto& c : constants_)
      fn(c);
  }

  void dis(strv_t prompt);
  sz_t dis_code(sz_t offset);
};

}