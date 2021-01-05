#include "doctest/doctest.h"

#include "step/entry_parser.h"

#include "IFC2X3/IfcAxis2Placement3D.h"
#include "IFC2X3/IfcBuildingElementProxy.h"
#include "IFC2X3/IfcCartesianPoint.h"
#include "IFC2X3/IfcDirection.h"
#include "IFC2X3/IfcOwnerHistory.h"
#include "IFC2X3/IfcShapeRepresentation.h"

TEST_CASE("parse product") {
  using building_element_proxy = IFC2X3::IfcBuildingElementProxy;

  constexpr auto const input =
      "#410 = IFCBUILDINGELEMENTPROXY('2K5zlWhbnD_Pplf7Wq7h2T', #2, "
      "'Platzhalter:88209840', $, $, #411, #416, 'Tag:88209840', $);";

  step::entry_parser p;
  p.register_parsers<building_element_proxy>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 410);
  REQUIRE(dynamic_cast<building_element_proxy*>(entry->second.get()));

  auto const& bep = *dynamic_cast<building_element_proxy*>(entry->second.get());
  CHECK(bep.GlobalId_ == "2K5zlWhbnD_Pplf7Wq7h2T");
  CHECK(reinterpret_cast<uintptr_t>(bep.OwnerHistory_) == 2);

  CHECK(!bep.Description_.has_value());
  CHECK(!bep.ObjectType_.has_value());

  CHECK(bep.ObjectPlacement_.has_value());
  CHECK(reinterpret_cast<uintptr_t>(*bep.ObjectPlacement_) == 411);

  CHECK(bep.Representation_.has_value());
  CHECK(reinterpret_cast<uintptr_t>(*bep.Representation_) == 416);

  REQUIRE(bep.Name_.has_value());
  CHECK(*bep.Name_ == "Platzhalter:88209840");

  REQUIRE(bep.Tag_.has_value());
  CHECK(bep.Tag_ == "Tag:88209840");

  CHECK(!bep.CompositionType_.has_value());

  bep.for_each_ref([](auto&& ref) {
    auto const id_ok = reinterpret_cast<uintptr_t>(&ref) == 2U ||
                       reinterpret_cast<uintptr_t>(&ref) == 411U ||
                       reinterpret_cast<uintptr_t>(&ref) == 416U;
    CHECK(id_ok);
  });
}

TEST_CASE("parse share representation") {
  using shape_representation = IFC2X3::IfcShapeRepresentation;

  constexpr auto const input =
      "#96944 = IFCSHAPEREPRESENTATION(#20, 'Body', 'MappedRepresentation', "
      "(#96933));";

  step::entry_parser p;
  p.register_parser<shape_representation>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 96944);
  REQUIRE(dynamic_cast<shape_representation*>(entry->second.get()));

  auto const& bep = *dynamic_cast<shape_representation*>(entry->second.get());
  REQUIRE(bep.RepresentationType_.has_value());
  CHECK(*bep.RepresentationType_ == "MappedRepresentation");
  CHECK(bep.Items_.size() == 1);
  CHECK(reinterpret_cast<uintptr_t>(bep.Items_[0]) == 96933U);

  bep.for_each_ref([](auto&& ref) {
    if constexpr (std::is_same_v<std::decay_t<decltype(ref)>,
                                 IFC2X3::IfcRepresentationItem>) {
      auto const id_ok = reinterpret_cast<uintptr_t>(&ref) == 96933U;
      CHECK(id_ok);
    }
  });
}

TEST_CASE("parse cartesian point 1") {
  using vertex = IFC2X3::IfcCartesianPoint;

  constexpr auto const input =
      "#5466 = IFCCARTESIANPOINT((-73910.476024,65619.415293,49080.450753));";

  step::entry_parser p;
  p.register_parsers<vertex>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 5466);
  REQUIRE(nullptr != dynamic_cast<vertex*>(entry->second.get()));
  auto const& coords = dynamic_cast<vertex*>(entry->second.get())->Coordinates_;
  CHECK(std::abs(coords[0] - -73910.476024) <= 0.000001);
  CHECK(std::abs(coords[1] - 65619.415293) <= 0.000001);
  CHECK(std::abs(coords[2] - 49080.450753) <= 0.000001);
}

TEST_CASE("parse cartesian point 2") {
  using vertex = IFC2X3::IfcCartesianPoint;

  constexpr auto const input =
      "#16783 = IFCCARTESIANPOINT((-29750.345510,68710.165565,53116.953431));";

  step::entry_parser p;
  p.register_parsers<vertex>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 16783);
  REQUIRE(nullptr != dynamic_cast<vertex*>(entry->second.get()));
  auto const& coords = dynamic_cast<vertex*>(entry->second.get())->Coordinates_;
  CHECK(std::abs(coords[0] - -29750.345510) <= 0.000001);
  CHECK(std::abs(coords[1] - 68710.165565) <= 0.000001);
  CHECK(std::abs(coords[2] - 53116.953431) <= 0.000001);
}

TEST_CASE("parse direction") {
  using direction = IFC2X3::IfcDirection;

  constexpr auto const input =
      "#5574 = IFCDIRECTION((0.000000,0.000000,1.000000));";

  step::entry_parser p;
  p.register_parsers<direction>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 5574);
  REQUIRE(nullptr != dynamic_cast<direction*>(entry->second.get()));
  auto const& coords =
      dynamic_cast<direction*>(entry->second.get())->DirectionRatios_;
  CHECK(std::abs(coords[0] - 0.0) <= 0.000001);
  CHECK(std::abs(coords[1] - 0.0) <= 0.000001);
  CHECK(std::abs(coords[2] - 1.0) <= 0.000001);
}

TEST_CASE("parse projection") {
  using projection = IFC2X3::IfcAxis2Placement3D;

  constexpr auto const input =
      "#5563 = IFCAXIS2PLACEMENT3D(#5564, #5565, #5566);";

  step::entry_parser p;
  p.register_parsers<projection>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 5563);
  REQUIRE(nullptr != dynamic_cast<projection*>(entry->second.get()));
  auto const proj = *dynamic_cast<projection*>(entry->second.get());
  CHECK(reinterpret_cast<uintptr_t>(proj.Location_) == 5564);
  REQUIRE(proj.Axis_.has_value());
  CHECK(reinterpret_cast<uintptr_t>(*proj.Axis_) == 5565);
  REQUIRE(proj.RefDirection_.has_value());
  CHECK(reinterpret_cast<uintptr_t>(*proj.RefDirection_) == 5566);
}

TEST_CASE("parse owner history") {
  using owner_history = IFC2X3::IfcOwnerHistory;

  constexpr auto const input =
      "#5=IFCOWNERHISTORY(#8,#9,$,.DELETED.,$,$,$,1591875543);";

  step::entry_parser p;
  p.register_parsers<owner_history>();
  auto const entry = p.parse(input);
  REQUIRE(entry.has_value());
  CHECK(entry->first.id_ == 5);
  REQUIRE(nullptr !=
          dynamic_cast<IFC2X3::IfcOwnerHistory*>(entry->second.get()));
  auto const history =
      dynamic_cast<IFC2X3::IfcOwnerHistory*>(entry->second.get());
  CHECK(history->ChangeAction_ == IFC2X3::IfcChangeActionEnum::DELETED);
}
