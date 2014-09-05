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
#include "el_unit.h"

namespace el {

UnitFramework::UnitFramework(void) {
}

UnitFramework::~UnitFramework(void) {
}

int UnitFramework::Run(void) {
  fprintf(stdout, "====================BEGIN====================\n");
  for (const auto& unit : unit_list_) {
    ColorPrintf(ColorType::COLORTYPE_GREEN, 
        "\tRun UnitCase : %s\n", unit.unit_name);
    unit.unit_case();
    ColorPrintf(ColorType::COLORTYPE_GREEN, 
        "\tEnd UnitCase : %s\n", unit.unit_name);
    fprintf(stdout, "=============================================\n");
  }

  return 0;
}

bool UnitFramework::RegisterUnit(
    const char* name, const UnitCase::UnitType& unit) {
  if (nullptr == name || nullptr == unit)
    return false;

  unit_list_.push_back(UnitCase(name, unit));
  return true;
}

int UnitPrint(const char* format, ...) {
  va_list ap;
  char buffer[2048];

  va_start(ap, format);
  vsnprintf(buffer, 2048, format, ap);
  va_end(ap);

  return fprintf(stdout, "\t\t%s", buffer);
}

}
