#pragma once

#include "cista/reflection/comparable.h"

namespace step {

struct id_t {
  CISTA_COMPARABLE()
  static id_t invalid() { return {std::numeric_limits<unsigned>::max()}; }
  unsigned id_;
};

}  // namespace step