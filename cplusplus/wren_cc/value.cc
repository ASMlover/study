// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <sstream>
#include "vm.hh"
#include "value.hh"

namespace wrencc {

std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

StringObject* TagValue::as_string(void) const {
  return Xt::down<StringObject>(as_object());
}

const char* TagValue::as_cstring(void) const {
  return Xt::down<StringObject>(as_object())->cstr();
}

ListObject* TagValue::as_list(void) const {
  return Xt::down<ListObject>(as_object());
}

RangeObject* TagValue::as_range(void) const {
  return Xt::down<RangeObject>(as_object());
}

FunctionObject* TagValue::as_function(void) const {
  return Xt::down<FunctionObject>(as_object());
}

UpvalueObject* TagValue::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(as_object());
}

ClosureObject* TagValue::as_closure(void) const {
  return Xt::down<ClosureObject>(as_object());
}

FiberObject* TagValue::as_fiber(void) const {
  return Xt::down<FiberObject>(as_object());
}

ClassObject* TagValue::as_class(void) const {
  return Xt::down<ClassObject>(as_object());
}

InstanceObject* TagValue::as_instance(void) const {
  return Xt::down<InstanceObject>(as_object());
}

str_t TagValue::stringify(void) const {
  if (is_numeric()) {
    return Xt::to_string(as_numeric());
  }
  else if (is_object()) {
    return as_object()->stringify();
  }
  else {
    switch (tag()) {
    case Tag::NaN: return "NaN";
    case Tag::NIL: return "nil";
    case Tag::TRUE: return "true";
    case Tag::FALSE: return "false";
    }
  }
  return "";
}

StringObject* ObjValue::as_string(void) const {
  return Xt::down<StringObject>(obj_);
}

const char* ObjValue::as_cstring(void) const {
  return Xt::down<StringObject>(obj_)->cstr();
}

ListObject* ObjValue::as_list(void) const {
  return Xt::down<ListObject>(obj_);
}

RangeObject* ObjValue::as_range(void) const {
  return Xt::down<RangeObject>(obj_);
}

FunctionObject* ObjValue::as_function(void) const {
  return Xt::down<FunctionObject>(obj_);
}

UpvalueObject* ObjValue::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(obj_);
}

ClosureObject* ObjValue::as_closure(void) const {
  return Xt::down<ClosureObject>(obj_);
}

FiberObject* ObjValue::as_fiber(void) const {
  return Xt::down<FiberObject>(obj_);
}

ClassObject* ObjValue::as_class(void) const {
  return Xt::down<ClassObject>(obj_);
}

InstanceObject* ObjValue::as_instance(void) const {
  return Xt::down<InstanceObject>(obj_);
}

bool ObjValue::operator==(const ObjValue& r) const noexcept {
  if (type_ != r.type_)
    return false;

  if (type_ == ValueType::NUMERIC)
    return num_ == r.num_;
  return obj_ == r.obj_;
}

bool ObjValue::operator!=(const ObjValue& r) const noexcept {
  return !(*this == r);
}

str_t ObjValue::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(num_);
  case ValueType::OBJECT: return obj_->stringify();
  }
  return "";
}

StringObject::StringObject(char c) noexcept
  : BaseObject(ObjType::STRING)
  , size_(1) {
  value_ = new char[2];
  value_[0] = c;
  value_[1] = 0;
}

StringObject::StringObject(const char* s, int n, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING)
  , size_(n) {
  if (replace_owner) {
    value_ = const_cast<char*>(s);
  }
  else {
    value_ = new char[Xt::as_type<sz_t>(size_ + 1)];
    if (s != nullptr) {
      memcpy(value_, s, n);
      value_[size_] = 0;
    }
  }
}

StringObject::~StringObject(void) {
  delete [] value_;
}

str_t StringObject::stringify(void) const {
  return value_;
}

