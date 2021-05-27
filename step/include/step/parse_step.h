#pragma once

#include <optional>
#include <variant>

#include "boost/algorithm/string.hpp"

#include "cista/type_hash/type_name.h"

#include "utl/parser/arg_parser.h"
#include "utl/parser/cstr.h"
#include "utl/verify.h"

#include "step/id_t.h"
#include "step/is_collection.h"
#include "step/logical.h"

namespace step {

template <class, class = void>
struct has_resize : std::false_type {};

template <class T>
struct has_resize<T, std::void_t<decltype(std::declval<T>().resize(0))>>
    : std::true_type {};

inline std::optional<utl::cstr> get_next_token(utl::cstr const s, char token) {
  if (s.len == 0) {
    return std::nullopt;
  }

  auto const* const pos =
      static_cast<char const*>(std::memchr(s.str, token, s.len));
  if (pos == nullptr) {
    return std::nullopt;
  }

  return utl::cstr{pos + 1, s.len - (pos - s.str) - 1};
}

inline void parse_step(utl::cstr& in, id_t& i) {
  utl::verify(in.len > 1 && (in[0] == '$' ||
                             (in[0] == '#' && (std::isdigit(in[1]) != 0))),
              "expected id, got: {}", in.view());
  if (in[0] == '$') {
    ++in;
    fmt::print("warning: id set to $, unable to resolve (-> nullptr)\n");
    i.id_ = id_t::kInvalid;
  } else {
    ++in;
    utl::parse_arg(in, i.id_);
  }
}

template <typename T>
void parse_step(utl::cstr& s, T*& ptr) {
  auto id = step::id_t{};
  parse_step(s, id);
  ptr = reinterpret_cast<T*>(static_cast<uintptr_t>(id.id_));
}

inline void parse_step(utl::cstr& s, double& val) {
  char* end = nullptr;
  val = std::strtod(s.str, &end);
  s.len -= end - s.str;
  s.str = end;
}

template <typename T>
std::enable_if_t<std::is_integral_v<T>> parse_step(utl::cstr& s, T& val) {
  utl::parse_arg(s, val);
}

inline void parse_step(utl::cstr& s, bool& val) {
  utl::verify(s.len > 0 && s[0] == '.', "expected bool, got {}", s.view());
  ++s;

  utl::verify(s.len > 0 && (s[0] == 'T' || s[0] == 'F'),
              "expected bool '.T.' or '.F.', got {}", s.view());
  val = s[0] == 'T';
  ++s;

  utl::verify(s.len > 0 && s[0] == '.', "expected bool, got {}", s.view());
  ++s;
}

inline void parse_step(utl::cstr& s, logical& val) {
  utl::verify(s.len > 0 && s[0] == '.', "expected bool, got {}", s.view());
  ++s;

  utl::verify(s.len > 0 && (s[0] == 'T' || s[0] == 'F' || s[0] == 'U'),
              "expected logical '.T.', '.F.', or '.U.', got {}", s.view());
  switch (s[0]) {
    case 'T': val = logical::TRUE;
    case 'F': val = logical::FALSE;
    case 'U': val = logical::UNKNOWN;
  }
  ++s;

  utl::verify(s.len > 0 && s[0] == '.', "expected bool, got {}", s.view());
  ++s;
}

inline void parse_step(utl::cstr& s, std::string& str) {
  utl::verify(s.len > 0 && s[0] == '\'', "string starts with ', got {}",
              s.view());
  ++s;

  auto const end = get_next_token(s, '\'');
  utl::verify(end.has_value(), "string ends with ', got {}", s.view());

  str = std::string{s.data(), static_cast<size_t>(end->data() - s.data() - 1)};
  s = *end;
}

template <typename T>
std::enable_if_t<is_collection<T>::value> parse_step(utl::cstr& s, T& v) {
  if (s.len != 0 && s[0] == '$') {  // invalid IFC handled gracefully
    ++s;
    return;
  }

  utl::verify(s.len != 0 && s[0] == '(', "set begins with (, got {}", s.view());
  ++s;
  auto i = 0U;
  while (s.len > 0 && s[0] != ')') {
    if constexpr (has_resize<T>::value) {
      v.resize(i + 1);
    }
    parse_step(s, v[i]);
    if (s.len > 0 && s[0] == ',') {
      ++s;
      s = s.skip_whitespace_front();
    }
    ++i;
  }
  utl::verify(s.len != 0 && s[0] == ')', "set ends with ), got {}",
              s.len > 0 ? s[0] : '?');
  ++s;
}

template <typename T>
std::enable_if_t<std::is_enum_v<T>> parse_step(utl::cstr& s, T& v) {}

template <typename T>
void parse_step(utl::cstr& s, std::optional<T>& o) {
  utl::verify(s.len > 0, "expected optional {}, got {}", cista::type_str<T>(),
              s.view());
  if (s[0] == '$') {
    ++s;
    o = std::nullopt;
  } else {
    T arg;
    parse_step(s, arg);
    o = std::move(arg);
  }
}

}  // namespace step
