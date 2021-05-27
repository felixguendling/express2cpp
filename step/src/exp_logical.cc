#include "step/exp_logical.h"

#include <ostream>

namespace step {

std::ostream& operator<<(std::ostream& out, exp_logical const l) {
  switch (l) {
    case exp_logical::EXP_FALSE: return out << ".F.";
    case exp_logical::EXP_TRUE: return out << ".T.";
    case exp_logical::EXP_UNKNOWN: [[fallthrough]];
    default: return out << ".U.";
  }
}

}  // namespace step
