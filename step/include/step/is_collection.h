#pragma once

#include <iterator>
#include <type_traits>

namespace step {

template <class, typename = void>
struct is_collection : std::false_type {};

template <class T>
struct is_collection<T, std::void_t<decltype(std::begin(std::declval<T>()))>>
    : std::true_type {};

}  // namespace step