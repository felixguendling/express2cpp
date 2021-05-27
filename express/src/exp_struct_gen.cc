#include "express/exp_struct_gen.h"

#include <algorithm>
#include <map>
#include <optional>
#include <ostream>

#include "boost/algorithm/string.hpp"

#include "cista/hash.h"

#include "utl/enumerate.h"
#include "utl/verify.h"

namespace express {

static auto const special =
    std::map<std::string, std::string>{{"BOOLEAN", "bool"},
                                       {"LOGICAL", "bool"},
                                       {"REAL", "double"},
                                       {"INTEGER", "int"},
                                       {"STRING", "std::string"},
                                       {"BINARY(32)", "uint32_t"},
                                       {"BINARY", "std::vector<uint8_t>"}};

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
      case data_type::ENTITY: return std::nullopt;
      case data_type::SELECT: return type_name;
      case data_type::BINARY: return "std::vector<uint8_t>";
      case data_type::UNKOWN: throw std::runtime_error{"unkown type"};
    }
  }
  if (auto const special_it = special.find(type_name);
      special_it != end(special)) {
    return special_it->second;
  }
  return std::nullopt;
}

bool is_value_type(schema const& s, type const& t) {
  auto const it = s.type_map_.find(t.name_);
  utl::verify(it != end(s.type_map_), "type {} not found in type map", t.name_);
  if (it->second->list_) {
    return true;
  }
  switch (it->second->data_type_) {
    case data_type::ALIAS:
      utl::verify(t.alias_ != t.name_, "infinite alias loop for {}", t.name_);
      return is_value_type(s, *s.type_map_.at(t.alias_));

    case data_type::BOOL:
    case data_type::LOGICAL:
    case data_type::REAL:
    case data_type::NUMBER:
    case data_type::STRING:
    case data_type::INTEGER:
    case data_type::ENUM:
    case data_type::BINARY: [[fallthrough]];
    case data_type::SELECT: return true;

    case data_type::ENTITY: return false;

    case data_type::UNKOWN: [[fallthrough]];
    default: throw std::runtime_error{"unkown type"};
  }
}

