#include "doctest/doctest.h"

#include <iostream>
#include <sstream>

#include "step/write.h"

#include "IFC2X3/IfcActionSourceTypeEnum.h"
#include "IFC2X3/IfcCartesianPoint.h"
#include "IFC2X3/IfcSurfaceStyle.h"
#include "IFC2X3/parser.h"

TEST_CASE("write enum test") {
  std::stringstream ss;
  write(step::write_context{}, ss,
        IFC2X3::IfcActionSourceTypeEnum::IFC2X3_DEAD_LOAD_G);
  CHECK(ss.str() == ".DEAD_LOAD_G.");
}

TEST_CASE("write entity test") {
  std::stringstream ss;
  IFC2X3::IfcCartesianPoint p;
  p.Coordinates_ = {1, 2, 3};
  write(step::write_context{}, ss, p);
  CHECK(ss.str() == "IFCCARTESIANPOINT((1, 2, 3));");
}

TEST_CASE("write model with references test") {
  constexpr auto const* const ifc_input =
      R"(#200158=IFCCOLOURRGB($,0.200000,0.200000,0.200000);
#200159=IFCSURFACESTYLERENDERING(#200158,$,$,$,$,$,$,$,.METAL.);
#200160=IFCSURFACESTYLE('Default Surface',.BOTH.,(#200159));)";

  std::stringstream ss;
  write(ss, IFC2X3::parse(ifc_input));
  auto const matches = ss.str() == R"(#0 = IFCCOLOURRGB($, 0.2, 0.2, 0.2);
#1 = IFCSURFACESTYLERENDERING(#0, $, $, $, $, $, $, $, .METAL.);
#2 = IFCSURFACESTYLE('Default Surface', .BOTH., (#1));
)";
  CHECK(matches);
}

TEST_CASE("write model with select value test") {
  constexpr auto const* const ifc_input =
      R"(#564425=IFCPROPERTYSINGLEVALUE('MaterialThickness','',IFCPOSITIVELENGTHMEASURE(86.),$);)";

  std::stringstream ss;
  write(ss, IFC2X3::parse(ifc_input));
  auto const matches = ss.str() ==
                       "#0 = IFCPROPERTYSINGLEVALUE('MaterialThickness', '', "
                       "IFCPOSITIVELENGTHMEASURE(86), $);\n";
  CHECK(matches);
}