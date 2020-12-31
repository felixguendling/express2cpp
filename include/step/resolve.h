#pragma once

#include <optional>
#include <vector>

#include "step/is_collection.h"
#include "step/root_entity.h"

namespace step {

template <typename T>
void resolve(std::vector<root_entity*> const& v, T*& el) {
  auto const id = reinterpret_cast<std::size_t>(el);
  el = (v.size() <= id) ? nullptr : reinterpret_cast<T*>(v[id]);
}

template <typename T>
std::enable_if_t<is_collection<T>::value> resolve(
    std::vector<root_entity*> const& v, T& vec) {
  for (auto& el : vec) {
    resolve(v, el);
  }
}

template <typename T>
void resolve(std::vector<root_entity*> const& v, std::optional<T>& opt) {
  if (opt.has_value()) {
    resolve(v, *opt);
  }
}

}  // namespace step