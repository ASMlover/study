#pragma once

#include "pl_python_utils.h"

namespace public_logic {

struct ComponentBase;
using ComponentPtr = boost::shared_ptr<ComponentBase>;

struct ComponentMap : public std::map<int, ComponentPtr> {
  inline py::object iter(void) const {
    py::handle<> it(PyObject_GetIter(keys().ptr()));
    return py::object(it);
  }

  inline py::object iterkeys(void) const {
    py::handle<> it(PyObject_GetIter(keys().ptr()));
    return py::object(it);
  }

  inline py::object itervalues(void) const {
    py::handle<> it(PyObject_GetIter(values().ptr()));
    return py::object(it);
  }

  inline py::object iteritems(void) const {
    py::handle<> it(PyObject_GetIter(items().ptr()));
    return py::object(it);
  }

  inline py::tuple getinitargs(void) const {
    return py::make_tuple(py::dict(items()));
  }

  inline py::object as_dict(void) const {
    return map_as_dict(*this);
  }

  inline bool contains(int key) const {
    return find(key) != end();
  }

  inline void setitem(int key, const ComponentPtr& val) {
    operator[](key) = val;
  }

  ComponentPtr& getitem(int key);
  void delitem(int key);
  py::object get(int key, const py::object& d = py::object()) const;
  py::object pop(int key, const py::object& d = py::object());
  py::list keys(void) const;
  py::list values(void) const;
  py::list items(void) const;
  void update(const ComponentMap& other);
  py::tuple popitem(void);
};

}
