#include <iostream>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

#include "cista/mmap.h"

#include "express/exp_struct_gen.h"
#include "express/parse_exp.h"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "usage: " << argv[0] << " EXPRESS_FILE TARGET_DIR\n";
    return 1;
  }

  auto const root = fs::path{argv[2]};
  fs::create_directories(root);
  if (!fs::is_regular_file(argv[1])) {
    std::cout << argv[1] << " has to be a file\n";
    return 1;
  }

  auto const expr = cista::mmap{argv[1], cista::mmap::protection::READ};
  auto const schema = express::parse(std::string_view{
      reinterpret_cast<char const*>(expr.data()), expr.size()});

  auto const header_path = root / "include" / schema.name_;

  fs::create_directories(header_path);
  auto source_out = std::ofstream{
      (root / (root.stem().generic_string() + ".cc")).generic_string().c_str()};
  source_out << "#include \"step/id_t.h\"\n";
  for (auto const& t : schema.types_) {
    auto header_out = std::ofstream{
        (header_path / (t.name_ + ".h")).generic_string().c_str()};
    express::generate_header(header_out, schema, t);
    express::generate_source(source_out, schema, t);
  }

  source_out << "\n\n"
                "#include \""
             << schema.name_ << "/"
             << "parser.h\"\n"
                "#include \"step/root_entity.h\"\n"
                "#include \"step/parse_lines.h\"\n"
                "\n"
             << "namespace " << schema.name_ << "{\n"
             << "\n"
                "struct full_parser {\n"
                "  std::optional<step::entity_ptr> parse(\n"
                "      utl::cstr type_name,\n"
                "      utl::cstr rest) const {\n"
                "    switch (cista::hash(type_name.view())) {\n";
  for (auto const& t : schema.types_) {
    if (t.data_type_ == express::data_type::ENTITY) {
      source_out << "      case "
                 << cista::hash(boost::to_upper_copy<std::string>(t.name_))
                 << "U: { auto v = std::make_unique<" << t.name_
                 << ">(); parse_step(rest, *v); return "
                    "std::unique_ptr<step::root_entity>(v.release()); }\n";
    }
  }
  source_out
      << "      default: return std::nullopt;\n"
         "    }\n"
         "  }\n"
         "};\n"
         "\n"
         "step::model parse(step::selective_entity_parser& p, utl::cstr s) {\n"
         "  return step::parse_lines(p, s);\n"
         "}\n"
         "\n"
         "step::model parse(utl::cstr s) {\n"
         "  return step::parse_lines(full_parser{}, s);\n"
         "}\n"
         "\n"
         "}  // namespace "
      << schema.name_ << "\n";

  auto types_header_out =
      std::ofstream{(header_path / ("parser.h")).generic_string().c_str()};
  types_header_out
      << "#pragma once\n\n"
      << "#include \"step/model.h\"\n"
      << "#include \"step/selective_entity_parser.h\"\n\n"
      << "namespace " << schema.name_ << " {\n"
      << "\n"
      << "step::model parse(utl::cstr);\n"
         "\n"
      << "step::model parse(step::selective_entity_parser&, utl::cstr);\n"
         "\n"
      << "template <typename... Entities>\n"
         "step::model parse(utl::cstr s) {\n"
         "  step::selective_entity_parser p;\n"
         "  p.register_parsers<Entities...>();\n"
         "  return parse(p, s);\n"
         "}\n"
         "\n"
      << "}  // namespace " << schema.name_;
}