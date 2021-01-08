#pragma once

#include <iosfwd>

#include "express/parse_exp.h"

namespace express {

void generate_header(std::ostream&, schema const&, type const&);
void generate_source(std::ostream&, schema const&, type const&);

}  // namespace express