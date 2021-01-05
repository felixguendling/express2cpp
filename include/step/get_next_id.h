#pragma once

#include <optional>

#include "utl/parser/arg_parser.h"

#include "step/id_t.h"

namespace step {

std::optional<step::id_t> get_next_id(utl::cstr& s);

}  // namespace step