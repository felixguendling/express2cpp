#pragma once

#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace express {

enum class data_type {
  UNKOWN,
  BOOL,
  LOGICAL,
  REAL,
  NUMBER,
  STRING,
  INTEGER,
  ENTITY,
  ENUM,
  SELECT,
  ALIAS
};
constexpr char const* data_type_str[] = {
    "UNKOWN",  "BOOL",   "LOGICAL", "REAL",   "NUMBER", "STRING",
    "INTEGER", "ENTITY", "ENUM",    "SELECT", "ALIAS"};

struct parser_exception : public std::exception {
  parser_exception(char const* from, char const* to) : from_{from}, to_{to} {}
  char const *from_, *to_;
};

struct member {
  std::string name_;
  std::string type_;
  bool optional_{false};
  bool list_{false};
  unsigned min_size_{0}, max_size_{std::numeric_limits<unsigned>::max()};
};

struct type {
  std::string name_;
  data_type data_type_{data_type::UNKOWN};
  std::vector<std::string> details_;
  std::string subtype_of_;
  std::vector<member> members_;
  bool list_{false};
  unsigned min_size_{0}, max_size_{std::numeric_limits<unsigned>::max()};
  std::string alias_;
};

struct schema {
  std::string name_;
  std::vector<type> types_;
  std::unordered_map<std::string, type const*> type_map_;
};

schema parse(std::string_view);

}  // namespace express