// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "el_test.h"
#include "../el_time.h"
#include "../el_logging.h"

UNIT_IMPL(Logging) {
  LOG_DEBUG("UNIT_IMPL(Logging) -> LOG_DEBUG\n");
  LOG_DEBUGX("UNIT_IMPL(Logging) -> LOG_DEBUGX\n");

  LOG_MSG("UNIT_IMPL(Logging) -> LOG_MSG\n");
  LOG_MSGX("UNIT_IMPL(Logging) -> LOG_MSGX\n");

  LOG_WARN("UNIT_IMPL(Logging) -> LOG_WARN\n");
  LOG_WARNX("UNIT_IMPL(Logging) -> LOG_WARNX\n");

  LOG_ERR("UNIT_IMPL(Logging) -> LOG_ERR\n");
  LOG_ERRX("UNIT_IMPL(Logging) -> LOG_ERRX\n");

  LOG_FAIL("UNIT_IMPL(Logging) -> LOG_FAILX\n");
  LOG_FAILX("UNIT_IMPL(Logging) -> LOG_FAILX\n");
}
