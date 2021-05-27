#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "doctest/doctest.h"

#include "boost/filesystem.hpp"

#include "cista/mmap.h"

#include "utl/overloaded.h"

#include "express/exp_struct_gen.h"
#include "express/parse_exp.h"

#include "test_dir.h"

using namespace express;

std::set<std::string_view> get_subtypes_of(schema const& s,
                                           std::string_view supertype) {
  std::unordered_map<std::string_view, std::vector<std::string_view>> subtypes;
  for (auto const& t : s.types_) {
    subtypes[t.subtype_of_].emplace_back(t.name_);
  }

  std::queue<std::string_view> q;
  q.emplace(supertype);

  std::set<std::string_view> rec_subtypes;
  while (!q.empty()) {
    auto const next = q.front();
    q.pop();
    if (!rec_subtypes.emplace(next).second) {
      continue;
    }
    if (auto const it = subtypes.find(next); it != end(subtypes)) {
      for (auto const& n : it->second) {
        q.emplace(n);
      }
    }
  }
  return rec_subtypes;
}

constexpr auto const* const str =
    // "(*\n"
    // "    Comment\n"
    // "*)\n\n"
    "SCHEMA IFC2X3;\n"
    "\n"
    "TYPE IfcAbsorbedDoseMeasure = REAL;\n"
    "END_TYPE;\n"
    "\n"
    "TYPE IfcAccelerationMeasure = REAL;\n"
    "END_TYPE;\n"
    "\n"
    "TYPE IfcNormalisedRatioMeasure = IfcRatioMeasure;\n"
    "  WHERE\n"
    "    WR1 : {0.0 <= SELF <= 1.0};\n"
    "END_TYPE;\n"
    "\n"
    "TYPE IfcGeometricSetSelect = SELECT\n"
    "  (IfcPoint\n"
    "  ,IfcCurve\n"
    "  ,IfcSurface\n);"
    "END_TYPE;\n"
    "\n"
    "ENTITY IfcPort\n"
    "  ABSTRACT SUPERTYPE OF (ONEOF\n"
    "      (IfcDistributionPort))\n"
    "  SUBTYPE OF (IfcProduct);\n"
    "  INVERSE\n"
    "    ContainedIn : IfcRelConnectsPortToElement FOR RelatingPort;\n"
    "    ConnectedFrom : SET [0:1] OF IfcRelConnectsPorts FOR RelatedPort;\n"
    "    ConnectedTo : SET [0:1] OF IfcRelConnectsPorts FOR RelatingPort;\n"
    "END_ENTITY;\n"
    "\n"
    "TYPE IfcActionTypeEnum = ENUMERATION OF\n"
    "  (PERMANENT_G\n"
    "  ,VARIABLE_Q\n"
    "  ,EXTRAORDINARY_A\n"
    "  ,USERDEFINED\n"
    "  ,NOTDEFINED);\n"
    "END_TYPE;\n"
    "\n"
    "ENTITY IfcTimeSeries\n"
    " ABSTRACT SUPERTYPE OF (ONEOF\n"
    "  (IfcIrregularTimeSeries\n"
    "  ,IfcRegularTimeSeries));\n"
    "  Name : IfcLabel;\n"
    "  Description : OPTIONAL IfcText;\n"
    "  StartTime : IfcDateTimeSelect;\n"
    "  EndTime : IfcDateTimeSelect;\n"
    "  TimeSeriesDataType : IfcTimeSeriesDataTypeEnum;\n"
    "  DataOrigin : IfcDataOriginEnum;\n"
    "  UserDefinedDataOrigin : OPTIONAL IfcLabel;\n"
    "  Unit : OPTIONAL IfcUnit;\n"
    " INVERSE\n"
    "  DocumentedBy : SET [0:1] OF IfcTimeSeriesReferenceRelationship FOR "
    "ReferencedTimeSeries;\n"
    "END_ENTITY;\n"
    "\n"
    "ENTITY Ifc2DCompositeCurve\n"
    " SUBTYPE OF (IfcCompositeCurve);\n"
    " WHERE\n"
    "  WR1 : SELF\\IfcCompositeCurve.ClosedCurve;\n"
    "  WR2 : SELF\\IfcCurve.Dim = 2;\n"
    "END_ENTITY;\n"
    "\n"
    "ENTITY IfcElement\n"
    " ABSTRACT SUPERTYPE OF (ONEOF\n"
    " (IfcBuildingElement\n"
    " ,IfcDistributionElement\n"
    " ,IfcElectricalElement\n"
    " ,IfcElementAssembly\n"
    " ,IfcElementComponent\n"
    " ,IfcEquipmentElement\n"
    " ,IfcFeatureElement\n"
    " ,IfcFurnishingElement\n"
    " ,IfcTransportElement\n"
    " ,IfcVirtualElement))\n"
    " SUBTYPE OF (IfcProduct);\n"
    " Tag : OPTIONAL IfcIdentifier;\n"
    " INVERSE\n"
    " HasStructuralMember : SET [0:?] OF IfcRelConnectsStructuralElement FOR "
    "RelatingElement;\n"
    "END_ENTITY;"
    "\n"
    "END_SCHEMA";

