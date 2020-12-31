#include "step/get_next_id.h"

#include "step/get_next_token.h"

namespace step {

std::optional<step::id_t> get_next_id(utl::cstr& s) {
  auto const l = get_next_token(s, '#');
  if (!l.has_value() || l->len < 1 || !std::isdigit((*l)[0])) {
    return std::nullopt;
  }

  s = *l;
  auto id = step::id_t{};
  utl::parse_arg(s, id.id_);

  return id;
}

}  // namespace step