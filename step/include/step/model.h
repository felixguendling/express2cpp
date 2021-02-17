#pragma once

#include <memory>
#include <vector>

#include "utl/verify.h"

#include "step/id_t.h"

namespace step {

struct root_entity;

struct model {
  template <typename T>
  T const& get_entity(step::id_t const& id) const {
    return const_cast<model*>(this)->get_entity<T>(id);  // NOLINT
  }

  template <typename T>
  T& get_entity(step::id_t const& id) {
    utl::verify(id_to_entity_.size() > id.id_, "invalid id");
    auto* const entity = dynamic_cast<T*>(id_to_entity_[id.id_]);
    utl::verify(entity != nullptr, "bad cast");
    return *entity;
  }

  template <typename T>
  T& add_entity() {
    auto const e = entity_mem_.emplace_back(std::make_unique<T>()).get();
    e->id_ = id_to_entity_.size();
    id_to_entity_.push_back(e);
    return *static_cast<T*>(e);
  }

  std::vector<root_entity*> id_to_entity_;
  std::vector<std::unique_ptr<root_entity>> entity_mem_;
};

}  // namespace step