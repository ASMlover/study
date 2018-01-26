#pragma once

#include <string>
#include "pl_python_utils.h"

namespace public_logic {

struct UnitBase : public boost::enable_shared_from_this<UnitBase> {
	int unit_id{};
	int hero_id{};
	int root_unit_id{};
	int unit_type{};
	int camp_type{};
	std::string unit_name;
	// TODO: need more class properties

	UnitBase(void) {
	}

	virtual ~UnitBase(void) {
	}

	virtual void tick(double dt) {
	}
};

class UnitBaseWrap : public UnitBase {
	PyObject* self_{};
public:
	UnitBaseWrap(PyObject* self)
		: self_(self) {
		py::xincref(self_);
	}

	virtual ~UnitBaseWrap(void) {
		py::xdecref(self_);
	}

	virtual void tick(double dt) {
		_PL_WITHOUT_GIL_TRY {
			if (self_ != nullptr)
				py::call_method<void>(self_, "tick", dt);
		} _PL_END_TRY
	}

	void default_tick(double dt) {
		UnitBase::tick(dt);
	}
};

}
