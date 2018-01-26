#pragma once

#include "pl_python_utils.h"

namespace public_logic {

struct ComponentBase;
using ComponentPtr = boost::shared_ptr<ComponentBase>;

struct ComponentMap : public std::map<int, ComponentPtr> {
	inline py::object iter(void) {
		py::handle<> it(PyObject_GetIter(keys().ptr()));
		return py::object(it);
	}

	inline py::tuple getinitargs(void) {
		return py::make_tuple(py::dict(items()));
	}

	inline bool contains(int key) const {
		return find(key) != end();
	}

	inline void setitem(int key, const ComponentPtr& val) {
		operator[](key) = val;
	}

	ComponentPtr& getitem(int key);
	void delitem(int key);
	py::list keys(void);
	py::list values(void);
	py::list items(void);
	void update(const ComponentMap& other);
	py::tuple popitem(void);
	static py::object get(const py::object& m, const py::object& k, const py::object& d = py::object());
};

}