StringObject* StringObject::make_string(WrenVM& vm, char c) {
  auto* o = new StringObject(c);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const str_t& s) {
  return make_string(vm, s.data(), Xt::as_type<int>(s.size()));
}

StringObject* StringObject::make_string(
    WrenVM& vm, StringObject* s1, StringObject* s2) {
  int n = s1->size() + s2->size();
  char* s = new char [Xt::as_type<sz_t>(n + 1)];
  memcpy(s, s1->cstr(), s1->size());
  memcpy(s + s1->size(), s2->cstr(), s2->size());
  s[n] = 0;

  auto* o = new StringObject(s, n, true);
  vm.append_object(o);
  return o;
}

ListObject::ListObject(int num_elements) noexcept
  : BaseObject(ObjType::LIST) {
  if (num_elements > 0)
    elements_.resize(num_elements);
}

str_t ListObject::stringify(void) const {
  std::stringstream ss;
  ss << "[";
  for (auto i = 0u; i < elements_.size(); ++i) {
    if (i > 0)
      ss << ", ";

    auto& v = elements_[i];
    if (v.is_string())
      ss << "\"" << v << "\"";
    else
      ss << v;
  }
  ss << "]";
  return ss.str();
}

void ListObject::gc_mark(WrenVM& vm) {
  for (auto& e : elements_)
    vm.mark_value(e);
}

ListObject* ListObject::make_list(WrenVM& vm, int num_elements) {
  auto* o = new ListObject(num_elements);
  vm.append_object(o);
  return o;
}

str_t RangeObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

RangeObject* RangeObject::make_range(WrenVM& vm, double from, double to) {
  auto* o = new RangeObject(from, to);
  vm.append_object(o);
  return o;
}

DebugObject::DebugObject(const str_t& name,
    const str_t& source_path, int* source_lines, int lines_count) noexcept
  : name_(name)
  , source_path_(source_path)
  , source_lines_(source_lines, source_lines + lines_count) {
}

