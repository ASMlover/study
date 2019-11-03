// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <ctime>
#include "wren.hh"
#include "vm.hh"
#include "random.hh"

namespace wrencc {

#include "random.wren.hh"

namespace random {
  namespace details {
    // implements the well equidistributed long-perild linear PRNG (WELL512a)
    //
    // https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
    struct Well512 {
      u32_t state[16];
      u32_t index;
    };

    u32_t advance_state(Well512& well) {
      u32_t a, b, c, d;
      a = well.state[well.index];
      c = well.state[(well.index + 13) & 15];
      b = a ^ c ^ (a << 16) ^ (c << 15);
      c = well.state[(well.index + 9) & 15];
      c ^= (c >> 11);
      a = well.state[well.index] = b ^ c;
      d = a ^ ((a << 5) & 0xda442d24U);

      well.index = (well.index + 15) & 15;
      a = well.state[well.index];
      well.state[well.index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
      return well.state[well.index];
    }

    void random_allocate(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(
          wrenSetSlotNewForeign(*vm, 0, 0, sizeof(Well512)));
      well->index = 0;
    }

    void random_seed0(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(wrenGetSlotForeign(*vm, 0));

      std::srand(Xt::as_type<u32_t>(std::time(nullptr)));
      for (int i = 0; i < 16; ++i)
        well->state[i] = std::rand();
    }

    void random_seed1(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(wrenGetSlotForeign(*vm, 0));

      std::srand(Xt::as_type<u32_t>(wrenGetSlotDouble(*vm, 1)));
      for (int i = 0; i < 16; ++i)
        well->state[i] = std::rand();
    }

    void random_seed16(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(wrenGetSlotForeign(*vm, 0));

      for (int i = 0; i < 16; ++i)
        well->state[i] = Xt::as_type<u32_t>(wrenGetSlotDouble(*vm, i + 1));
    }

    void random_float(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(wrenGetSlotForeign(*vm, 0));

      double result = Xt::as_type<double>(advance_state(*well)) * (1 << 21);
      result += Xt::as_type<double>(advance_state(*well) & ((1 << 21) - 1));
      result /= 9007199254740992.0;

      wrenSetSlotDouble(*vm, 0, result);
    }

    void random_int0(WrenVM* vm) {
      Well512* well = Xt::as_type<Well512*>(wrenGetSlotForeign(*vm, 0));
      wrenSetSlotDouble(*vm, 0, advance_state(*well));
    }

    inline WrenForeignClass bind_foreign_class(
        WrenVM& vm, const str_t& module, const str_t& class_name) {
      ASSERT(class_name == "Random", "should be in Random class");
      return WrenForeignClass{random_allocate, nullptr};
    }

    inline WrenForeignFn bind_foreign_method(WrenVM& vm,
        const str_t& class_name, bool is_static, const str_t& signature) {
      ASSERT(class_name == "Random", "should be in Random class");

      if (signature == "<allocate>")
        return random_allocate;
      if (signature == "seed()")
        return random_seed0;
      if (signature == "seed(_)")
        return random_seed1;
      if (signature == "seed(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)")
        return random_seed16;
      if (signature == "float()")
        return random_float;
      if (signature == "int()")
        return random_int0;

      ASSERT(false, "unknown method");
      return nullptr;
    }
  }

  const str_t get_source(void) {
#if AUX_RANDOM
    return kLibSource;
#endif
    return "";
  }

  WrenForeignClass bind_foreign_class(
      WrenVM& vm, const str_t& module, const str_t& class_name) {
#if AUX_RANDOM
    return details::bind_foreign_class(vm, module, class_name);
#endif
    return WrenForeignClass{nullptr, nullptr};
  }

  WrenForeignFn bind_foreign_method(WrenVM& vm,
      const str_t& class_name, bool is_static, const str_t& signature) {
#if AUX_RANDOM
    return details::bind_foreign_method(vm, class_name, is_static, signature);
#endif
    return nullptr;
  }
}

}
