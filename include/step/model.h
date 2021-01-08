#pragma once

#include <memory>
#include <vector>

#include "utl/verify.h"

#include "step/id_t.h"

namespace step {

struct root_entity;

struct model {
  template <typename T>
  T const& get_entity(step::id_t const& id) {
    utl::verify(id_to_entity_.size() > id.id_, "invalid id");
    auto const* const entity = dynamic_cast<T*>(id_to_entity_[id.id_]);
    utl::verify(entity != nullptr, "bad cast");
    return *entity;
  }

  std::vector<root_entity*> id_to_entity_;
  std::vector<std::unique_ptr<root_entity>> entity_mem_;
};

}  // namespace step