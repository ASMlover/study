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
#include "fix16_wrapper.h"

namespace py = ::boost::python;

namespace fixmath_wrapper {

void fixmath_wrap(void) {
  PyEval_InitThreads();

  fixmath_wrapper::fix16::wraps();

  py::class_<fixmath_wrapper::vector2>("vector2", py::init<>())
    .def(py::init<const fixmath_wrapper::fix16&,
        const fixmath_wrapper::fix16&>())
    .def(py::init<const fixmath_wrapper::fix16&>())
    .def_readwrite("x", &fixmath_wrapper::vector2::x_)
    .def_readwrite("y", &fixmath_wrapper::vector2::y_)
    .def(py::self + py::self)
    .def(py::self - py::self)
    .def(py::self * py::self)
    .def(py::self / py::self)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def(py::self * py::other<fixmath_wrapper::fix16>())
    .def("__repr__", &fixmath_wrapper::vector2::as_repr)
    .def("__str__", &fixmath_wrapper::vector2::as_str)
    .def("__copy__", &fixmath_wrapper::vector2::copy)
    .def("clone", &fixmath_wrapper::vector2::copy)
    .add_property("length_squared",
        &fixmath_wrapper::vector2::get_length_squared)
    .add_property("length",
        &fixmath_wrapper::vector2::get_length,
        &fixmath_wrapper::vector2::set_length)
    ;

  py::class_<fixmath_wrapper::vector3>("vector3", py::init<>())
    .def(py::init<const fixmath_wrapper::fix16&,
        const fixmath_wrapper::fix16&, const fixmath_wrapper::fix16&>())
    .def(py::init<const fixmath_wrapper::fix16&>())
    .def_readwrite("x", &fixmath_wrapper::vector3::x_)
    .def_readwrite("y", &fixmath_wrapper::vector3::y_)
    .def_readwrite("z", &fixmath_wrapper::vector3::z_)
    .def(py::self + py::self)
    .def(py::self - py::self)
    .def(py::self * py::self)
    .def(py::self / py::self)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def(py::self * py::other<fixmath_wrapper::fix16>())
    .def("__repr__", &fixmath_wrapper::vector3::as_repr)
    .def("__str__", &fixmath_wrapper::vector3::as_str)
    .def("__copy__", &fixmath_wrapper::vector3::copy)
    .def("clone", &fixmath_wrapper::vector3::copy)
    .def("dot", &fixmath_wrapper::vector3::dot)
    .def("set_pitch_yaw", &fixmath_wrapper::vector3::set_pitch_yaw)
    .add_property("length_squared",
        &fixmath_wrapper::vector3::get_length_squared)
    .add_property("length",
        &fixmath_wrapper::vector3::get_length,
        &fixmath_wrapper::vector3::set_length)
    .add_property("pitch",
        &fixmath_wrapper::vector3::get_pitch,
        &fixmath_wrapper::vector3::set_pitch)
    .add_property("yaw",
        &fixmath_wrapper::vector3::get_yaw, &fixmath_wrapper::vector3::set_yaw)
    ;
}

}

BOOST_PYTHON_MODULE(_fixmath) {
  fixmath_wrapper::fixmath_wrap();
}
