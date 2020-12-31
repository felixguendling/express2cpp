#include <queue>
#include <unordered_map>

#include "express/get_subtypes_of.h"

namespace express {

std::set<std::string_view> get_subtypes_of(schema const& s,
                                           std::string_view supertype) {
  std::unordered_map<std::string_view, std::vector<std::string_view>> subtypes;
  for (auto const& t : s.types_) {
    subtypes[t.subtype_of_].emplace_back(t.name_);
  }

  std::queue<std::string_view> q;
  q.emplace(supertype);

  std::set<std::string_view> rec_subtypes;
  while (!q.empty()) {
    auto const next = q.front();
    q.pop();
    if (!rec_subtypes.emplace(next).second) {
      continue;
    }
    if (auto const it = subtypes.find(next); it != end(subtypes)) {
      for (auto const& n : it->second) {
        q.emplace(n);
      }
    }
  }
  return rec_subtypes;
}

}  // namespace express