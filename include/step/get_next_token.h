#pragma once

#include <optional>

#include "utl/parser/cstr.h"

namespace step {

std::optional<utl::cstr> get_next_token(utl::cstr const s, char token);

}  // namespace step