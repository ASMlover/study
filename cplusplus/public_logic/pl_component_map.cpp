#include "pl_component_map.h"

namespace public_logic {

ComponentPtr& ComponentMap::getitem(int key) {
  if (find(key) == end()) {
    PyErr_SetString(PyExc_KeyError, "key not in C++ map");
    py::throw_error_already_set();
  }
  return operator[](key);
}

void ComponentMap::delitem(int key) {
  auto pos = find(key);
  if (pos == end()) {
    PyErr_SetString(PyExc_KeyError, "key not in C++ map");
    py::throw_error_already_set();
  }
  erase(pos);
}

py::object ComponentMap::get(int key, const py::object& d) const {
  auto pos = find(key);
  if (pos == end())
    return d;

  py::return_by_value::apply<ComponentPtr>::type convertor;
  return py::object(py::handle<>(convertor(pos->second)));
}

py::object ComponentMap::pop(int key, const py::object& d) {
  auto pos = find(key);
  if (pos == end())
    return d;

  py::return_by_value::apply<ComponentPtr>::type convertor;
  auto r = py::object(py::handle<>(convertor(pos->second)));

  erase(pos);
  return r;
}

py::list ComponentMap::keys(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(v.first);
  return r;
}

py::list ComponentMap::values(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(v.second);
  return r;
}

py::list ComponentMap::items(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(py::make_tuple(v.first, v.second));
  return r;
}

void ComponentMap::update(const ComponentMap& other) {
  for (auto& v : other)
    operator[](v.first) = v.second;
}

py::tuple ComponentMap::popitem(void) {
  auto pos = begin();
  if (pos == end()) {
    PyErr_SetString(PyExc_KeyError, "popitem(): C++ map is empty");
    py::throw_error_already_set();
  }

  auto r = py::make_tuple(pos->first, pos->second);
  erase(pos);

  return r;
}

}