TEST_CASE("abstract supertype") {
  constexpr auto const* const s =
      "SCHEMA IFC2X3;\n"
      "\n"
      "ENTITY IfcTimeSeries\n"
      " ABSTRACT SUPERTYPE OF (ONEOF\n"
      "  (IfcIrregularTimeSeries\n"
      "  ,IfcRegularTimeSeries));\n"
      "  Name : IfcLabel;\n"
      "  Description : OPTIONAL IfcText;\n"
      "  StartTime : IfcDateTimeSelect;\n"
      "  EndTime : IfcDateTimeSelect;\n"
      "  TimeSeriesDataType : IfcTimeSeriesDataTypeEnum;\n"
      "  DataOrigin : IfcDataOriginEnum;\n"
      "  UserDefinedDataOrigin : OPTIONAL IfcLabel;\n"
      "  Unit : OPTIONAL IfcUnit;\n"
      " INVERSE\n"
      "  DocumentedBy : SET [0:1] OF IfcTimeSeriesReferenceRelationship FOR "
      "ReferencedTimeSeries;\n"
      "END_ENTITY;\n"
      "\n"
      "END_SCHEMA";

  auto const schema = parse(s);
  REQUIRE(schema.types_.size() == 1);
  CHECK(schema.types_.front().name_ == "IfcTimeSeries");

  auto const ifc_time_series = schema.types_.front();
  auto i = 0U;
  CHECK(ifc_time_series.members_[i++].name_ == "Name");
  CHECK(ifc_time_series.members_[i++].name_ == "Description");
  CHECK(ifc_time_series.members_[i++].name_ == "StartTime");
  CHECK(ifc_time_series.members_[i++].name_ == "EndTime");
  CHECK(ifc_time_series.members_[i++].name_ == "TimeSeriesDataType");
  CHECK(ifc_time_series.members_[i++].name_ == "DataOrigin");
  CHECK(ifc_time_series.members_[i++].name_ == "UserDefinedDataOrigin");
  CHECK(ifc_time_series.members_[i++].name_ == "Unit");

  // TODO(felix)
  //  i = 0U;
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcLabel");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcText");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcDateTimeSelect");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcDateTimeSelect");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcTimeSeriesDataTypeEnum");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcDataOriginEnum");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcLabel");
  //  CHECK(ifc_time_series.members_[i++].type_ == "IfcUnit");

  i = 0U;
  CHECK(ifc_time_series.members_[i++].optional_ == false);
  CHECK(ifc_time_series.members_[i++].optional_ == true);
  CHECK(ifc_time_series.members_[i++].optional_ == false);
  CHECK(ifc_time_series.members_[i++].optional_ == false);
  CHECK(ifc_time_series.members_[i++].optional_ == false);
  CHECK(ifc_time_series.members_[i++].optional_ == false);
  CHECK(ifc_time_series.members_[i++].optional_ == true);
  CHECK(ifc_time_series.members_[i++].optional_ == true);
}

TEST_CASE("express parser: ifc address") {
  constexpr auto const* const input = R"(
SCHEMA IFC2X3;

ENTITY IfcAddress
 ABSTRACT SUPERTYPE OF (ONEOF
	(IfcPostalAddress
	,IfcTelecomAddress));
	Purpose : OPTIONAL IfcAddressTypeEnum;
	Description : OPTIONAL IfcText;
	UserDefinedPurpose : OPTIONAL IfcLabel;
 INVERSE
	OfPerson : SET [0:?] OF IfcPerson FOR Addresses;
	OfOrganization : SET [0:?] OF IfcOrganization FOR Addresses;
 WHERE
	WR1 : (NOT(EXISTS(Purpose))) OR
            ((Purpose <> IfcAddressTypeEnum.USERDEFINED) OR
            ((Purpose = IfcAddressTypeEnum.USERDEFINED) AND
              EXISTS(SELF.UserDefinedPurpose)));
END_ENTITY;

END_SCHEMA
)";

  auto const schema = parse(input);
  REQUIRE(schema.types_.size() == 1);
}

