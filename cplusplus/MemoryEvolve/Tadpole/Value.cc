#include <cstring>
#include <Tadpole/Chunk.hh>
#include <Tadpole/Value.hh>

namespace _mevo::tadpole {

StringObject* BaseObject::as_string() {
  return down<StringObject>(this);
}

const char* BaseObject::as_cstring() {
  return down<StringObject>(this)->cstr();
}

NativeObject* BaseObject::as_native() {
  return down<NativeObject>(this);
}

FunctionObject* BaseObject::as_function() {
  return down<FunctionObject>(this);
}

UpvalueObject* BaseObject::as_upvalue() {
  return nullptr;
}

ClosureObject* BaseObject::as_closure() {
  return nullptr;
}

bool Value::is_truthy() const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric != 0;
  case ValueType::OBJECT: return as_.object->is_truthy();
  }
  return false;
}

str_t Value::stringify() const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return convert_to_string(as_.numeric);
  case ValueType::OBJECT: return as_.object->stringify();
  }
  return "";
}

inline u32_t get_hash(const char* s, sz_t n) noexcept {
  u32_t hash = 2166136261u;
  for (sz_t i = 0; i < n; ++i) {
    hash ^= s[i];
    hash *= 16777619;
  }
  return hash;
}

template <typename Object, typename... Args>
inline Object* make_object(VM& vm, Args&&... args) noexcept {
  auto* o = new Object(std::forward<Args>(args)...);
  // vm.append_object(o); // TODO:
  return o;
}

StringObject::StringObject(
  const char* s, sz_t n, u32_t h, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING)
  , size_(n)
  , hash_(h) {
  if (replace_owner) {
    data_ = const_cast<char*>(s);
  }
  else {
    data_ = new char[size_ + 1];
    std::memcpy(data_, s, n);
    data_[size_] = 0;
  }
}

StringObject::~StringObject() {
  delete [] data_;
}

str_t StringObject::stringify() const {
  return data_;
}

StringObject* StringObject::create(VM& vm, const char* s, sz_t n) {
  u32_t h = get_hash(s, n);
  // if (auto* o = vm.get_interned(h); o != nullptr)
  //   return o;

  auto* o = make_object<StringObject>(vm, s, n, h);
  // vm.set_interned(h, o);
  return o;
}

StringObject* StringObject::concat(VM& vm, StringObject* s1, StringObject* s2) {
  sz_t n = s1->size() + s2->size();
  char* s = new char[n + 1];
  std::memcpy(s, s1->data(), s1->size());
  std::memcpy(s + s1->size(), s2->data(), s2->size());
  s[n] = 0;

  u32_t h = get_hash(s, n);
  // if (auto* o = vm.get_interned(h); o != nullptr) {
  //   delete [] s;
  //   return o;
  // }

  auto* o = make_object<StringObject>(vm, s, n, h, true);
  // vm.set_interned(h, o);
  return o;
}

str_t NativeObject::stringify() const {
  ss_t ss;
  ss << "<native function at `" << this << "`>";
  return ss.str();
}

NativeObject* NativeObject::create(VM& vm, NativeFn&& fn) {
  return make_object<NativeObject>(vm, std::move(fn));
}

FunctionObject::FunctionObject(StringObject* name) noexcept
  : BaseObject(ObjType::FUNCTION), name_(name) {
  chunk_ = new Chunk();
}

FunctionObject::~FunctionObject() {
  delete chunk_;
}

str_t FunctionObject::stringify() const {
  ss_t ss;
  ss << "<function `" << name_asstr() << "` at `" << this << "`>";
  return ss.str();
}

void FunctionObject::gc_blacken(VM& vm) {
  // TODO:
}

FunctionObject* FunctionObject::create(VM& vm, StringObject* name) {
  return make_object<FunctionObject>(vm, name);
}

}
