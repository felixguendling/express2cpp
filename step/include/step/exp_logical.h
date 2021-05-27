#pragma once

#include <iosfwd>

namespace step {

enum class exp_logical { TRUE, FALSE, UNKNOWN };

std::ostream& operator<<(std::ostream&, exp_logical);

}  // namespace step