TEST_CASE("express parser: LIST OF") {
  constexpr auto const* const input = R"(
SCHEMA IFC2X3;

ENTITY IfcTypeProduct
 SUPERTYPE OF (ONEOF
	(IfcDoorStyle
	,IfcElementType
	,IfcWindowStyle))
 SUBTYPE OF (IfcTypeObject);
	RepresentationMaps : OPTIONAL LIST [1:?] OF UNIQUE IfcRepresentationMap;
	Tag : OPTIONAL IfcLabel;
 WHERE
	WR41 : NOT(EXISTS(SELF\IfcTypeObject.ObjectTypeOf[1])) OR
             (SIZEOF(QUERY(temp <* SELF\IfcTypeObject.ObjectTypeOf[1].RelatedObjects |
               NOT('IFC2X3.IFCPRODUCT' IN TYPEOF(temp)))
             ) = 0);
END_ENTITY;

END_SCHEMA
)";

  auto const schema = parse(input);
  REQUIRE(schema.types_.size() == 1);
  REQUIRE(schema.types_.front().members_.size() == 2);

  auto const& m = schema.types_.front().members_.front();
  CHECK(m.name_ == "RepresentationMaps");
  CHECK(m.is_list());
  CHECK(m.get_type_name() == "IfcRepresentationMap");

  // TODO(felix)
  //  CHECK(schema.types_.front().members_.front().list_);
  //  CHECK(!schema.types_.front().members_.back().list_);
}

TEST_CASE("parse test schema") {
  auto const schema = parse(str);
  REQUIRE(schema.types_.size() == 9U);

  auto const& ifc_port = schema.types_[4];
  CHECK(ifc_port.name_ == "IfcPort");
  CHECK(ifc_port.subtype_of_ == "IfcProduct");
}

TEST_CASE("parse ifc site") {
  constexpr auto const* const exp_input = R"(
SCHEMA IFC2X3;

TYPE IfcCompoundPlaneAngleMeasure = LIST [3:4] OF INTEGER;
 WHERE
	WR1 : { -360 <= SELF[1] < 360 };
	WR2 : { -60 <= SELF[2] < 60 };
	WR3 : { -60 <= SELF[3] < 60 };
	WR4 : ((SELF[1] >= 0) AND (SELF[2] >= 0) AND (SELF[3] >= 0)) OR ((SELF[1] <= 0) AND (SELF[2] <= 0) AND (SELF[3] <= 0));
END_TYPE;

ENTITY IfcSite
 SUBTYPE OF (IfcSpatialStructureElement);
	RefLatitude : OPTIONAL IfcCompoundPlaneAngleMeasure;
	RefLongitude : OPTIONAL IfcCompoundPlaneAngleMeasure;
	RefElevation : OPTIONAL IfcLengthMeasure;
	LandTitleNumber : OPTIONAL IfcLabel;
	SiteAddress : OPTIONAL IfcPostalAddress;
END_ENTITY;

END_SCHEMA
)";

  auto const schema = parse(exp_input);
  REQUIRE(schema.types_.size() == 2U);

  auto const& angle = schema.types_.at(0);
  CHECK(angle.list_ == true);
  CHECK(angle.data_type_ == data_type::INTEGER);

  auto const& site = schema.types_.at(1);
  CHECK(site.name_ == "IfcSite");
  CHECK(boost::get<type_name>(site.members_.at(1).type_).name_ ==
        "IfcCompoundPlaneAngleMeasure");
}

TEST_CASE("parse ifc schema") {
  boost::filesystem::current_path(TEST_EXECUTION_DIR);
  auto const f =
      cista::mmap{"express/test/ifc23.txt", cista::mmap::protection::READ};
  auto const schema = parse(
      std::string_view{reinterpret_cast<char const*>(f.data()), f.size()});

  for (auto const& t : schema.types_) {
    std::cout << "- " << t.name_ << " "
              << data_type_str[static_cast<std::underlying_type_t<data_type>>(
                     t.data_type_)];
    if (!t.alias_.empty()) {
      std::cout << " ALIAS=\"" << t.alias_ << "\"";
    }
    if (t.list_) {
      std::cout << " LIST[" << t.min_size_ << ", " << t.max_size_ << "]";
    }
    std::cout << "\n";
    for (auto const& m : t.members_) {
      std::cout << "-     name=\"" << m.name_ << "\", type=\"";
      struct visit {
        void operator()(express::type_name const& s) {
          std::cout << s.name_ << "\n";
        }
        void operator()(express::list const& l) {
          std::cout << "LIST[" << l.min_ << ", " << l.max_ << "] ";
          boost::apply_visitor([&](auto&& el) { (*this)(el); }, l.m_);
        }
      };
      boost::apply_visitor(visit{}, m.type_);
      std::cout << "\" ";
      std::cout << (m.optional_ ? "OPTIONAL" : "") << "\n";
    }
  }

  CHECK(get_subtypes_of(schema, "IfcProduct").size() == 90);
}

