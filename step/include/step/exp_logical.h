#pragma once

#include <iosfwd>

namespace step {

enum class exp_logical { EXP_TRUE, EXP_FALSE, EXP_UNKNOWN };

std::ostream& operator<<(std::ostream&, exp_logical);

}  // namespace step
