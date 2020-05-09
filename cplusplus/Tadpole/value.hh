#pragma once

#include "common.hh"

namespace tadpole {

enum class ObjType : u8_t {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
};

class VM;
class Chunk;

class StringObject;
class NativeObject;
class FunctionObject;
class UpvalueObject;
class ClosureObject;

class BaseObject : private UnCopyable {
  ObjType type_;
  bool marked_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_marked() const noexcept { return marked_; }
  inline void set_marked(bool marked = true) noexcept { marked_ = marked; }

  virtual bool is_truthy() const { return true; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void gc_blacken(VM& vm) {}

  StringObject* as_string();
  const char* as_cstring();
  NativeObject* as_native();
  FunctionObject* as_function();
  UpvalueObject* as_upvalue();
  ClosureObject* as_closure();
};

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

  template <typename T> inline void set_numeric(T x) noexcept {
    as_.numeric = as_type<double>(x);
  }

  inline bool is(ObjType type) const noexcept {
    return is_object() && as_.object->type() == type;
  }
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
  inline bool is_native() const noexcept { return is(ObjType::STRING); }
  inline bool is_function() const noexcept { return is(ObjType::FUNCTION); }
  inline bool is_upvalue() const noexcept { return is(ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(ObjType::CLOSURE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline bool as_numeric() const noexcept { return as_.numeric; }
  inline BaseObject* as_object() const noexcept { return as_.object; }
  inline StringObject* as_string() const noexcept { return as_.object->as_string(); }
  inline const char* as_cstring() const noexcept { return as_.object->as_cstring(); }
  inline NativeObject* as_native() const noexcept { return as_.object->as_native(); }
  inline FunctionObject* as_function() const noexcept { return as_.object->as_function(); }
  inline UpvalueObject* as_upvalue() const noexcept { return as_.object->as_upvalue(); }
  inline ClosureObject* as_closure() const noexcept { return as_.object->as_closure(); }

  bool is_truthy() const;
  str_t stringify() const;
};

inline std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

class StringObject final : public BaseObject {
  sz_t size_{};
  char* data_{};
  u32_t hash_{};
public:
  StringObject(const char* s, sz_t n, u32_t h, bool is_move = false) noexcept;
  virtual ~StringObject();

  inline const char* data() const noexcept { return data_; }
  inline const char* cstr() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }

  virtual bool is_truthy() const override;
  virtual str_t stringify() const override;

  static StringObject* create(VM& vm, const char* s, sz_t n);
  static StringObject* concat(VM& vm, StringObject* s1, StringObject* s2);

  template <typename N>
  static StringObject* create(VM& vm, const char* s, N n) {
    return create(vm, s, as_type<sz_t>(n));
  }

  static StringObject* create(VM& vm, const str_t& s) {
    return create(vm, s.data(), s.size());
  }

  static StringObject* create(VM& vm, strv_t s) {
    return create(vm, s.data(), s.size());
  }
};

}