void generate_header(std::ostream& out, schema const& s, type const& t) {
  out << "#pragma once\n\n";

  auto const uses_optional =
      std::any_of(begin(t.members_), end(t.members_),
                  [](member const& m) { return m.optional_; });
  auto const uses_list =
      std::any_of(begin(t.members_), end(t.members_),
                  [](member const& m) { return m.is_list(); });
  auto const uses_string =
      t.data_type_ == data_type::STRING ||
      std::any_of(begin(t.members_), end(t.members_), [&](member const& m) {
        auto const special = is_special(s, m.get_type_name());
        return special.has_value() && *special == "std::string";
      });
  auto const uses_variant = t.data_type_ == data_type::SELECT;

  if (t.subtype_of_.empty()) {
    out << "#include \"step/root_entity.h\"\n";
  } else {
    out << "#include \"" << s.name_ << "/" << t.subtype_of_ << ".h\"\n";
  }

  if (uses_list || uses_optional || uses_string || uses_variant) {
    out << "\n";
  }
  out << "#include <iosfwd>\n"
      << (uses_list ? "#include <vector>\n" : "")
      << (uses_optional ? "#include <optional>\n" : "")
      << (uses_string ? "#include <string>\n" : "")
      << (uses_variant ? "#include <variant>\n" : "")  //
      << "#include <vector>\n";
  if (uses_list || uses_optional || uses_string || uses_variant) {
    out << "\n";
  }
  if (t.data_type_ == data_type::SELECT) {
    out << "#include \"step/id_t.h\"\n\n";
    for (auto const& d : t.details_) {
      if (is_value_type(s, *s.type_map_.at(d))) {
        out << "#include \"" << s.name_ << "/" << d << ".h\"\n";
      }
    }
  }
  if (t.data_type_ == data_type::ALIAS) {
    out << "#include \"" << s.name_ << "/" << t.alias_ << ".h\"\n";
  }
  if (t.data_type_ == data_type::ENTITY) {
    for (auto const& m : t.members_) {
      if (auto const it = s.type_map_.find(m.get_type_name());
          it != end(s.type_map_) &&
          (it->second->data_type_ == data_type::ENUM ||
           it->second->data_type_ == data_type::SELECT)) {
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

    case data_type::BINARY:
      out << "using " << t.name_ << " = std::vector<uint8_t>;\n";
      break;

    case data_type::SELECT:
      for (auto const& m : t.details_) {
        if (!is_value_type(s, *s.type_map_.at(m))) {
          out << "struct " << m << ";\n";
        }
      }
      out << "\nstruct " << t.name_ << " {\n";
      out << "  friend void parse_step(utl::cstr&, " << t.name_ << "&);\n";
      out << "  friend void write(step::write_context const&, std::ostream&, "
          << t.name_ << " const&);\n";
      out << "  std::string_view name() const;\n";
      out << "  void resolve(std::vector<step::root_entity*> const&);\n\n";
      out << "  std::variant<\n";
      for (auto const& [i, v] : utl::enumerate(t.details_)) {
        out << "    " << v << (!is_value_type(s, *s.type_map_.at(v)) ? "*" : "")
            << (i != t.details_.size() - 1 ? "," : "") << "\n";
      }
      out << "  > data_;\n";
      out << "  step::id_t tmp_id_{std::numeric_limits<unsigned>::max()};\n";
      out << "};";
      break;

    case data_type::ALIAS:
      out << "using " << t.name_ << " = ";
      if (t.list_) {
        out << "std::vector<";
      }
      out << t.alias_
          << (!is_value_type(s, *s.type_map_.at(t.alias_)) ? "*" : "");
      if (t.list_) {
        out << ">";
      }
      out << ";\n";
      break;

    case data_type::ENUM:
      out << "enum class " << t.name_ << " {\n";
      for (auto const& [i, v] : utl::enumerate(t.details_)) {
        out << "  " << s.name_ << "_" << v
            << (i != t.details_.size() - 1 ? "," : "") << "\n";
      }
      out << "};\n";
      out << "void parse_step(utl::cstr&, " << t.name_ << "&);\n";
      out << "void write(step::write_context const&, std::ostream&, " << t.name_
          << " const&);\n";
      break;

    case data_type::ENTITY: {
      for (auto const& m : t.members_) {
        if (auto const data_type = is_special(s, m.get_type_name());
            !data_type.has_value()) {
          out << "struct " << m.get_type_name() << ";\n";
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
          << "  std::string_view name() const override { return NAME; }\n"
          << "  friend void parse_step(utl::cstr&, " << t.name_ << "&);\n"
          << "  void resolve(std::vector<root_entity*> const&) override;\n"
             "  void write(step::write_context const&, std::ostream&, bool "
             "const write_type_name) const "
             "override;\n";

      for (auto const& m : t.members_) {
        auto const type_it = s.type_map_.find(m.get_type_name());
        auto const is_list = m.is_list() || (type_it != end(s.type_map_) &&
                                             type_it->second->list_);

        struct visit {
          visit(schema const& s, std::ostream& o) : s_{s}, out_{o} {}
          void operator()(express::type_name const& t) const {
            if (auto const data_type = is_special(s_, t.name_);
                data_type.has_value()) {
              out_ << *data_type;
            } else {
              out_ << t.name_ << "*";
            }
          }
          void operator()(express::list const& l) const {
            out_ << "std::vector<";
            boost::apply_visitor(*this, l.m_);
            out_ << ">";
          }
          schema const& s_;
          std::ostream& out_;
        };

        out << "  "  //
            << (m.optional_ ? "std::optional<" : "");

        boost::apply_visitor(visit{s, out}, m.type_);

        auto const data_type = is_special(s, m.get_type_name());
        data_type.has_value();
        out << (m.optional_ ? ">" : "")  //
            << " " << m.name_ << "_"
            << (!data_type.has_value() && !is_list && !m.optional_ ? "{nullptr}"
                                                                   : "")
            << ";\n";
      }
      out << "};\n";
      break;

      case data_type::UNKOWN:
      default: throw std::runtime_error{"unknown data type"};
    }
  }

  out << "\n}  // namespace " << s.name_ << "\n";
}

void list_select_cases(std::ostream& out, schema const& s,
                       std::vector<std::pair<type const*, std::size_t>> chain) {
  if (chain.back().first->data_type_ != data_type::SELECT) {
    // Recursion base case -> output case
    auto const [last_type, last_id] = chain.back();
    chain.resize(chain.size() - 1);
    out << "    case "
        << cista::hash(boost::to_upper_copy<std::string>(last_type->name_))
        << "U"
        << ": { " << s.name_ << "::" << last_type->name_
        << " v; parse_step(s, v); e = ";
    for (auto const& [i, entry] : utl::enumerate(chain)) {
      auto const& [el_t, select_index] = entry;
      out << s.name_ << "::" << el_t->name_ << "{"
          << "decltype(std::declval<" << s.name_ << "::" << el_t->name_
          << ">().data_){std::in_place_index_t<" << select_index << ">{}, ";
    }
    out << "v";
    for (auto const& [i, c] : utl::enumerate(chain)) {
      out << "}}";
    }
    out << ";";
    out << " break; }\n";
  } else {
    // Continue recursion for each type in the select.
    for (auto const& [i, m] : utl::enumerate(chain.back().first->details_)) {
      auto const m_type = *s.type_map_.at(m);
      if (!is_value_type(s, m_type)) {
        continue;  // handled by ID case
      }
      auto next_chain = chain;
      next_chain.back().second = i;
      next_chain.emplace_back(&m_type, std::numeric_limits<std::size_t>::max());
      list_select_cases(out, s, std::move(next_chain));
    }
  }
}

bool has_members(schema const& s, type const& t) {
  return !t.members_.empty() ||
         (!t.subtype_of_.empty() &&
          has_members(s, *s.type_map_.at(t.subtype_of_)));
};

void generate_source(std::ostream& out, schema const& s, type const& t) {
  switch (t.data_type_) {
    case data_type::SELECT: {
      out << "#include \"" << s.name_ << "/" << t.name_ << ".h\"\n\n"
          << "#include \"utl/parser/cstr.h\"\n"
          << "#include \"utl/verify.h\"\n\n"
          << "#include \"step/parse_step.h\"\n"
          << "#include \"step/write.h\"\n"
          << "#include \"step/assign_entity_ptr_to_select.h\"\n"
          << "#include \"step/resolve.h\"\n\n"
          << "namespace " << s.name_ << " {\n\n";
      out << "void parse_step(utl::cstr& s, " << t.name_ << "& e) {\n";
      out << "  using step::parse_step;\n";
      out << "  if (s.len != 0 && s[0] == '#') {\n";
      out << "    parse_step(s, e.tmp_id_);\n";
      out << "    return;\n";
      out << "  }\n";

      auto const select_has_value_types = std::any_of(
          begin(t.details_), end(t.details_), [&](std::string const& m) {
            return is_value_type(s, *s.type_map_.at(m));
          });
      if (select_has_value_types) {
        out << "  auto const name_end = step::get_next_token(s, '(');\n";
        out << R"(  utl::verify(name_end.has_value(), "expected SELECT name, got {}", s.view());)"
            << "\n";
        out << R"(  auto const name = std::string_view{s.str, static_cast<std::size_t>(name_end->str - s.str - 1)};)"
            << "\n";
        out << "  s = *name_end;\n";
        out << "  switch(cista::hash(name)) {\n";
        list_select_cases(out, s,
                          {{&t, std::numeric_limits<std::size_t>::max()}});
        out << "    default: utl::verify(false, \"unable to parse select "
            << s.name_ << "::" << t.name_
            << ", got name '{}', hash={}\", name, cista::hash(name));\n";
        out << "  }\n";
        out << R"(  utl::verify(s.len != 0 && s[0] == ')', "expected select end ')', got {}", s.view());)"
            << "\n";
        out << "  ++s;\n";
      } else {
        out << "  utl::verify(false, \"select: expected id, got {}\", "
               "s.view());\n";
      }

      out << "}\n\n";
      out << "void " << t.name_
          << "::resolve(std::vector<step::root_entity*> const& m) {\n";
      out << "  if (tmp_id_ == step::id_t::invalid()) { return; }\n";
      out << "  step::assign_entity_ptr_to_select(*this, m.at(tmp_id_.id_));\n";
      out << "}\n\n";

      out << "std::string_view " << t.name_ << "::name() const {\n";
      out << "  static char const* names[] = {\n";
      for (auto const& [i, m] : utl::enumerate(t.details_)) {
        out << "    \"" << boost::to_upper_copy<std::string>(m) << "\"";
        if (i != t.details_.size() - 1) {
          out << ",\n";
        } else {
          out << "\n";
        }
      }
      out << "  };\n";
      out << "  return names[data_.index()];\n"
             "}\n\n";

      out << "void write(step::write_context const& ctx, std::ostream& out, "
          << t.name_ << " const& el) {\n";
      out << "  std::visit([&](auto&& data) {\n"
             "    using step::write;\n"
             "    using Type = std::decay_t<decltype(data)>\n;"
             "    constexpr auto const final = "
             "!step::has_data<Type>::value && !std::is_pointer_v<Type>;\n"
             "    if constexpr (final) {\n"
             "      out << el.name() << '(';\n"
             "    }\n"
             "    write(ctx, out, data);\n"
             "    if constexpr (final) {\n"
             "      out << ')';\n"
             "    }\n"
             "  }, el.data_);";
      out << "}\n";
      out << "\n}  // namespace " << s.name_ << "\n\n\n";
    } break;

    case data_type::ENTITY:
      out << "#include \"" << s.name_ << "/" << t.name_ << ".h\"\n\n"
          << "#include \"utl/parser/cstr.h\"\n\n"
          << "#include \"step/parse_step.h\"\n"
          << "#include \"step/resolve.h\"\n"
          << "#include \"step/write.h\"\n\n"
          << "namespace " << s.name_ << " {\n\n";
      out << "void parse_step(utl::cstr& s, " << t.name_ << "& e) {\n";
      out << "  using step::parse_step;\n";
      if (!t.subtype_of_.empty()) {
        out << "  parse_step(s, *static_cast<" << t.subtype_of_ << "*>(&e));\n";
      }
      for (auto const& m : t.members_) {
        out << "  if (s.len > 0 && s[0] == '*') {\n"
            << "    ++s;\n"
            << "  } else {\n"
            << "    parse_step(s, e." << m.name_ << "_);\n"
            << "  }\n"
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
        if (auto const member_type_it = s.type_map_.find(m.get_type_name());
            member_type_it != end(s.type_map_) &&
            (member_type_it->second->data_type_ == data_type::ENTITY ||
             member_type_it->second->data_type_ == data_type::SELECT)) {
          out << "  step::resolve(m, " << m.name_ << "_);\n";
        }
      }
      out << "}\n";

      out << "void " << t.name_
          << "::write(step::write_context const& ctx, std::ostream& out, bool "
             "const write_type_name) const "
             "{\n"
             "  using step::write;\n"
             "  if (write_type_name) { out << \""
          << boost::to_upper_copy<std::string>(t.name_) << "(\"; }\n";
      if (!t.subtype_of_.empty()) {
        out << "  " << t.subtype_of_ << "::write(ctx, out, false);\n";
        if (!t.members_.empty() &&
            has_members(s, *s.type_map_.at(t.subtype_of_))) {
          out << "  out << \", \";\n";
        }
      }

      if (!t.members_.empty()) {
        for (auto const& [i, m] : utl::enumerate(t.members_)) {
          out << "  write(ctx, out, " << m.name_ << "_);\n";
          if (i != t.members_.size() - 1) {
            out << "  out << \", \";\n";
          }
        }
      }
      out << "  if (write_type_name) { out << \");\"; }\n";
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
            << "::" << s.name_ << "_" << m << "; break;\n";
      }
      out << "    default: utl::verify(false, \"expected enum value, got {}\", "
             "str);\n";
      out << "  }\n";
      out << "  s = *end;\n";
      out << "}\n\n";
      out << "void write(step::write_context const&, std::ostream& out, "
          << t.name_ << " const& val) {\n"
          << "  switch (val) {\n";
      for (auto const& [i, m] : utl::enumerate(t.details_)) {
        out << "    case " << t.name_ << "::" << s.name_ << "_" << m
            << ": out << \"." << m << ".\"; break;\n";
      }
      out << "    default: throw std::runtime_error{\"unknown enum value\"};\n";
      out << "  }\n";
      out << "}\n\n";
      out << "}  // namespace " << s.name_ << "\n\n\n";
      break;

    default: /* skip */ break;
  }
}

}  // namespace express
