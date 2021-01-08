#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include "utl/parser/cstr.h"

#include "step/root_entity.h"

namespace step {

struct selective_entity_parser {
  using parser_fn_t = std::function<entity_ptr(utl::cstr)>;
  using parser_map_t = std::unordered_map<std::string_view, parser_fn_t>;

  std::optional<entity_ptr> parse(utl::cstr type_name, utl::cstr rest) const {
    if (auto const it = parsers_.find(type_name.view()); it != end(parsers_)) {
      return it->second(rest);
    }
    return std::nullopt;
  }

  template <typename... Ts>
  void register_parsers() {
    (register_parser<Ts>(), ...);
  }

  template <typename T>
  void register_parser() {
    parsers_[T::NAME] = [](utl::cstr s) {
      auto v = std::make_unique<T>();
      parse_step(s, *v);
      return v;
    };
  }

  parser_map_t parsers_;
};

}  // namespace step