TEST_CASE("alias to SET") {
  constexpr auto const* exp_input = R"(
SCHEMA IFC2X3;

ENTITY IfcRoot
 ABSTRACT SUPERTYPE OF (ONEOF
    (IfcObjectDefinition
    ,IfcPropertyDefinition
    ,IfcRelationship));
	GlobalId : IfcGloballyUniqueId;
	OwnerHistory : OPTIONAL IfcOwnerHistory;
	Name : OPTIONAL IfcLabel;
	Description : OPTIONAL IfcText;
 UNIQUE
	UR1 : GlobalId;
END_ENTITY;

TYPE IfcPropertySetDefinitionSet = SET [1:?] OF IfcPropertySetDefinition;
END_TYPE;

ENTITY IfcPropertyDefinition
 ABSTRACT SUPERTYPE OF (ONEOF
    (IfcPropertySetDefinition
    ,IfcPropertyTemplateDefinition))
 SUBTYPE OF (IfcRoot);
 INVERSE
	HasContext : SET [0:1] OF IfcRelDeclares FOR RelatedDefinitions;
	HasAssociations : SET [0:?] OF IfcRelAssociates FOR RelatedObjects;
END_ENTITY;

ENTITY IfcPropertySetDefinition
 ABSTRACT SUPERTYPE OF (ONEOF
    (IfcPreDefinedPropertySet
    ,IfcPropertySet
    ,IfcQuantitySet))
 SUBTYPE OF (IfcPropertyDefinition);
 INVERSE
	DefinesType : SET [0:?] OF IfcTypeObject FOR HasPropertySets;
	IsDefinedBy : SET [0:?] OF IfcRelDefinesByTemplate FOR RelatedPropertySets;
	DefinesOccurrence : SET [0:?] OF IfcRelDefinesByProperties FOR RelatingPropertyDefinition;
END_ENTITY;

TYPE IfcPropertySetDefinitionSelect = SELECT
	(IfcPropertySetDefinition
	,IfcPropertySetDefinitionSet);
END_TYPE;

ENTITY IfcRelDefinesByProperties
 SUBTYPE OF (IfcRelDefines);
	RelatingPropertyDefinition : IfcPropertySetDefinitionSelect;
 WHERE
	NoRelatedTypeObject : SIZEOF(QUERY(Types <* SELF\IfcRelDefinesByProperties.RelatedObjects |  'IFC4.IFCTYPEOBJECT' IN TYPEOF(Types))) = 0;
END_ENTITY;

END_SCHEMA)";

  auto const schema = parse(exp_input);
  REQUIRE(schema.types_.size() == 6U);

  auto const& x = *schema.type_map_.at("IfcPropertySetDefinitionSet");
  CHECK(x.list_);
  CHECK(x.alias_ == "IfcPropertySetDefinition");
  CHECK(x.data_type_ == data_type::ALIAS);

  std::stringstream ss;
  for (auto const& t : schema.types_) {
    CHECK_NOTHROW(generate_header(ss, schema, t));
  }
}

TEST_CASE("list of list") {
  constexpr auto const* exp_input = R"(
SCHEMA IFC2X3;

TYPE IfcLengthMeasure = REAL;
END_TYPE;

ENTITY IfcCartesianPoint
 SUBTYPE OF (IfcPoint);
	Coordinates : LIST [1:3] OF IfcLengthMeasure;
 DERIVE
	Dim : IfcDimensionCount := HIINDEX(Coordinates);
 WHERE
	CP2Dor3D : HIINDEX(Coordinates) >= 2;
END_ENTITY;

ENTITY IfcBSplineSurface
 ABSTRACT SUPERTYPE OF (ONEOF
    (IfcBSplineSurfaceWithKnots))
 SUBTYPE OF (IfcBoundedSurface);
	ControlPointsList : LIST [2:?] OF LIST [2:?] OF IfcCartesianPoint;
 DERIVE
	UUpper : IfcInteger := SIZEOF(ControlPointsList) - 1;
	VUpper : IfcInteger := SIZEOF(ControlPointsList[1]) - 1;
	ControlPoints : ARRAY [0:UUpper] OF ARRAY [0:VUpper] OF IfcCartesianPoint := IfcMakeArrayOfArray(ControlPointsList,
0,UUpper,0,VUpper);
END_ENTITY;

END_SCHEMA)";

  auto const schema = parse(exp_input);

  std::stringstream ss;
  for (auto const& t : schema.types_) {
    CHECK_NOTHROW(generate_header(ss, schema, t));
  }
  std::cout << ss.str() << "\n";
}
