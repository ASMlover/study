#pragma once

#include <string>

namespace ms {

constexpr const char* kMapleCoreVersion = "0.0.0";
int RunScript(const std::string& source, std::string* error);

}  // namespace ms
