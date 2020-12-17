module;
#include "common.hh"

export module common;

export namespace tadpole {

str_t as_string(double d) noexcept {
  ss_t ss;
  ss << d;
  return ss.str();
}

}