FunctionObject::FunctionObject(int num_upvalues,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& source_path, const str_t& debug_name,
    int* source_lines, int lines_count) noexcept
  : BaseObject(ObjType::FUNCTION)
  , num_upvalues_(num_upvalues)
  , codes_(codes, codes + codes_count)
  , constants_(constants, constants + constants_count)
  , debug_(debug_name, source_path, source_lines, lines_count) {
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

void FunctionObject::gc_mark(WrenVM& vm) {
  for (auto& c : constants_)
    vm.mark_value(c);
}

int FunctionObject::get_argc(
    const u8_t* bytecode, const Value* constants, int ip) {
  // returns the number of arguments to the instruction at [ip] in bytecode

  switch (Code instruction = Xt::as_type<Code>(bytecode[ip])) {
  case Code::NIL:
  case Code::FALSE:
  case Code::TRUE:
  case Code::POP:
  case Code::IS:
  case Code::CLOSE_UPVALUE:
  case Code::RETURN:
  case Code::NEW:
  case Code::CLASS:
  case Code::SUBCLASS:
  case Code::END:
    return 0;

  case Code::LOAD_LOCAL:
  case Code::STORE_LOCAL:
  case Code::LOAD_UPVALUE:
  case Code::STORE_UPVALUE:
  case Code::LOAD_GLOBAL:
  case Code::STORE_GLOBAL:
  case Code::LOAD_FIELD_THIS:
  case Code::STORE_FIELD_THIS:
  case Code::LOAD_FIELD:
  case Code::STORE_FIELD:
  case Code::LIST:
    return 1;

    // instructions with two arguments
  case Code::CONSTANT:
  case Code::CALL_0:
  case Code::CALL_1:
  case Code::CALL_2:
  case Code::CALL_3:
  case Code::CALL_4:
  case Code::CALL_5:
  case Code::CALL_6:
  case Code::CALL_7:
  case Code::CALL_8:
  case Code::CALL_9:
  case Code::CALL_10:
  case Code::CALL_11:
  case Code::CALL_12:
  case Code::CALL_13:
  case Code::CALL_14:
  case Code::CALL_15:
  case Code::CALL_16:
  case Code::SUPER_0:
  case Code::SUPER_1:
  case Code::SUPER_2:
  case Code::SUPER_3:
  case Code::SUPER_4:
  case Code::SUPER_5:
  case Code::SUPER_6:
  case Code::SUPER_7:
  case Code::SUPER_8:
  case Code::SUPER_9:
  case Code::SUPER_10:
  case Code::SUPER_11:
  case Code::SUPER_12:
  case Code::SUPER_13:
  case Code::SUPER_14:
  case Code::SUPER_15:
  case Code::SUPER_16:
  case Code::JUMP:
  case Code::LOOP:
  case Code::JUMP_IF:
  case Code::AND:
  case Code::OR:
  case Code::METHOD_INSTANCE:
  case Code::METHOD_STATIC:
    return 2;

  case Code::CLOSURE:
    {
      int constant = (bytecode[ip + 1] << 8) | (bytecode[ip + 2]);
      FunctionObject* loaded_fn = constants[constant].as_function();

      // there are two arguments for the constant, then one for each upvalue
      return 2 + loaded_fn->num_upvalues();
    }
  }
  return 0;
}

FunctionObject* FunctionObject::make_function(
    WrenVM& vm, int num_upvalues,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& source_path, const str_t& debug_name,
    int* source_lines, int lines_count) {
  auto* o = new FunctionObject(num_upvalues,
      codes, codes_count, constants, constants_count,
      source_path, debug_name, source_lines, lines_count);
  vm.append_object(o);
  return o;
}

UpvalueObject::UpvalueObject(Value* value, UpvalueObject* next) noexcept
  : BaseObject(ObjType::UPVALUE)
  , value_(value)
  , next_(next) {
}

str_t UpvalueObject::stringify(void) const {
  std::stringstream ss;
  ss << "[upvalue `" << this << "`]";
  return ss.str();
}

void UpvalueObject::gc_mark(WrenVM& vm) {
  vm.mark_value(closed_);
}

UpvalueObject* UpvalueObject::make_upvalue(
    WrenVM& vm, Value* value, UpvalueObject* next) {
  auto* o = new UpvalueObject(value, next);
  vm.append_object(o);
  return o;
}

ClosureObject::ClosureObject(FunctionObject* fn) noexcept
  : BaseObject(ObjType::CLOSURE)
  , fn_(fn) {
  int num_upvalues = fn_->num_upvalues();
  if (num_upvalues > 0) {
    upvalues_ = new UpvalueObject*[num_upvalues];
    for (int i = 0; i < num_upvalues; ++i)
      upvalues_[i] = nullptr;
  }
}

ClosureObject::~ClosureObject(void) {
  if (upvalues_ != nullptr)
    delete [] upvalues_;
}

str_t ClosureObject::stringify(void) const {
  std::stringstream ss;
  ss << "[closure `" << this << "`]";
  return ss.str();
}

void ClosureObject::gc_mark(WrenVM& vm) {
  vm.mark_object(fn_);
  for (int i = 0; i < fn_->num_upvalues(); ++i)
    vm.mark_object(upvalues_[i]);
}

ClosureObject* ClosureObject::make_closure(WrenVM& vm, FunctionObject* fn) {
  auto* o = new ClosureObject(fn);
  vm.append_object(o);
  return o;
}

FiberObject::FiberObject(BaseObject* fn) noexcept
  : BaseObject(ObjType::FIBER) {
  stack_.reserve(kDefaultCap);
  const u8_t* ip;
  if (fn->type() == ObjType::FUNCTION)
    ip = Xt::down<FunctionObject>(fn)->codes();
  else
    ip = Xt::down<ClosureObject>(fn)->fn()->codes();
  frames_.push_back(CallFrame(ip, fn, 0));
}

void FiberObject::call_function(BaseObject* fn, int argc) {
  const u8_t* ip;
  if (fn->type() == ObjType::FUNCTION)
    ip = Xt::down<FunctionObject>(fn)->codes();
  else
    ip = Xt::down<ClosureObject>(fn)->fn()->codes();
  frames_.push_back(CallFrame(ip, fn, stack_size() - argc));
}

UpvalueObject* FiberObject::capture_upvalue(WrenVM& vm, int slot) {
  // capture the local variable in [slot] into an [upvalue], if that local is
  // already in an upvalue, the existing one will be used. otherwise it will
  // create a new open upvalue and add it into the fiber's list of upvalues

  Value* local = &stack_[slot];
  // if there are no open upvalues at all, we must need a new one
  if (open_upvlaues_ == nullptr) {
    open_upvlaues_ = UpvalueObject::make_upvalue(vm, local);
    return open_upvlaues_;
  }

  UpvalueObject* prev_upvalue = nullptr;
  UpvalueObject* upvalue = open_upvlaues_;

  // walk towards the bottom of the stack until we find a previously
  // existing upvalue or pass where it should be
  while (upvalue != nullptr && upvalue->value() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }
  // found an existing upvalue for this local
  if (upvalue != nullptr && upvalue->value() == local)
    return upvalue;

  // walked past this local on the stack, so there must not be an upvalue
  // for it already. make a new one and link it in the right place to keep
  // the list sorted
  UpvalueObject* created_upvalue = UpvalueObject::make_upvalue(vm, local, upvalue);
  if (prev_upvalue == nullptr)
    open_upvlaues_ = created_upvalue;
  else
    prev_upvalue->set_next(created_upvalue);
  return created_upvalue;
}

void FiberObject::close_upvalue(void) {
  UpvalueObject* upvalue = open_upvlaues_;

  // move the value into the upvalue itself and point the value to it
  upvalue->set_closed(upvalue->value_asref());
  upvalue->set_value(upvalue->closed_asptr());

  // remove it from the open upvalue list
  open_upvlaues_ = upvalue->next();
}

void FiberObject::close_upvalues(int slot) {
  Value* first = &stack_[slot];
  while (open_upvlaues_ != nullptr && open_upvlaues_->value() >= first)
    close_upvalue();
}

void FiberObject::riter_frames(
    std::function<void (const CallFrame&, FunctionObject*)>&& visit) {
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto& frame = *it;
    FunctionObject* fn;
    if (frame.fn->type() == ObjType::FUNCTION)
      fn = Xt::down<FunctionObject>(frame.fn);
    else
      fn = Xt::down<ClosureObject>(frame.fn)->fn();

    visit(frame, fn);
  }
}

