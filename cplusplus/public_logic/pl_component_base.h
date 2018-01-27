#pragma once

#include "pl_python_utils.h"

namespace public_logic {

struct ComponentBase : public boost::enable_shared_from_this<ComponentBase> {
  py::object weakref_owner{};
  int comp_type{};
  bool need_tick{};
  bool is_render_tick{};

  ComponentBase(void) {
  }

  virtual ~ComponentBase(void) {
  }

  py::object get_owner(void) {
    auto* ref = Py_None;
    if (!weakref_owner.is_none())
      ref = PyWeakref_GetObject(weakref_owner.ptr());

    return py::object(py::handle<>(py::borrowed(ref)));
  }

  void destroy(void) {
    if (!weakref_owner.is_none())
      weakref_owner = py::object();
    comp_type = 0;
  }

  void register_owner(const py::object& owner) {
    auto* weakref = PyWeakref_NewRef(owner.ptr(), nullptr);
    if (weakref == nullptr)
      PyErr_Print();

    weakref_owner = py::object(py::handle<>(weakref));
  }
};

class ComponentBaseWrap : public ComponentBase {
  PyObject* self_{};
  public:
  ComponentBaseWrap(PyObject* self)
    : self_(self) {
      py::xincref(self_);
    }

  virtual ~ComponentBaseWrap(void) {
    py::xdecref(self_);
  }
};

}
