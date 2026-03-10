#pragma once

#include <string>

namespace ms {

constexpr const char* kMapleCoreVersion = "0.0.0";
int run_script(const std::string& source, std::string* error);

}  // namespace ms
