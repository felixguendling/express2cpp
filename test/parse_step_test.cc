#include <iostream>

#include "doctest/doctest.h"

#include "step/entity_map.h"
#include "step/parse_step.h"

TEST_CASE("parse basic data types") {
  using step::parse_step;
  SUBCASE("int") {
    SUBCASE("before comma") {
      int i;
      auto s = utl::cstr{"123,"};
      parse_step(s, i);
      CHECK(i == 123);
      CHECK(s.view() == ",");
    }
    SUBCASE("before bracket") {
      int i;
      auto s = utl::cstr{"123)"};
      parse_step(s, i);
      CHECK(i == 123);
      CHECK(s.view() == ")");
    }
  }
  SUBCASE("double") {
    SUBCASE("before comma") {
      double d;
      auto s = utl::cstr{"123,"};
      parse_step(s, d);
      CHECK(d == 123);
      CHECK(s.view() == ",");
    }
    SUBCASE("before bracket") {
      double d;
      auto s = utl::cstr{"123)"};
      parse_step(s, d);
      CHECK(d == 123);
      CHECK(s.view() == ")");
    }
    SUBCASE("E notation") {
      double d;
      auto s = utl::cstr{"123E2)"};
      parse_step(s, d);
      CHECK(d == 12300);
      CHECK(s.view() == ")");
    }
    SUBCASE("full test") {
      double d;
      auto s = utl::cstr{"0.12E1)"};
      parse_step(s, d);
      CHECK(d == 1.2);
      CHECK(s.view() == ")");
    }
  }
  SUBCASE("bool") {
    SUBCASE("true") {
      bool b{false};
      auto s = utl::cstr{".T."};
      parse_step(s, b);
      CHECK(b == true);
    }
    SUBCASE("false") {
      bool b{true};
      auto s = utl::cstr{".F."};
      parse_step(s, b);
      CHECK(b == false);
    }
    SUBCASE("bad bool") {
      SUBCASE("not T or F") {
        bool b{true};
        auto s = utl::cstr{".A."};
        CHECK_THROWS(parse_step(s, b));
      }
      SUBCASE("missing trailing point") {
        bool b{true};
        auto s = utl::cstr{".A"};
        CHECK_THROWS(parse_step(s, b));
      }
      SUBCASE("missing leading point") {
        bool b{true};
        auto s = utl::cstr{"T."};
        CHECK_THROWS(parse_step(s, b));
      }
    }
  }
  SUBCASE("ptr") {
    void* ptr;
    auto s = utl::cstr{"#123"};
    parse_step(s, ptr);
    CHECK(reinterpret_cast<uintptr_t>(ptr) == 123);
  }
  SUBCASE("optional") {
    SUBCASE("ptr") {
      SUBCASE("no value") {
        std::optional<void*> ptr;
        auto s = utl::cstr{"$"};
        parse_step(s, ptr);
        CHECK(!ptr.has_value());
        CHECK(s.len == 0);
      }
      SUBCASE("value") {
        std::optional<void*> ptr;
        auto s = utl::cstr{"#123"};
        parse_step(s, ptr);
        REQUIRE(ptr.has_value());
        CHECK(reinterpret_cast<uintptr_t>(*ptr) == 123);
      }
    }
    SUBCASE("int") {
      SUBCASE("no value") {
        std::optional<int> i;
        auto s = utl::cstr{"$,"};
        parse_step(s, i);
        CHECK(s.view() == ",");
        CHECK(!i.has_value());
      }
      SUBCASE("value") {
        std::optional<int> i;
        auto s = utl::cstr{"123,"};
        parse_step(s, i);
        REQUIRE(i.has_value());
        CHECK(*i == 123);
        CHECK(s.view() == ",");
      }
    }
  }
}

#include "IFC2X3/IfcFlowController.h"
#include "IFC2X3/IfcProductRepresentation.h"
#include "IFC2X3/IfcRepresentation.h"
#include "IFC2X3/register_all_types.h"

std::string ifc_str(std::string const& guid) {
  return "#96945 = IFCFLOWCONTROLLER('" + guid +
         R"(', #2, 'linDbNetComponent', $, $, #96946, #96951, 'Tag:-281736096');
#96951 = IFCPRODUCTDEFINITIONSHAPE($, $, (#96944));
#96944 = IFCSHAPEREPRESENTATION(#20, 'Body', 'MappedRepresentation', (#96933));
#96933 = IFCMAPPEDITEM(#96927,#96934);
#96935 = IFCDIRECTION((0.000002,1.000000,-0.000000));
#96936 = IFCDIRECTION((1.000000,-0.000002,0.000000));
#96937 = IFCDIRECTION((0.000000,-0.000000,-1.000000));
#96938 = IFCCARTESIANPOINT((-55853.364335,57958.087044,46051.925317));
#96934 = IFCCARTESIANTRANSFORMATIONOPERATOR3D(#96935,#96936,#96938,1.000000,#96937);
#96927 = IFCREPRESENTATIONMAP(#96929,#96928);
#96930 = IFCCARTESIANPOINT((0.000000,0.000000,0.000000));
#96931 = IFCDIRECTION((0.000000,0.000000,1.000000));
#96932 = IFCDIRECTION((1.000000,0.000000,0.000000));
#96929 = IFCAXIS2PLACEMENT3D(#96930, #96931, #96932);
#96928 = IFCSHAPEREPRESENTATION(#20, 'Body', 'MappedRepresentation', (#92397,#94162,#96919));
#96919 = IFCMAPPEDITEM(#96913,#96920);
#96913 = IFCREPRESENTATIONMAP(#96915,#96914);
#96914 = IFCSHAPEREPRESENTATION(#20, 'Body', 'Brep', (#94559));
#94559 = IFCFACETEDBREP(#94560);
#94560 = IFCCLOSEDSHELL((#94561));
#94561 = IFCFACE((#94562));
#94562 = IFCFACEOUTERBOUND(#94563, .T.);
#94563 = IFCPOLYLOOP((#94165, #94166, #94167));
#94165 = IFCCARTESIANPOINT((8.793929,21.230422,105.000056));
#94166 = IFCCARTESIANPOINT((5.005869,26.779034,104.999912));
#94167 = IFCCARTESIANPOINT((-0.000000,22.979643,105.000056));
)";
}

TEST_CASE("parse ifc") {
  auto const ifc_input = ifc_str("0Gkk91VZX968DF0GjbXoN4");

  auto parser = step::entry_parser{};
  IFC2X3::register_all_types(parser);
  auto model = step::entity_map{parser, ifc_input};
  auto const flow_ctrl =
      model.get_entity<IFC2X3::IfcFlowController>(step::id_t{96945});
  CHECK(flow_ctrl.GlobalId_ == "0Gkk91VZX968DF0GjbXoN4");
  REQUIRE(flow_ctrl.Representation_.has_value());
  auto const& repr = flow_ctrl.Representation_.value();
  REQUIRE(repr->Representations_.size() == 1);
  REQUIRE(repr->Representations_.at(0)->RepresentationType_.has_value());
  CHECK(repr->Representations_.at(0)->RepresentationType_.value() ==
        "MappedRepresentation");
}