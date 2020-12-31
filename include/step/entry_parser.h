#pragma once

#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "cista/containers/hash_map.h"
#include "cista/reflection/for_each_field.h"

#include "utl/parser/arg_parser.h"
#include "utl/parser/cstr.h"
#include "utl/verify.h"

#include "step/parse_step.h"
#include "step/root_entity.h"

namespace step {

struct entry_parser {
  using parser_fn_t = std::function<entity_ptr(utl::cstr)>;
  using parser_map_t = cista::raw::hash_map<std::string_view, parser_fn_t>;

  template <typename... Ts>
  void register_parsers() {
    (register_parser<Ts>(), ...);
  }

  std::optional<std::pair<id_t, entity_ptr>> parse(utl::cstr line) {
    if (line.len == 0 || line[0] != '#') {
      return std::nullopt;
    }

    std::pair<id_t, entity_ptr> value;
    parse_step(line, value.first);

    line = line.skip_whitespace_front();
    utl::verify(line.len > 3 && line[0] == '=', "no '=' found: {}",
                line.view());

    auto const params_end = line.view().find(");");
    if (params_end == std::string_view::npos) {
      return std::nullopt;
    }

    line = line.substr(1, utl::size{params_end - 1});
    line = line.skip_whitespace_front();

    auto const bracket_pos = line.view().find('(');
    utl::verify(bracket_pos != std::string_view::npos,
                "no bracket '(' found in: {}", line.view());

    auto const type_name = line.substr(0, utl::size{bracket_pos}).view();
    if (auto const it = parsers_.find(type_name); it != end(parsers_)) {
      value.second = it->second(line.substr(bracket_pos + 1));
      return value;
    }

    return std::nullopt;
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