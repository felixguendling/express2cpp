#include "doctest/doctest.h"

#include "IFC2X3/IfcColourRgb.h"
#include "IFC2X3/IfcFlowController.h"
#include "IFC2X3/IfcProductRepresentation.h"
#include "IFC2X3/IfcRepresentation.h"
#include "IFC2X3/IfcSite.h"
#include "IFC2X3/IfcSurfaceStyle.h"
#include "IFC2X3/IfcSurfaceStyleRendering.h"
#include "IFC2X3/parser.h"

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
#96932 = IFCDIRECTION((1.000000,0.000000,0.000000));git s
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
  auto model = IFC2X3::parse(ifc_input);
  auto const& flow_ctrl =
      model.get_entity<IFC2X3::IfcFlowController>(step::id_t{96945});
  CHECK(flow_ctrl.GlobalId_ == "0Gkk91VZX968DF0GjbXoN4");
  REQUIRE(flow_ctrl.Representation_.has_value());
  auto const& repr = flow_ctrl.Representation_.value();
  REQUIRE(repr->Representations_.size() == 1);
  REQUIRE(repr->Representations_.at(0)->RepresentationType_.has_value());
  CHECK(repr->Representations_.at(0)->RepresentationType_.value() ==
        "MappedRepresentation");
}

TEST_CASE("parse id select") {
  constexpr auto const* const ifc_input =
      R"(#200158=IFCCOLOURRGB($,0.200000,0.200000,0.200000);
#200159=IFCSURFACESTYLERENDERING(#200158,$,$,$,$,$,$,$,.METAL.);
#200160=IFCSURFACESTYLE('Default Surface',.BOTH.,(#200159));
#200161=IFCPRESENTATIONSTYLEASSIGNMENT((#200160));)";

  auto model = IFC2X3::parse(ifc_input);
  auto const& surface_style = model.get_entity<IFC2X3::IfcSurfaceStyle>(200160);
  REQUIRE(surface_style.Styles_.at(0).data_.index() == 0U);

  auto* const shading = std::get<0>(surface_style.Styles_.at(0).data_);
  REQUIRE(shading->SurfaceColour_ != nullptr);
  CHECK(shading->SurfaceColour_->Red_ == 0.2);
  CHECK(shading->SurfaceColour_->Green_ == 0.2);
  CHECK(shading->SurfaceColour_->Blue_ == 0.2);
}

TEST_CASE("parse ifc site") {
  auto const constexpr input =
      "#23 = IFCSITE('2xNM1YvyH50w3CkBOfaqX1', #2, 'Default Site', "
      "'Description of Default Site', $, #24, $, $, .ELEMENT., (24, 28, 0), "
      "(54, 25, 0), $, $, $);";

  auto model = IFC2X3::parse(input);
  auto const& site = model.get_entity<IFC2X3::IfcSite>(step::id_t{23});
  CHECK(site.GlobalId_ == "2xNM1YvyH50w3CkBOfaqX1");
}