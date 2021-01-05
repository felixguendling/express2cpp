#include "express/parse_exp.h"

#include "boost/foreach.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "boost/spirit/include/phoenix_core.hpp"
#include "boost/spirit/include/phoenix_fusion.hpp"
#include "boost/spirit/include/phoenix_object.hpp"
#include "boost/spirit/include/phoenix_operator.hpp"
#include "boost/spirit/include/phoenix_stl.hpp"
#include "boost/spirit/include/qi.hpp"
#include "boost/variant/recursive_variant.hpp"

#include "utl/verify.h"

namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(
    express::type,
    (std::string, name_)
    (express::data_type, data_type_)
    (std::vector<std::string>, details_)
    (std::string, subtype_of_)
    (std::vector<express::member>, members_)
    (bool, list_)
    (unsigned, min_size_)
    (unsigned, max_size_)
    (std::string, alias_)
);
BOOST_FUSION_ADAPT_STRUCT(
    express::schema,
    (std::string, name_)
    (std::vector<express::type>, types_)
);
BOOST_FUSION_ADAPT_STRUCT(
    express::member,
    (std::string, name_)
    (std::string, type_)
    (bool, optional_)
    (bool, list_)
    (unsigned, min_size_)
    (unsigned, max_size_)
)
// clang-format on

namespace express {

template <typename Iterator>
struct express_grammar : qi::grammar<Iterator, schema(), ascii::space_type> {
  express_grammar() : express_grammar::base_type{express_} {
    using phoenix::at_c;
    using phoenix::push_back;
    using qi::as_string;
    using qi::bool_;
    using qi::char_;
    using qi::int_;
    using qi::lexeme;
    using qi::matches;
    using qi::space;
    using qi::string;
    using qi::symbols;
    using namespace qi::labels;

    // clang-format off
    data_type_.add
      ("BOOL", data_type::BOOL)
      ("LOGICAL", data_type::LOGICAL)
      ("REAL", data_type::REAL)
      ("NUMBER", data_type::NUMBER)
      ("STRING", data_type::STRING)
      ("INTEGER", data_type::INTEGER)
      ("ENTITY", data_type::ENTITY)
      ("ENUM", data_type::ENUM)
      ("SELECT", data_type::SELECT);
    // clang-format on

    enum_ = "TYPE "  //
            >>
            as_string[lexeme[+(char_ - '=' - space)]]
                     [at_c<0>(_val) = _1, at_c<1>(_val) = data_type::ENUM]  //
            >> '=' >> "ENUMERATION" >> "OF" >> '(' >>
            (as_string[*(char_ - char_(",)") - space)] %
             (*space >> ','))[at_c<2>(_val) = _1] >>
            *(char_ - "END_TYPE;")  //
            >> "END_TYPE;";

    select_ =
        "TYPE "  //
        >> as_string[lexeme[+(char_ - '=' - space)]]
                    [at_c<0>(_val) = _1, at_c<1>(_val) = data_type::SELECT]  //
        >> "= SELECT"  //
        >> '(' >> (as_string[lexeme[*(char_ - char_(",)") - space)]] %
                   ',')[at_c<2>(_val) = _1]  //
        >> *(char_ - "END_TYPE;")  //
        >> "END_TYPE;";

    type_ = "TYPE "  //
            >> as_string[lexeme[+(char_ - '=' - space)]][at_c<0>(_val) = _1] >>
            '='  //
            >> qi::matches[(string("LIST") | "ARRAY") >> '[' >>
                           int_[at_c<6>(_val) = _1] >> ':' >>
                           int_[at_c<7>(_val) = _1] >> ']' >> "OF"]
                          [at_c<5>(_val) = _1] >>
            (data_type_[at_c<1>(_val) = _1] |
             as_string[lexeme[+(char_ - ';' - space)]]
                      [at_c<8>(_val) = _1,
                       at_c<1>(_val) = data_type::ALIAS])  // data type
            >> *(char_ - "END_TYPE;")  //
            >> "END_TYPE;";

    member_ = as_string[lexeme[+(char_ - space)]][at_c<0>(_val) = _1]  //
              >> ':' >> qi::matches["OPTIONAL"][at_c<2>(_val) = _1]  //
              >> qi::matches[(string("LIST") | string("SET")) >> '[' >>
                             (int_[at_c<4>(_val) = _1] | '?') >> ':' >>
                             (int_[at_c<5>(_val) = _1] | '?') >> ']' >> "OF" >>
                             -string("UNIQUE")][at_c<3>(_val) = _1] >>
              as_string[lexeme[+(char_ - ';')]][at_c<1>(_val) = _1]  //
              >> ';';

    entity_ =
        "ENTITY"  //
        >> as_string[lexeme[*(char_ - space - ';')]]
                    [at_c<0>(_val) = _1,
                     at_c<1>(_val) = data_type::ENTITY]  // type name
        >> -(-string("ABSTRACT") >> "SUPERTYPE" >> "OF" >> '(' >>
             ((string("ONEOF") >> '(' >> *(char_ - ')') >> ')') |
              *(char_ - ')')) >>
             ')')  //
        >> -("SUBTYPE OF (" >>
             as_string[lexeme[+(char_ - ')')]][at_c<3>(_val) = _1] >> ")")  //
        >> ';'  //
        >> *(member_[push_back(at_c<4>(_val), _1)])  //
        >> -((string("INVERSE") | "WHERE" | "UNIQUE" | "DERIVE") >>
             *(char_ - "END_ENTITY;"))  //
        >> "END_ENTITY;";

    schema_ = "SCHEMA "  //
              >> as_string[lexeme[+(char_ - ';')]][at_c<0>(_val) = _1] >>
              ';'  //
              >> *(enum_[push_back(at_c<1>(_val), _1)] |
                   entity_[push_back(at_c<1>(_val), _1)] |
                   select_[push_back(at_c<1>(_val), _1)] |
                   type_[push_back(at_c<1>(_val), _1)])  //
              >> *(char_ - "END_SCHEMA") >> "END_SCHEMA";

    comment_ = "(*" >> *(char_ - "*)") >> "*)";

    express_ = *comment_ >> schema_[_val = _1];
  }

  qi::symbols<char, data_type> data_type_;
  qi::rule<Iterator, member(), ascii::space_type> member_;
  qi::rule<Iterator, type(), ascii::space_type> enum_;
  qi::rule<Iterator, type(), ascii::space_type> select_;
  qi::rule<Iterator, type(), ascii::space_type> type_;
  qi::rule<Iterator, type(), ascii::space_type> entity_;
  qi::rule<Iterator, schema(), ascii::space_type> schema_;
  qi::rule<Iterator, ascii::space_type> comment_;
  qi::rule<Iterator, schema(), ascii::space_type> express_;
};

schema parse(std::string_view s) {
  using boost::spirit::ascii::space;

  schema schema;
  auto iter = &s[0];  // NOLINT
  auto end = &s[s.size() - 1] + 1;  // NOLINT
  auto comment = express_grammar<decltype(iter)>{};
  auto const is_success = phrase_parse(iter, end, comment, space, schema);
  utl::verify_ex(is_success, parser_exception{iter, end});
  for (auto const& t : schema.types_) {
    schema.type_map_[t.name_] = &t;
  }
  return schema;
}

}  // namespace express