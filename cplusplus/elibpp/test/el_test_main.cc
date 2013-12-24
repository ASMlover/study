//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "el_test_header.h"



UnitFramework::UnitFramework(void)
{
}

UnitFramework::~UnitFramework(void)
{
}

int 
UnitFramework::Run(void)
{
  fprintf(stdout, "=====================BEGIN====================\n");

  size_t size = unit_list_.size();
  for (size_t i = 0; i < size; ++i) {
    el::ColorPrintf(el::kColorTypeGreen, 
        "\tRun UnitCase : %s\n", unit_list_[i].unit_name);
    unit_list_[i].unit_case();
    el::ColorPrintf(el::kColorTypeGreen, 
        "\tEnd UnitCase : %s\n", unit_list_[i].unit_name);
    fprintf(stdout, "==============================================\n\n");
  }

  return 0;
}

bool 
UnitFramework::RegisterUnit(const char* name, void (*unit)(void))
{
  if (NULL == name || NULL == unit)
    return false;

  unit_list_.push_back(UnitCase(name, unit));
  return true;
}




int 
main(int argc, char* argv[])
{
  return UNIT_RUN_ALL();
}
