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
#include <boost/python.hpp>
#include "fix16_wrapper.h"

namespace py = ::boost::python;

namespace fixmath_wrapper {

void fixmath_wrap(void) {
  PyEval_InitThreads();

  py::class_<fixmath_wrapper::fix16>("fix16", py::init<>())
    .def(py::init<std::int16_t>())
    .def(py::init<float>())
    .def(py::init<double>())
    .def(py::self + py::self)
    .def(py::self - py::self)
    .def(py::self * py::self)
    .def(py::self / py::self)
    .def(py::self > py::self)
    .def(py::self >= py::self)
    .def(py::self < py::self)
    .def(py::self <= py::self)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def("__repr__", &fixmath_wrapper::fix16::as_repr)
    .def("__str__", &fixmath_wrapper::fix16::as_str)
    .def("__int__", &fixmath_wrapper::fix16::as_int)
    .def("__float__", &fixmath_wrapper::fix16::as_float)
    .def("__copy__", &fixmath_wrapper::fix16::copy)
    .def("clone", &fixmath_wrapper::fix16::copy)
    .def("sin", &fixmath_wrapper::fix16::sin)
    .def("cos", &fixmath_wrapper::fix16::cos)
    .def("tan", &fixmath_wrapper::fix16::tan)
    .def("asin", &fixmath_wrapper::fix16::asin)
    .def("acos", &fixmath_wrapper::fix16::acos)
    .def("atan", &fixmath_wrapper::fix16::atan)
    .def("atan2", &fixmath_wrapper::fix16::atan2)
    .def("sin_parabola", &fixmath_wrapper::fix16::sin_parabola)
    .def("rad2deg", &fixmath_wrapper::fix16::rad2deg)
    .def("deg2rad", &fixmath_wrapper::fix16::deg2rad)
    .def("sqrt", &fixmath_wrapper::fix16::sqrt)
    .def("square", &fixmath_wrapper::fix16::square)
    .def("exp", &fixmath_wrapper::fix16::exp)
    .def("log", &fixmath_wrapper::fix16::log)
    .def("log2", &fixmath_wrapper::fix16::log2)
    .def("abs", &fixmath_wrapper::fix16::abs)
    .def("floor", &fixmath_wrapper::fix16::floor)
    .def("ceil", &fixmath_wrapper::fix16::ceil)
    .def("min", &fixmath_wrapper::fix16::min)
    .def("max", &fixmath_wrapper::fix16::max)
    ;

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
}

}

BOOST_PYTHON_MODULE(_fixmath) {
  fixmath_wrapper::fixmath_wrap();
}
