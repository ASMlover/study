// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __ESTL_TYPE_TRAITS_HEADER_H__
#define __ESTL_TYPE_TRAITS_HEADER_H__

namespace estl {

struct TrueType {};
struct FalseType {};

template <typename T>
struct TypeTraits {
  typedef FalseType HasTrivialDefaultConstructor;
  typedef FalseType HasTrivialCopyConstructor;
  typedef FalseType HasTrivialAssignmentOperator;
  typedef FalseType HasTrivialDestructor;
  typedef FalseType IsPODType;
};

template <>
struct TypeTraits<bool> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<char> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<signed char> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned char> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<wchar_t> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<short> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned short> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<int> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned int> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<long> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned long> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<long long> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned long long> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<float> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<double> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<long double> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <typename T>
struct TypeTraits<T*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <typename T>
struct TypeTraits<const T*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<signed char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<unsigned char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<const char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<const signed char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

template <>
struct TypeTraits<const unsigned char*> {
  typedef TrueType HasTrivialDefaultConstructor;
  typedef TrueType HasTrivialCopyConstructor;
  typedef TrueType HasTrivialAssignmentOperator;
  typedef TrueType HasTrivialDestructor;
  typedef TrueType IsPODType;
};

}

#endif  // __ESTL_TYPE_TRAITS_HEADER_H__
