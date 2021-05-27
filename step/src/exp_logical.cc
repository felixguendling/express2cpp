#include "step/exp_logical.h"

#include <ostream>

namespace step {

std::ostream& operator<<(std::ostream& out, exp_logical const l) {
  switch (l) {
    case exp_logical::FALSE: return out << ".F.";
    case exp_logical::TRUE: return out << ".T.";
    case exp_logical::UNKNOWN: [[fallthrough]];
    default: return out << ".U.";
  }
}

}  // namespace step
