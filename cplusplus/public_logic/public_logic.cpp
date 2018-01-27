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
#include "pl_python_utils.h"
#include "pl_component_base.h"
#include "pl_component_map.h"
#include "pl_unit_base.h"
#include "pl_unit_manager.h"

namespace public_logic {

void pl_register_unit(const UnitBasePtr& u) {
  UnitManager::instance().register_unit(u);
}

void pl_tick_all_units(double dt) {
  UnitManager::instance().tick_all(dt);
}

void pl_clear_all_unitts(void) {
  UnitManager::instance().clear_all();
}

void pl_initialize(void) {
  PyEval_InitThreads();
  UnitManager::instance();
}

}

BOOST_PYTHON_MODULE(public_logic) {
  public_logic::pl_initialize();

  py::class_<public_logic::ComponentBase,
    boost::shared_ptr<public_logic::ComponentBaseWrap>, boost::noncopyable>("ComponentBase", py::init<>())
    .def_readonly("owner", &public_logic::ComponentBase::get_owner)
    .def_readwrite("comp_type", &public_logic::ComponentBase::comp_type)
    .def_readwrite("need_tick", &public_logic::ComponentBase::need_tick)
    .def_readwrite("is_render_tick", &public_logic::ComponentBase::is_render_tick)
    .def("destroy", &public_logic::ComponentBase::destroy)
    .def("on_add_to_unit", &public_logic::ComponentBase::register_owner)
    ;

  py::class_<public_logic::UnitBase,
    boost::shared_ptr<public_logic::UnitBaseWrap>, boost::noncopyable>("UnitBase", py::init<>())
    .def_readwrite("unit_id", &public_logic::UnitBase::unit_id)
    .def_readwrite("hero_id", &public_logic::UnitBase::hero_id)
    .def_readwrite("root_unit_id", &public_logic::UnitBase::root_unit_id)
    .def_readwrite("unit_type", &public_logic::UnitBase::unit_type)
    .def_readwrite("camp_type", &public_logic::UnitBase::camp_type)
    .def_readwrite("unit_name", &public_logic::UnitBase::unit_name)
    .def("tick", &public_logic::UnitBaseWrap::default_tick)
    ;

  py::def("register_unit", public_logic::pl_register_unit);
  py::def("tick_all_units", public_logic::pl_tick_all_units);
  py::def("clear_all_units", public_logic::pl_clear_all_unitts);

  py::class_<public_logic::ComponentMap, boost::shared_ptr<public_logic::ComponentMap>>("ComponentMap")
    .def(py::init<const public_logic::ComponentMap&>())
    .def("size", &public_logic::ComponentMap::size)
    .def("__len__", &public_logic::ComponentMap::size)
    .def("clear", &public_logic::ComponentMap::clear)
    .def("__contains__", &public_logic::ComponentMap::contains)
    .def("has_key", &public_logic::ComponentMap::contains)
    .def("get", &public_logic::ComponentMap::get, (py::arg("d") = py::object()))
    .def("pop", &public_logic::ComponentMap::pop, (py::arg("d") = py::object()))
    .def("erase", (std::size_t (public_logic::ComponentMap::*)(const int&))&public_logic::ComponentMap::erase)
    .def("keys", &public_logic::ComponentMap::keys)
    .def("values", &public_logic::ComponentMap::values)
    .def("items", &public_logic::ComponentMap::items)
    .def("iterkeys", &public_logic::ComponentMap::iterkeys)
    .def("itervalues", &public_logic::ComponentMap::itervalues)
    .def("iteritems", &public_logic::ComponentMap::iteritems)
    .def("update", &public_logic::ComponentMap::update)
    .def("popitem", &public_logic::ComponentMap::popitem)
    .def("as_dict", &public_logic::ComponentMap::as_dict)
    .def("__setitem__", &public_logic::ComponentMap::setitem)
    .def("__getitem__", &public_logic::ComponentMap::getitem, py::return_value_policy<py::copy_non_const_reference>())
    .def("__delitem__", &public_logic::ComponentMap::delitem)
    .def("__iter__", &public_logic::ComponentMap::iter)
    .def("__getinitargs__", &public_logic::ComponentMap::getinitargs)
    ;
}
