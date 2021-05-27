#include "step/logical.h"

#include <ostream>

namespace step {

std::ostream& operator<<(std::ostream& out, logical const l) {
  switch (l) {
    case logical::FALSE: return out << ".F.";
    case logical::TRUE: return out << ".T.";
    case logical::UNKNOWN: [[fallthrough]];
    default: return out << ".U.";
  }
}

}  // namespace step
