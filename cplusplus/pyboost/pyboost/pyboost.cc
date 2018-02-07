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
#include <iostream>
#include <boost/python.hpp>
#include "component.h"
#include "unit.h"
#include "objmgr.h"

const char* greet(void) {
  return "Hello, PyBoost";
}

struct FakeBase {
  std::string basename;

  FakeBase(void) {
  }

  FakeBase(const char* name)
    : basename(name) {
  }

  virtual ~FakeBase(void) {
  }

  std::string get_name(void) const {
    return basename;
  }
};

class World {
  std::string message_;

  World(const World&) = delete;
  World& operator=(const World&) = delete;
public:
  World(void) = default;
  ~World(void) = default;

  World(const char* msg)
    : message_(msg) {
  }

  void set_msg(const std::string& message) {
    message_ = message;
  }

  std::string get_msg(void) const {
    return message_;
  }
};

class WorldWrapper : public World {
  PyObject* self_{};
public:
  WorldWrapper(PyObject* self)
    : self_(self) {
    boost::python::xincref(self_);
  }

  WorldWrapper(PyObject* self, const char* msg)
    : World(msg)
    , self_(self) {
    boost::python::xincref(self_);
  }

  ~WorldWrapper(void) {
    boost::python::xdecref(self_);
  }
};

struct Var {
  std::string name;
  float value{};

  Var(const char* n = "")
    : name(n) {
  }
};

class VarWrapper : public Var {
  PyObject* self_{};
public:
  VarWrapper(PyObject* self)
    : self_(self) {
    boost::python::xincref(self_);
  }

  VarWrapper(PyObject* self, const char* name)
    : Var(name)
    , self_(self) {
    boost::python::xincref(self_);
  }

  ~VarWrapper(void) {
    boost::python::xdecref(self_);
  }
};

class Num {
  float value_{};

  Num(const Num&) = delete;
  Num& operator=(const Num&) = delete;
public:
  Num(void) = default;
  ~Num(void) = default;

  void set(float value) {
    value_ = value;
  }

  float get(void) const {
    return value_;
  }
};

class NumWrapper : public Num {
  PyObject* self_{};
public:
  NumWrapper(PyObject* self)
    : self_(self) {
    boost::python::xincref(self_);
  }

  ~NumWrapper(void) {
    boost::python::xdecref(self_);
  }
};

struct Base {
  virtual ~Base(void) {}
  virtual std::string get_name(void) const {
    return "Base";
  }
};

struct Derived : public Base {
  virtual std::string get_name(void) const override {
    return "Derived";
  }
};

static void print_base(Base* b) {
  std::cout << "Base name: " << b->get_name() << std::endl;
}

static void print_derived(Derived* d) {
  std::cout << "Derived name: " << d->get_name() << std::endl;
}

Base* base_factory(void) {
  return new Derived();
}

class VirtualBase {
public:
  virtual ~VirtualBase(void) {}
  virtual int get_value(void) const {
    return 0;
  }
};

class VirtualBaseWrap
  : public VirtualBase, public boost::python::wrapper<VirtualBase> {
public:
  virtual int get_value(void) const override {
    if (boost::python::override f = get_override("get_value"))
      return f();
    return VirtualBase::get_value();
  }

  int default_get_value(void) const {
    return VirtualBase::get_value();
  }
};

class Int : private boost::noncopyable {
  int value_{};
public:
  Int(int v = 0)
    : value_(v) {
  }

  void set(int v) {
    value_ = v;
  }

  int get(void) const {
    return value_;
  }

  operator int(void) const {
    return value_;
  }
};

class IntWrapper : public Int {
  PyObject* self_{};
public:
  IntWrapper(PyObject* self)
    : self_(self) {
    boost::python::xincref(self_);
  }

  IntWrapper(PyObject* self, int v)
    : Int(v)
    , self_(self) {
    boost::python::xincref(self_);
  }

  ~IntWrapper(void) {
    boost::python::xdecref(self_);
  }
};

void pyboost_init(void) {
  PyEval_InitThreads();
  OBJMGR();
}

void pyboost_objmgr_add_object(const boost::shared_ptr<ObjNode>& o) {
  OBJMGR().add_object(o);
}

BOOST_PYTHON_MODULE(pyboost) {
  pyboost_init();

  boost::python::def("greet", greet);

  boost::python::class_<FakeBase>("FakeBase")
    .def(boost::python::init<const char*>())
    .def_readwrite("basename", &FakeBase::basename)
    .def("get_name", &FakeBase::get_name);

  boost::python::class_<World,
    std::shared_ptr<WorldWrapper>, boost::noncopyable>("World")
    .def(boost::python::init<const char*>())
    .def("set_msg", &World::set_msg)
    .def("get_msg", &World::get_msg);

  boost::python::class_<Var,
    std::shared_ptr<VarWrapper>, boost::noncopyable>("Var")
    .def(boost::python::init<const char*>())
    .def_readonly("name", &Var::name)
    .def_readwrite("value", &Var::value);

  boost::python::class_<Num,
    std::shared_ptr<NumWrapper>, boost::noncopyable>("Num")
    .add_property("rvalue", &Num::get)
    .add_property("value", &Num::get, &Num::set);

  boost::python::class_<Base>("Base")
    .def("get_name", &Base::get_name);
  boost::python::class_<Derived, boost::python::bases<Base>>("Derived")
    .def("get_name", &Derived::get_name);
  boost::python::def("print_base", print_base);
  boost::python::def("print_derived", print_derived);
  boost::python::def("base_factory", base_factory,
      boost::python::return_value_policy<boost::python::manage_new_object>());

  boost::python::class_<VirtualBaseWrap, boost::noncopyable>("VirtualBase")
    .def("get_value", &VirtualBase::get_value);

  boost::python::class_<Int,
    std::shared_ptr<IntWrapper>, boost::noncopyable>("Int")
    .def(boost::python::init<int>())
    .def("set", &Int::set)
    .def("get", &Int::get);

  boost::python::class_<ComponentBase>("ComponentBase")
    .def_readonly("owner", &ComponentBase::get_owner)
    .def_readwrite("need_tick", &ComponentBase::need_tick)
    .def_readwrite("is_render_tick", &ComponentBase::is_render_tick)
    .def_readwrite("comp_type", &ComponentBase::comp_type)
    .def("destroy", &ComponentBase::destroy)
    .def("on_add_to_unit", &ComponentBase::on_add_to_unit);

  boost::python::class_<UnitBaseWrap, boost::noncopyable>("UnitBase")
    .def_readwrite("unit_id", &UnitBase::unit_id)
    .def_readwrite("hero_id", &UnitBase::hero_id)
    .def_readwrite("root_unit_id", &UnitBase::root_unit_id)
    .def_readwrite("unit_type", &UnitBase::unit_type)
    .def_readwrite("camp_type", &UnitBase::camp_type)
    .def_readwrite("unit_name", &UnitBase::unit_name)
    .def("tick", &UnitBase::tick, &UnitBaseWrap::default_tick);

  py::class_<ObjNode,
    boost::shared_ptr<ObjNodeWrap>, boost::noncopyable>("ObjNode")
    .def(py::init<int>())
    .def_readonly("obj_id", &ObjNode::obj_id)
    .def("show", &ObjNodeWrap::show_default);

  py::def("objmgr_add_object", pyboost_objmgr_add_object);
}
