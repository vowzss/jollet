#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include "serris/providers/json.h"
#include "serris/types/jvalue.h"
#include "serris/utils/string.h"

using namespace serris;
using namespace serris::utils;
using namespace serris::types;

TEST_CASE("utils/string.h") {
    std::string s = "\t\"Hello \n World\"  \t  !\b\f\rTest";

    SUBCASE("sanitize") {
        std::string copy = s;
        string::sanitize(copy);

        CHECK(copy.find(' ') != std::string::npos);
        CHECK(copy.find('\t') == std::string::npos);
        CHECK(copy.find('\n') == std::string::npos);
        CHECK(copy.find('\r') == std::string::npos);
    }

    SUBCASE("remove_quotes") {
        std::string copy = s;
        string::remove_quotes(copy);

        CHECK(copy.find('"') == std::string::npos);
    }

    SUBCASE("remove_whitespace") {
        std::string copy = s;
        string::remove_whitespace(copy);

        CHECK(copy.find('\t') == std::string::npos);
        CHECK(copy.find('\n') == std::string::npos);
        CHECK(copy.find('\r') == std::string::npos);
        CHECK(copy.find('\f') == std::string::npos);
    }
}

TEST_CASE("types/jvalue.h: basic types") {

    SUBCASE("null") {
        jvalue j;
        CHECK(j.is_null());
    }

    SUBCASE("bools") {
        jvalue j_true(true);
        jvalue j_false(false);

        CHECK(j_true.as_bool() == true);
        CHECK(j_false.as_bool() == false);

        auto try_true = j_true.try_as_bool();
        auto try_false = j_false.try_as_bool();

        REQUIRE(try_true);
        REQUIRE(try_false);
        CHECK(*try_true == true);
        CHECK(*try_false == false);
    }

    SUBCASE("ints") {
        jvalue j_int(123);

        CHECK(j_int.as_int() == 123);
        CHECK(j_int.as_short() == 123);
        CHECK(j_int.as_long() == 123);

        auto try_val = j_int.try_as<int>();
        REQUIRE(try_val);
        CHECK(*try_val == 123);
    }

    SUBCASE("floats") {
        jvalue j_float(3.14f);
        jvalue j_double(2.718);

        CHECK(j_float.as_float() == doctest::Approx(3.14f));
        CHECK(j_float.as_double() == doctest::Approx(3.14));

        CHECK(j_double.as_double() == doctest::Approx(2.718));
        CHECK(j_double.as_float() == doctest::Approx(2.718f));

        auto try_val = j_double.try_as<double>();
        REQUIRE(try_val);
        CHECK(*try_val == doctest::Approx(2.718));
    }

    SUBCASE("strings") {
        jvalue j1(std::string("hello"));
        jvalue j2("world");

        CHECK(j1.as_string() == "hello");
        CHECK(j2.as_string() == "world");

        auto try1 = j1.try_as_string();
        auto try2 = j2.try_as_string();

        REQUIRE(try1);
        REQUIRE(try2);
        CHECK(*try1 == "hello");
        CHECK(*try2 == "world");
    }
}

TEST_CASE("types/jvalue.h: containers") {

    SUBCASE("objects") {
        jvalue::jobject obj{{"a", jvalue(1)}, {"b", jvalue(2)}};
        jvalue j_obj(obj);

        CHECK(j_obj["a"].as_int() == 1);
        CHECK(j_obj["b"].as_int() == 2);

        j_obj["c"] = jvalue(3);
        CHECK(j_obj["c"].as_int() == 3);

        auto try_obj = j_obj.try_as_object();
        REQUIRE(try_obj);
        CHECK(try_obj->at("a").as_int() == 1);
    }

    SUBCASE("arrays") {
        jvalue::jarray arr{jvalue(1), jvalue(2), jvalue(3)};
        jvalue j_arr(arr);

        CHECK(j_arr[0].as_int() == 1);
        CHECK(j_arr[1].as_int() == 2);
        CHECK(j_arr[2].as_int() == 3);

        j_arr[1] = jvalue(42);
        CHECK(j_arr[1].as_int() == 42);

        j_arr[5] = jvalue(99);
        CHECK(j_arr[5].as_int() == 99);

        auto try_arr = j_arr.try_as_array();
        REQUIRE(try_arr);
        CHECK(try_arr->size() >= 6);
    }

    SUBCASE("nested objects and arrays") {
        jvalue root(jvalue::jobject{});

        root["numbers"][0] = jvalue(1);
        root["numbers"][1] = jvalue(2);
        root["numbers"][2] = jvalue(3);

        root["info"]["name"] = jvalue("Chad");
        root["info"]["age"] = jvalue(30);

        CHECK(root["numbers"][0].as_int() == 1);
        CHECK(root["numbers"][2].as_int() == 3);
        CHECK(root["info"]["name"].as_string() == "Chad");
        CHECK(root["info"]["age"].as_int() == 30);
    }
}

TEST_CASE("types/jvalue.h: equality") {

    SUBCASE("primitive equality") {
        jvalue a(42);
        jvalue b(42);
        jvalue c(43);

        CHECK(a == b);
        CHECK(a != c);
    }

    SUBCASE("object equality") {
        jvalue::jobject obj{{"x", jvalue(1)}};
        jvalue j1(obj);
        jvalue j2(obj);

        CHECK(j1 == j2);
    }
}

TEST_CASE("providers/json.h: stringify") {
    jvalue root(jvalue::jobject{});

    root["numbers"][0] = jvalue(1);
    root["numbers"][1] = jvalue(2);
    root["numbers"][2] = jvalue(3);

    root["info"]["name"] = jvalue("Chad");
    root["info"]["age"] = jvalue(30);
    root["info"]["skills"][0] = jvalue("C++");
    root["info"]["skills"][1] = jvalue("Python");

    SUBCASE("compact JSON") {
        std::string compact = providers::json::stringify(root);
        std::cout << "Compact JSON:\n" << compact << "\n";

        CHECK(!compact.empty());
        CHECK(compact.find("Chad") != std::string::npos);
        CHECK(compact.find("C++") != std::string::npos);
    }

    SUBCASE("pretty JSON") {
        std::string pretty = providers::json::stringify(root, true);
        std::cout << "Pretty JSON:\n" << pretty << "\n";

        CHECK(!pretty.empty());
        CHECK(pretty.find('\n') != std::string::npos);
        CHECK(pretty.find("Chad") != std::string::npos);
        CHECK(pretty.find("Python") != std::string::npos);
    }
}