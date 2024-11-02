// Copyright (c) 2024 ASMlover. All rights reserved.
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
#pragma once

#include <utility>

namespace owl {

template <typename ValType, ValType val> struct IntergralConstant {
  static constexpr ValType value = val;

  using ValueType = ValType;
  using Type      = IntergralConstant<ValueType, value>;

  constexpr operator ValueType() const noexcept { return value; }
  constexpr ValueType operator()() const noexcept { return value; }
};

using TrueType  = IntergralConstant<bool, true>;
using FalseType = IntergralConstant<bool, false>;

template <bool val>
using BoolConstant = IntergralConstant<bool, val>;

template <bool val>
struct BoolType : BoolConstant<val> {};

template <typename...> struct ParamTester {
  using Type      = void;
  using IsTypedef = TrueType;
};

template <typename... types>
using void_t = typename ParamTester<types...>::Type;

template <typename... types>
using typedef_t = typename ParamTester<types...>::IsTypedef;

template <bool val, typename = void> struct EnableIf {};
template <typename type_> struct EnableIf<true, type_> {
  using Type = type_;
};

template <bool test, typename type = void>
using EnableIf_t = typename EnableIf<test, type>::Type;

template <typename Type> struct TypeTraits {
  using HasTrivialDefaultConstructor  = FalseType;
  using HasTrivialCopyConstructor     = FalseType;
  using HasTrivialAssignmentOperator  = FalseType;
  using HasTrivialDestructor          = FalseType;
  using IsPODType                     = FalseType;
};

template <> struct TypeTraits<char> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<signed char> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<unsigned char> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<wchar_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<short> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<unsigned short> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<int> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<unsigned int> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<long> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<unsigned long> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<float> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<double> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <> struct TypeTraits<long double> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <typename Type> struct TypeTraits<Type*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <typename Type> struct TypeTraits<const Type*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignmentOperator  = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPODType                     = TrueType;
};

template <typename, typename = void> struct IteratorTraitsBase {
};

template <typename Iter> struct IteratorTraitsBase<Iter,
         void_t<typename Iter::IteratorCategory,
            typename Iter::ValueType,
            typename Iter::DifferenceType,
            typename Iter::Pointer,
            typename Iter::Reference>> {
  using IteratorCategory  = typename Iter::IteratorCategory;
  using ValueType         = typename Iter::ValueType;
  using DifferenceType    = typename Iter::DifferenceType;
  using Pointer           = typename Iter::Pointer;
  using Reference         = typename Iter::Reference;
  using MemoryCopyTag     = FalseType;
};

template <typename Iter> struct IteratorTraits : IteratorTraitsBase<Iter> {
  using MemoryCopyTag = FalseType;
};

template <typename Type> struct IteratorTraits<Type*> {
  using IteratorCategory  = std::random_access_iterator_tag;
  using ValueType         = Type;
  using DifferenceType    = ptrdiff_t;
  using Pointer           = Type*;
  using Reference         = Type&;
  using MemoryCopyTag     = TrueType;
};

template <typename Type> struct IteratorTraits<const Type*> {
  using IteratorCategory  = std::random_access_iterator_tag;
  using ValueType         = Type;
  using DifferenceType    = ptrdiff_t;
  using Pointer           = const Type*;
  using Reference         = const Type&;
  using MemoryCopyTag     = TrueType;
};

template <typename Iter> using IterVal_t  = typename IteratorTraits<Iter>::ValueType;
template <typename Iter> using IterDiff_t = typename IteratorTraits<Iter>::DifferenceType;
template <typename Iter> using IterCate_t = typename IteratorTraits<Iter>::IteratorCategory;

template <typename Iter, typename = void> struct IsIterator : FalseType {
};

template <typename Iter>
struct IsIterator<Iter, void_t<typename IteratorTraits<Iter>::IteratorCategory>> : TrueType {
};

template <typename Type1, typename Type2> struct IsSame : FalseType {};
template <typename Type> struct IsSame<Type, Type> : TrueType {};

template <bool, typename Type1, typename Type2> struct If {
  using Type = Type2;
};

template <typename Type1, typename Type2> struct If<true, Type1, Type2> {
  using Type = Type1;
};

template <typename ValueType, typename = void> struct IsTypedefPod : FalseType {
  using Type = FalseType;
};

template <typename ValueType> struct IsTypedefPod<ValueType, void_t<typename ValueType::IsPodTag>> : TrueType {
};

template <typename ValueType, typename = void>
struct IsPod : BoolType<TypeTraits<ValueType>::IsPODType::value ||
               If<IsTypedefPod<ValueType>::value, typename IsTypedefPod<ValueType>::Type, FalseType>::Type::value> {
};

template <typename Iter, typename = void> struct IsTypedefMemCopy : FalseType {
  using Type = FalseType;
};

template <typename Iter> struct IsTypedefMemCopy<Iter, void_t<typename Iter::MemoryCopyTag>> : TrueType {
  using Type = typename Iter::MemoryCopyTag;
};

template <typename Iter, typename = void>
struct IsMemCopy : BoolType<IteratorTraits<Iter>::MemoryCopyTag::value ||
                   If<IsTypedefMemCopy<Iter>::value, typename IsTypedefMemCopy<Iter>::Type, FalseType>::Type::value> {
};

}
