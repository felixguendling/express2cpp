#include <iostream>

#include "doctest/doctest.h"

#include "step/model.h"
#include "step/parse_step.h"

TEST_CASE("parse basic data types") {
  using step::parse_step;
  SUBCASE("int") {
    SUBCASE("before comma") {
      int i{};
      auto s = utl::cstr{"123,"};
      parse_step(s, i);
      CHECK(i == 123);
      CHECK(s.view() == ",");
    }
    SUBCASE("before bracket") {
      int i{};
      auto s = utl::cstr{"123)"};
      parse_step(s, i);
      CHECK(i == 123);
      CHECK(s.view() == ")");
    }
  }
  SUBCASE("double") {
    SUBCASE("before comma") {
      double d{};
      auto s = utl::cstr{"123,"};
      parse_step(s, d);
      CHECK(d == 123);
      CHECK(s.view() == ",");
    }
    SUBCASE("before bracket") {
      double d{};
      auto s = utl::cstr{"123)"};
      parse_step(s, d);
      CHECK(d == 123);
      CHECK(s.view() == ")");
    }
    SUBCASE("E notation") {
      double d{};
      auto s = utl::cstr{"123E2)"};
      parse_step(s, d);
      CHECK(d == 12300);
      CHECK(s.view() == ")");
    }
    SUBCASE("full test") {
      double d{};
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
    void* ptr{};
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