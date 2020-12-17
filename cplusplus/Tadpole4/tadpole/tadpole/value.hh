#pragma once

#include "common.hh"
#include "object.hh"

namespace tadpole {

enum class ValueType : u8_t {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* object{};
  } as_;

  template <typename T> inline void set_numeric(T x) noexcept { as_.numeric = as_type<double>(x); }
  inline bool is(ObjType type) const noexcept { return is_object() && objtype() == type; }
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
  Value(i8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(float n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(double n) noexcept : type_(ValueType::NUMERIC) { as_.numeric = n; }
  Value(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }

  inline ObjType objtype() const noexcept { return as_.object->type(); }

  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return is(ObjType::STRING); }
  inline bool is_native() const noexcept { return is(ObjType::NATIVE); }
  inline bool is_function() const noexcept { return is(ObjType::FUNCTION); }
  inline bool is_upvalue() const noexcept { return is(ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(ObjType::CLOSURE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline double as_numeric() const noexcept { return as_.numeric; }
  inline BaseObject* as_object() const noexcept { return as_.object; }
  inline StringObject* as_string() const noexcept { return as_.object->as_string(); }
  inline const char* as_cstring() const noexcept { return as_.object->as_cstring(); }
  inline NativeObject* as_native() const noexcept { return as_.object->as_native(); }
  inline FunctionObject* as_function() const noexcept { return as_.object->as_function(); }
  inline UpvalueObject* as_upvalue() const noexcept { return as_.object->as_upvalue(); }
  inline ClosureObject* as_closure() const noexcept { return as_.object->as_closure(); }

  bool is_truthy() const {
    switch (type_) {
    case ValueType::NIL: return false;
    case ValueType::BOOLEAN: return as_.boolean;
    case ValueType::NUMERIC: return as_.numeric != 0;
    case ValueType::OBJECT: return as_.object->is_truthy();
    }
    return false;
  }

  str_t stringify() const {
    switch (type_) {
    case ValueType::NIL: return "nil";
    case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
    case ValueType::NUMERIC: return tadpole::as_string(as_.numeric);
    case ValueType::OBJECT: return as_.object->stringify();
    }
    return "<value>";
  }
};

inline std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

}