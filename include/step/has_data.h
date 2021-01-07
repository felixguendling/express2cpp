#pragma once

#include <type_traits>

namespace step {

template <typename T, typename = void>
struct has_data : std::false_type {};

template <typename T>
struct has_data<T, decltype(std::declval<T>().data_, void())> : std::true_type {
};

}  // namespace step