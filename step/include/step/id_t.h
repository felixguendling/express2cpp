#pragma once

#include <iosfwd>
#include <limits>

namespace step {

struct id_t {
  static constexpr auto const kInvalid = std::numeric_limits<unsigned>::max();
  id_t() = default;
  id_t(unsigned id) : id_{id} {}  // NOLINT
  static id_t invalid() { return {kInvalid}; }
  bool operator!=(id_t const& o) const { return o.id_ != id_; }
  bool operator==(id_t const& o) const { return o.id_ == id_; }
  bool operator!=(unsigned id) const { return id_ != id; }
  bool operator==(unsigned id) const { return id_ == id; }
  friend std::ostream& operator<<(std::ostream&, id_t const&);
  unsigned id_{kInvalid};
};

}  // namespace step