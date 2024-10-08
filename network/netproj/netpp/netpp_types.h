// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <cstdint>
#include <functional>
#include <system_error>
#include <Chaos/Base/Platform.h>

#if defined(CHAOS_WINDOWS)
# if defined(CHAOS_ARCH64)
    using socket_t = std::uint64_t;
    using ssize_t = std::int64_t;
# else
    using socket_t = std::uint32_t;
    using ssize_t = long;
# endif
  using socklen_t = int;
#else
  using socket_t = int;
#endif

namespace netpp {

static constexpr socket_t kInvalidSocket = (socket_t)(~0);
static constexpr int kSocketError = -1;

using AcceptHandler = std::function<void (const std::error_code&)>;
using ConnectHandler = std::function<void (const std::error_code&)>;
using ReadHandler = std::function<void (const std::error_code&, std::size_t)>;
using WriteHandler = std::function<void (const std::error_code&, std::size_t)>;

}
