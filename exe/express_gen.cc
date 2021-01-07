#include <iostream>

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

  source_out << "\n\n#include \"step/entry_parser.h\"\n";
  source_out << "namespace " << schema.name_ << "{\n";
  source_out << "void register_all_types(step::entry_parser& m) {";
  for (auto const& t : schema.types_) {
    if (t.data_type_ == express::data_type::ENTITY) {
      source_out << "  m.register_parser<" << t.name_ << ">();\n";
    }
  }
  source_out << "}\n";
  source_out << "}  // namespace " << schema.name_ << "\n";

  auto types_header_out = std::ofstream{
      (header_path / ("register_all_types.h")).generic_string().c_str()};
  types_header_out << "#pragma once\n\n"
                   << "namespace step { struct entry_parser; }\n\n";
  types_header_out << "namespace " << schema.name_ << " {\n\n"
                   << "void register_all_types(step::entry_parser&);\n";
  types_header_out << "}  // namespace " << schema.name_;
}