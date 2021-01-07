#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "cista/containers/hash_map.h"

#include "utl/enumerate.h"
#include "utl/parser/cstr.h"

#include "step/entry_parser.h"
#include "step/get_next_id.h"
#include "step/id_t.h"
#include "step/is_collection.h"
#include "step/resolve.h"
#include "step/root_entity.h"

namespace step {

struct entity_map {
  explicit entity_map(entry_parser& parser, utl::cstr s) {
    for (auto [line_idx, line] : utl::enumerate(utl::lines{s})) {
      try {
        lines_.emplace_back(line.view());
        auto p = parser.parse(line);
        if (p.has_value()) {
          auto& [id, entity] = *p;
          add_entity(id, line_idx, std::move(entity));
        }
      } catch (std::exception const& e) {
        std::cerr << "unable to parse line " << (line_idx + 1) << ": "
                  << line.view() << "\n";
      }
    }
    resolve_all();
  }

  void add_line(std::string_view line) { lines_.emplace_back(line); }

  void add_entity(step::id_t const& id, unsigned const line_idx,
                  entity_ptr&& e) {
    auto* const e_ptr = entity_mem_.emplace_back(std::move(e)).get();

    if (id_to_entity_.size() <= id.id_) {
      id_to_entity_.resize(id.id_ + 1);
    }
    id_to_entity_[id.id_] = e_ptr;

    if (line_to_entity_.size() <= line_idx) {
      line_to_entity_.resize(line_idx + 1);
    }
    line_to_entity_[line_idx] = e_ptr;
  }

  void resolve_all() {
    for (auto& ptr : entity_mem_) {
      ptr->resolve(id_to_entity_);
    }
  }

  template <typename T>
  T const& get_entity(step::id_t const& id) {
    utl::verify(id_to_entity_.size() > id.id_, "invalid id");
    auto const* const entity = dynamic_cast<T*>(id_to_entity_[id.id_]);
    utl::verify(entity != nullptr, "bad cast");
    return *entity;
  }

  template <typename T>
  T const* get_entity_ptr(step::id_t const& id) {
    return &get_entity<T>(id);
  }

  std::vector<std::string_view> lines_;
  std::vector<root_entity*> id_to_entity_;
  std::vector<root_entity*> line_to_entity_;
  std::vector<entity_ptr> entity_mem_;
};

}  // namespace step