str_t FiberObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fiber " << this << "]";
  return ss.str();
}

void FiberObject::gc_mark(WrenVM& vm) {
  // stack functions
  for (auto& f : frames_)
    vm.mark_object(f.fn);

  // stack variables
  for (auto& v : stack_)
    vm.mark_value(v);

  // open upvalues
  for (auto* upvalue = open_upvlaues_;
      upvalue != nullptr; upvalue = upvalue->next())
    vm.mark_object(upvalue);

  vm.mark_object(caller_);
}

FiberObject* FiberObject::make_fiber(WrenVM& vm, BaseObject* fn) {
  auto* o = new FiberObject(fn);
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(void) noexcept
  : BaseObject(ObjType::CLASS) {
}

ClassObject::ClassObject(
    ClassObject* meta_class, ClassObject* supercls, int num_fields) noexcept
  : BaseObject(ObjType::CLASS)
  , meta_class_(meta_class)
  , num_fields_(num_fields) {
  if (supercls != nullptr)
    bind_superclass(supercls);
}

void ClassObject::bind_superclass(ClassObject* superclass) {
  ASSERT(superclass != nullptr, "must have superclass");

  superclass_ = superclass;
  // include the superclass in the total number of fields
  num_fields_ += superclass->num_fields_;
  // inherit methods from its superclass
  int super_methods_count = superclass_->methods_count();
  for (int i = 0; i < super_methods_count; ++i)
    bind_method(i, superclass_->methods_[i]);
}

void ClassObject::bind_method(FunctionObject* fn) {
  int ip = 0;
  for (;;) {
    switch (Code c = Xt::as_type<Code>(fn->get_code(ip++))) {
    case Code::LOAD_FIELD:
    case Code::STORE_FIELD:
    case Code::LOAD_FIELD_THIS:
    case Code::STORE_FIELD_THIS:
      {
        auto num_fields = fn->get_code(ip) + superclass_->num_fields();
        fn->set_code(ip++, num_fields);
      } break;
    case Code::CLOSURE:
      {
        int constant = (fn->get_code(ip) << 8) | fn->get_code(ip + 1);
        bind_method(fn->get_constant(constant).as_function());
        ip += FunctionObject::get_argc(fn->codes(), fn->constants(), ip - 1);
      } break;

    case Code::END: return;
    default:
      // other instructions are unaffected, so just skip over them
      ip += FunctionObject::get_argc(fn->codes(), fn->constants(), ip - 1);
      break;
    }
  }
}

void ClassObject::bind_method(int i, int method_type, const Value& fn) {
  FunctionObject* method_fn = fn.is_function()
    ? fn.as_function() : fn.as_closure()->fn();

  // methods are always bound against the class, and not the metaclass, even
  // for static methods, so that constructors (which are static) get bound
  // like instance methods
  bind_method(method_fn);

  if (Xt::as_type<Code>(method_type) == Code::METHOD_STATIC )
    meta_class_->bind_method(i, fn.as_object());
  else
    bind_method(i, fn.as_object());
}

void ClassObject::bind_method(int i, const Method& method) {
  // make sure the buffer is big enough to reach the symbol's index

  while (i >= methods_count())
    methods_.push_back(Method());
  methods_[i] = method;
}

str_t ClassObject::stringify(void) const {
  std::stringstream ss;
  ss << "[class `" << this << "`]";
  return ss.str();
}

void ClassObject::gc_mark(WrenVM& vm) {
  vm.mark_object(meta_class_);
  vm.mark_object(superclass_);
  for (auto& m : methods_) {
    if (m.type == MethodType::BLOCK)
      vm.mark_object(m.fn());
  }
}

ClassObject* ClassObject::make_single_class(WrenVM& vm) {
  auto* o = new ClassObject(nullptr, nullptr);
  vm.append_object(o);
  return o;
}

ClassObject* ClassObject::make_class(
    WrenVM& vm, ClassObject* superclass, int num_fields) {
  // metaclasses always inherit Class and do not parallel the non-metaclass
  // hierarclly
  ClassObject* meta_class = new ClassObject(vm.class_cls(), vm.class_cls(), 0);
  vm.append_object(meta_class);

  auto* o = new ClassObject(meta_class, superclass, num_fields);
  vm.append_object(o);
  return o;
}

InstanceObject::InstanceObject(ClassObject* cls) noexcept
  : BaseObject(ObjType::INSTANCE)
  , cls_(cls)
  , fields_(cls->num_fields()) {
  for (int i = 0; i < cls_->num_fields(); ++i)
    fields_[i] = nullptr;
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "[instance `" << this << "`]";
  return ss.str();
}

void InstanceObject::gc_mark(WrenVM& vm) {
  for (auto& v : fields_)
    vm.mark_value(v);
}

InstanceObject* InstanceObject::make_instance(WrenVM& vm, ClassObject* cls) {
  auto* o = new InstanceObject(cls);
  vm.append_object(o);
  return o;
}

}
