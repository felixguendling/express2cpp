#pragma once

#include <memory>
#include <vector>

namespace step {

struct root_entity {
  root_entity() = default;
  root_entity(root_entity const&) = delete;
  root_entity(root_entity&&) = delete;
  root_entity& operator=(root_entity const&) = delete;
  root_entity& operator=(root_entity&&) = delete;
  virtual ~root_entity() = default;
  virtual void resolve(std::vector<root_entity*> const&) = 0;
  std::size_t line_idx_{0U};
};

using entity_ptr = std::unique_ptr<root_entity>;

}  // namespace step