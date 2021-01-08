#pragma once

#include <variant>

#include "step/has_data.h"
#include "step/root_entity.h"

namespace step {

namespace {

template <typename Fn, typename... T, std::size_t... Is>
bool iterate_variant_impl(Fn&& f, std::variant<T...> v,
                          std::index_sequence<Is...>) {
  return (f(Is, std::variant_alternative_t<Is, decltype(v)>()) || ...);
}

template <typename Fn, typename... T>
bool iterate_variant(Fn&& f, std::variant<T...> v) {
  return iterate_variant_impl(std::forward<Fn>(f), std::forward<decltype(v)>(v),
                              std::index_sequence_for<T...>());
}

}  // namespace

template <typename T>
bool assign_entity_ptr_to_select(T& t, root_entity* entity) {
  return iterate_variant(
      [&](std::size_t const index, auto&& el) {
        using Type = std::decay_t<decltype(el)>;
        if constexpr (std::is_pointer_v<Type>) {
          el = dynamic_cast<Type>(entity);
          if (el != nullptr) {
            t.data_ = el;
            return true;
          }
        } else if constexpr (has_data<Type>::value) {
          if (assign_entity_ptr_to_select(el, entity)) {
            t.data_ = el;
            return true;
          }
        }
        return false;
      },
      t.data_);
}

}  // namespace step