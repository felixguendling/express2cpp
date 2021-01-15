#pragma once

#include <ostream>
#include <type_traits>
#include <unordered_map>

#include "cista/type_hash/type_name.h"

#include "utl/enumerate.h"
#include "utl/verify.h"

#include "step/id_t.h"
#include "step/is_collection.h"

namespace step {

struct model;
struct root_entity;

struct write_context {
  std::unordered_map<root_entity*, step::id_t> ptr_to_id_;
};

void write(std::ostream&, model const&);

template <class, typename = void>
struct is_comparable : std::false_type {};

template <class T>
struct is_comparable<
    T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>>
    : std::true_type {};

template <typename T>
void write(write_context const& ctx, std::ostream& out, T const& e) {
  using Type = std::decay_t<T>;
  if constexpr (std::is_base_of_v<root_entity, Type>) {
    e.write(ctx, out, true);
  } else if constexpr (std::is_pointer_v<Type>) {
    if (e != nullptr) {
      auto const it = ctx.ptr_to_id_.find(e);
      utl::verify(it != end(ctx.ptr_to_id_), "could not resolve {}* {}",
                  cista::type_str<Type>(), static_cast<void const*>(e));
      out << "#" << it->second.id_;
    }
  } else if constexpr (std::is_same_v<std::string, Type>) {
    out << '\'' << e << '\'';
  } else if constexpr (is_collection<Type>::value) {
    out << "(";
    for (auto const& [i, el] : utl::enumerate(e)) {
      if constexpr (is_comparable<std::decay_t<decltype(el)>>::value) {
        if (el != decltype(el){}) {
          write(ctx, out, el);
          if (i != e.size() - 1) {
            out << ", ";
          }
        }
      } else {
        write(ctx, out, el);
        if (i != e.size() - 1) {
          out << ", ";
        }
      }
    }
    out << ")";
  } else {
    out << e;
  }
}

template <typename T>
void write(write_context const& ctx, std::ostream& out,
           std::optional<T> const& e) {
  if (e.has_value()) {
    write(ctx, out, *e);
  } else {
    out << "$";
  }
}

}  // namespace step