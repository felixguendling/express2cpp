#include "step/split_line.h"

#include "step/parse_step.h"

namespace step {

std::optional<line> split_line(utl::cstr in) {
  if (in.len == 0 || in[0] != '#') {
    return std::nullopt;
  }

  line output;
  parse_step(in, output.id_);

  in = in.skip_whitespace_front();
  utl::verify(in.len > 3 && in[0] == '=', "no '=' found, got {}", in.view());

  auto const params_end = in.view().find(");");
  utl::verify(params_end != std::string_view::npos,
              "end ');' not found, got {}", in.view());

  in = in.substr(1, utl::size{params_end - 1});
  in = in.skip_whitespace_front();

  auto const bracket_pos = in.view().find('(');
  utl::verify(bracket_pos != std::string_view::npos,
              "no bracket '(' found in: {}", in.view());

  output.name_ = in.substr(0, utl::size{bracket_pos});
  output.entity_ = in.substr(bracket_pos + 1);
  return output;
}

}  // namespace step