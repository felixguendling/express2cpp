#pragma once

#include <iosfwd>

namespace step {

enum class logical { TRUE, FALSE, UNKNOWN };

std::ostream& operator<<(std::ostream&, logical);

}  // namespace step
