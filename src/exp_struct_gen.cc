#include "express/exp_struct_gen.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ostream>

#include "boost/algorithm/string.hpp"

#include "cista/hash.h"

#include "utl/enumerate.h"

namespace express {

static auto const special = std::map<std::string, std::string>{
    {"BOOLEAN", "bool"}, {"LOGICAL", "bool"},       {"REAL", "double"},
    {"INTEGER", "int"},  {"STRING", "std::string"}, {"BINARY(32)", "uint32_t"}};

std::optional<std::string> is_special(schema const& s,
                                      std::string const& type_name) {
  if (auto const type_it = s.type_map_.find(type_name);
      type_it != end(s.type_map_)) {
    auto const& t = type_it->second;
    switch (t->data_type_) {
      case data_type::ALIAS: return is_special(s, t->alias_);
      case data_type::BOOL: [[fallthrough]];
      case data_type::LOGICAL: return "bool";
      case data_type::REAL: [[fallthrough]];
      case data_type::NUMBER: return "double";
      case data_type::STRING: return "std::string";
      case data_type::INTEGER: return "int";
      case data_type::ENUM: return type_name;
      case data_type::ENTITY:
      case data_type::SELECT: return std::nullopt;
      case data_type::UNKOWN: throw std::runtime_error{"unkown type"};
    }
  }
  if (auto const special_it = special.find(type_name);
      special_it != end(special)) {
    return special_it->second;
  }
  return std::nullopt;
}

void generate_header(std::ostream& out, schema const& s, type const& t) {
  out << "#pragma once\n";

  auto const uses_optional =
      std::any_of(begin(t.members_), end(t.members_),
                  [](member const& m) { return m.optional_; });
  auto const uses_vector =
      std::any_of(begin(t.members_), end(t.members_), [](member const& m) {
        return m.list_ && m.max_size_ == std::numeric_limits<unsigned>::max();
      });
  auto const uses_array =
      std::any_of(begin(t.members_), end(t.members_), [](member const& m) {
        return m.list_ && m.max_size_ != std::numeric_limits<unsigned>::max();
      });
  auto const uses_string =
      t.data_type_ == data_type::STRING ||
      std::any_of(begin(t.members_), end(t.members_),
                  [](member const& m) { return m.type_ == "STRING"; });
  auto const uses_variant = t.data_type_ == data_type::SELECT;

  if (t.subtype_of_.empty()) {
    out << "#include \"step/root_entity.h\"\n";
  } else {
    out << "#include \"" << s.name_ << "/" << t.subtype_of_ << ".h\"\n";
  }

  if (uses_array || uses_optional || uses_string || uses_variant ||
      uses_vector) {
    out << "\n";
  }
  out << (uses_array ? "#include <array>\n" : "")
      << (uses_optional ? "#include <optional>\n" : "")
      << (uses_string ? "#include <string>\n" : "")
      << (uses_variant ? "#include <variant>\n" : "")  //
      << "#include <vector>\n";
  if (uses_array || uses_optional || uses_string || uses_variant ||
      uses_vector) {
    out << "\n";
  }
  if (t.data_type_ == data_type::SELECT) {
    for (auto const& d : t.details_) {
      out << "#include \"" << s.name_ << "/" << d << ".h\"\n";
    }
  }
  if (t.data_type_ == data_type::ALIAS) {
    out << "#include \"" << s.name_ << "/" << t.alias_ << ".h\"\n";
  }
  if (t.data_type_ == data_type::ENTITY) {
    for (auto const& m : t.members_) {
      if (auto const it = s.type_map_.find(m.type_);
          it != end(s.type_map_) && it->second->data_type_ == data_type::ENUM) {
        out << "#include \"" << s.name_ << "/" << it->second->name_ << ".h\"\n";
      }
    }
  }

  if (t.data_type_ == data_type::ENTITY || t.data_type_ == data_type::ENUM) {
    out << "\nnamespace utl { struct cstr; }\n";
  }

  out << "\nnamespace " << s.name_ << " {\n\n";

  switch (t.data_type_) {
    case data_type::BOOL: [[fallthrough]];
    case data_type::LOGICAL: out << "using " << t.name_ << " = bool;\n"; break;

    case data_type::REAL: [[fallthrough]];
    case data_type::NUMBER: out << "using " << t.name_ << " = double;\n"; break;

    case data_type::INTEGER: out << "using " << t.name_ << " = int;\n"; break;

    case data_type::STRING:
      out << "using " << t.name_ << " = std::string;\n";
      break;

    case data_type::SELECT:
      out << "using " << t.name_ << " = std::variant<\n";
      for (auto const& [i, v] : utl::enumerate(t.details_)) {
        out << "  " << v << (i != t.details_.size() - 1 ? "," : "") << "\n";
      }
      out << ">;\n";
      break;

    case data_type::ALIAS:
      out << "using " << t.name_ << " = " << t.alias_ << ";\n";
      break;

    case data_type::ENUM:
      out << "enum class " << t.name_ << " {\n";
      for (auto const& [i, v] : utl::enumerate(t.details_)) {
        if (v == "NULL") {
          out << "  VNULL" << (i != t.details_.size() - 1 ? "," : "") << "\n";
        } else {
          out << "  " << v << (i != t.details_.size() - 1 ? "," : "") << "\n";
        }
      }
      out << "};\n";
      out << "void parse_step(utl::cstr&, " << t.name_ << "&);\n";
      break;

    case data_type::ENTITY: {
      for (auto const& m : t.members_) {
        if (auto const data_type = is_special(s, m.type_);
            !data_type.has_value()) {
          out << "struct " << m.type_ << ";\n";
        }
      }
      if (!t.members_.empty()) {
        out << "\n";
      }

      out << "struct " << t.name_;
      if (t.subtype_of_.empty()) {
        out << " : public step::root_entity";
      } else {
        out << " : public " << t.subtype_of_;
      }
      out << " {\n"
          << "  static constexpr auto const NAME = \""
          << boost::to_upper_copy<std::string>(t.name_) << "\";\n"
          << "  friend void parse_step(utl::cstr&, " << t.name_ << "&);\n"
          << "  virtual void resolve(std::vector<root_entity*> const&) "
             "override;\n";

      out << "  template <typename Fn>\n";
      out << "  void for_each_ref(Fn&& f) const {\n";
      if (!t.subtype_of_.empty()) {
        out << "    static_cast<" << t.subtype_of_
            << " const*>(this)->for_each_ref(std::forward<Fn>(f));\n";
      }
      for (auto const& m : t.members_) {
        if (auto const data_type = is_special(s, m.type_);
            !data_type.has_value()) {
          if (m.optional_) {
            out << "    if (" << m.name_ << "_.has_value()";
            if (!m.list_) {
              out << " && *" << m.name_ << "_ != nullptr";
            }
            out << ") {\n";
          }
          if (m.list_) {
            out << "      for (auto const& e : " << (m.optional_ ? "*" : "")
                << m.name_ << "_) {\n";
            out << "        if (e != nullptr) { f(*e); }\n";
            out << "      }\n";
          } else {
            out << "    if (" << (m.optional_ ? "*" : "") << m.name_
                << "_ != nullptr) {";
            out << "      f(" << (m.optional_ ? "**" : "*") << m.name_
                << "_);\n";
            out << "    }\n";
          }
          if (m.optional_) {
            out << "    }\n";
          }
        }
      }
      out << "  }\n\n";

      for (auto const& m : t.members_) {
        auto const use_array =
            m.max_size_ != std::numeric_limits<unsigned>::max();
        out << "  "  //
            << (m.optional_ ? "std::optional<" : "")
            << (m.list_ ? use_array ? "std::array<" : "std::vector<" : "");

        if (auto const data_type = is_special(s, m.type_);
            data_type.has_value()) {
          out << *data_type;
        } else {
          out << m.type_ << "*";
        }
        if (use_array) {
          out << ", " << m.max_size_;
        }
        out << (m.list_ ? ">" : "")  //
            << (m.optional_ ? ">" : "")  //
            << " " << m.name_ << "_;\n";
      }
      out << "};\n";
      break;

      case data_type::UNKOWN:
      default: throw std::runtime_error{"unknown data type"};
    }
  }

  out << "\n}  // namespace " << s.name_ << "\n";
}

void generate_source(std::ostream& out, schema const& s, type const& t) {
  switch (t.data_type_) {
    case data_type::ENTITY:
      out << "#include \"" << s.name_ << "/" << t.name_ << ".h\"\n\n"
          << "#include \"utl/parser/cstr.h\"\n\n"
          << "#include \"step/parse_step.h\"\n"
          << "#include \"step/resolve.h\"\n\n"
          << "namespace " << s.name_ << " {\n\n";
      out << "void parse_step(utl::cstr& s, " << t.name_ << "& e) {\n";
      out << "  using step::parse_step;\n";
      if (!t.subtype_of_.empty()) {
        out << "  parse_step(s, *static_cast<" << t.subtype_of_ << "*>(&e));\n";
      }
      for (auto const& m : t.members_) {
        out << "  parse_step(s, e." << m.name_ << "_);\n"
            << "  if (s.len > 0 && s[0] == ',') {\n"
            << "    ++s;\n"
            << "  }\n"
            << "  s = s.skip_whitespace_front();\n\n";
      }
      out << "}\n\n";
      out << "void " << t.name_
          << "::resolve(std::vector<root_entity*> const& m) {\n";
      if (!t.subtype_of_.empty()) {
        out << "  " << t.subtype_of_ << "::resolve(m);\n";
      }
      for (auto const& m : t.members_) {
        if (auto const data_type = is_special(s, m.type_);
            !data_type.has_value()) {
          out << "  step::resolve(m, " << m.name_ << "_);\n";
        }
      }
      out << "}\n";
      out << "\n}  // namespace " << s.name_ << "\n\n\n";

      break;

    case data_type::ENUM:
      out << "#include \"" << s.name_ << "/" << t.name_ << ".h\"\n\n";
      out << "#include \"utl/parser/cstr.h\"\n";
      out << "#include \"utl/verify.h\"\n\n";
      out << "#include \"cista/hash.h\"\n\n";
      out << "#include \"step/parse_step.h\"\n\n";
      out << "namespace " << s.name_ << " {\n\n";
      out << "void parse_step(utl::cstr& s, " << t.name_ << "& v) {\n";
      out << "  utl::verify(s.len != 0 && s[0] == '.', \"expected enum start "
             "'.', got {}\", s.view());\n";
      out << "  ++s;\n";
      out << "  auto const end = step::get_next_token(s, '.');\n";
      out << "  utl::verify(end.has_value(), \"enum end not found, got {}\", "
             "s.view());\n";
      out << "  auto const str = std::string_view{s.str, "
             "static_cast<unsigned>(end->str - s.str - 1)};\n";
      out << "  switch(cista::hash(str)) {\n";
      for (auto const& [i, m] : utl::enumerate(t.details_)) {
        out << "    case " << cista::hash(m) << "U: v = " << t.name_
            << "::" << (m == "NULL" ? "VNULL" : m) << "; break;\n";
      }
      out << "    default: utl::verify(false, \"expected enum value, got {}\", "
             "str);\n";
      out << "  }\n";
      out << "  s = *end;\n";
      out << "}\n\n";
      out << "}  // namespace " << s.name_ << "\n\n\n";
      break;

    default: /* skip */ break;
  }
}

}  // namespace express