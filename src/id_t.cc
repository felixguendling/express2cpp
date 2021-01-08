#include "step/id_t.h"

#include <ostream>

namespace step {

std::ostream& operator<<(std::ostream& out, id_t const& id) {
  return out << '#' << id.id_;
}

}  // namespace step