#pragma once

#include <set>
#include <string>

#include "express/parse_exp.h"

namespace express {

std::set<std::string_view> get_subtypes_of(schema const&,
                                           std::string_view supertype);

}  // namespace express