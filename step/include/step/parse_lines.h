#pragma once

#include <optional>

#include "utl/enumerate.h"
#include "utl/parser/cstr.h"

#include "fmt/core.h"

#include "step/model.h"
#include "step/root_entity.h"
#include "step/split_line.h"

namespace step {

template <typename Parser>
model parse_lines(Parser const& p, utl::cstr step) {
  model m;
  for (auto [line_idx, line] : utl::enumerate(utl::lines{step})) {
    try {
      auto const split = split_line(line);
      if (!split.has_value()) {
        continue;
      }

      auto entity = p.parse(split->name_, split->entity_);
      if (!entity.has_value()) {
        continue;
      }

      auto* const e_ptr = m.entity_mem_.emplace_back(std::move(*entity)).get();
      e_ptr->id_ = split->id_;
      if (m.id_to_entity_.size() <= split->id_.id_) {
        m.id_to_entity_.resize(split->id_.id_ + 1);
      }
      m.id_to_entity_[split->id_.id_] = e_ptr;
    } catch (std::exception const& e) {
      fmt::print("unable to parse line {}: {}", line_idx + 1, line.view());
    }
  }
  for (auto& ptr : m.entity_mem_) {
    ptr->resolve(m.id_to_entity_);
  }
  return m;
}

}  // namespace step