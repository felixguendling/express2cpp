#pragma once

#include <optional>

#include "utl/parser/cstr.h"

#include "step/id_t.h"

namespace step {

struct line {
  step::id_t id_;
  utl::cstr name_, entity_;
};

std::optional<line> split_line(utl::cstr);

}  // namespace step