#pragma once

#include <memory>
#include <vector>

namespace step {

struct root_entity {
  virtual ~root_entity() = default;
  virtual void resolve(std::vector<root_entity*> const&) = 0;
  std::size_t line_idx_;
};

using entity_ptr = std::unique_ptr<root_entity>;

}  // namespace step