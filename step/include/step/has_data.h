#pragma once

#include <type_traits>

namespace step {

template <typename T, typename = void>
struct has_data : std::false_type {};

template <typename T>
struct has_data<T, std::void_t<decltype(std::declval<T>().data_)>>
    : std::true_type {};

}  // namespace step