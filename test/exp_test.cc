#include <iostream>

#include "doctest/doctest.h"

#include "boost/filesystem.hpp"

#include "cista/mmap.h"

#include "express/exp_struct_gen.h"
#include "express/get_subtypes_of.h"
#include "express/parse_exp.h"

#include "test_dir.h"

using namespace express;

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
  CHECK(schema.types_.size() == 1);
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

  i = 0U;
  CHECK(ifc_time_series.members_[i++].type_ == "IfcLabel");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcText");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcDateTimeSelect");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcDateTimeSelect");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcTimeSeriesDataTypeEnum");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcDataOriginEnum");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcLabel");
  CHECK(ifc_time_series.members_[i++].type_ == "IfcUnit");

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
  CHECK(schema.types_.front().members_.front().list_);
  CHECK(!schema.types_.front().members_.back().list_);
}

TEST_CASE("parse test schema") {
  auto const schema = parse(str);
  REQUIRE(schema.types_.size() == 9U);

  auto const& ifc_port = schema.types_[4];
  CHECK(ifc_port.name_ == "IfcPort");
  CHECK(ifc_port.subtype_of_ == "IfcProduct");
}

TEST_CASE("parse ifc schema") {
  boost::filesystem::current_path(TEST_EXECUTION_DIR);
  auto const f = cista::mmap{"test/ifc23.txt", cista::mmap::protection::READ};
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
      std::cout << "-     name=\"" << m.name_ << "\", type=\"" << m.type_
                << "\" ";
      if (m.list_) {
        std::cout << "LIST[" << m.min_size_ << ", " << m.max_size_ << "] ";
      }
      std::cout << (m.optional_ ? "OPTIONAL" : "") << "\n";
    }
  }

  CHECK(get_subtypes_of(schema, "IfcProduct").size() == 90);
}