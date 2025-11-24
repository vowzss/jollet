#include <filesystem>
#include <memory>
#include <optional>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "serris/providers/json.h"
#include "serris/types/jvalue.h"

using namespace serris;
using namespace serris::types;

namespace fs = std::filesystem;

static std::string format_duration(std::chrono::nanoseconds ns) {
    using namespace std::chrono;

    auto ms = duration_cast<milliseconds>(ns) % 1000;
    auto sec = duration_cast<seconds>(ns) % 60;
    auto min = duration_cast<minutes>(ns);

    std::ostringstream oss;
    oss << min.count() << "m " << sec.count() << "s " << ms.count() << "ms";
    return oss.str();
}

TEST_CASE("types/jvalue.h: basic types") {

    SUBCASE("null") {
        jvalue val;
        CHECK(val.is_null());
    }

    SUBCASE("bools") {
        jvalue val1(true);
        jvalue val2(false);

        CHECK(val1.as_bool() == true);
        CHECK(val2.as_bool() == false);

        const std::optional<bool> val1_ptr = val1.try_as_bool();
        const std::optional<bool> val2_ptr = val2.try_as_bool();

        REQUIRE(val1_ptr);
        REQUIRE(val2_ptr);
        CHECK(*val1_ptr == true);
        CHECK(*val2_ptr == false);
    }

    SUBCASE("ints") {
        jvalue val(123);

        CHECK(val.as_int() == 123);
        CHECK(val.as_short() == 123);
        CHECK(val.as_long() == 123);

        const std::optional<int> val_ptr = val.try_as_int();
        REQUIRE(val_ptr);
        CHECK(*val_ptr == 123);
    }

    SUBCASE("floats") {
        jvalue val1(3.14f);
        jvalue val2(2.718);

        CHECK(val1.as_float() == doctest::Approx(3.14f));
        CHECK(val1.as_double() == doctest::Approx(3.14));

        CHECK(val2.as_double() == doctest::Approx(2.718));
        CHECK(val2.as_float() == doctest::Approx(2.718f));

        const std::optional<double> val2_ptr = val2.try_as_double();
        REQUIRE(val2_ptr);
        CHECK(*val2_ptr == doctest::Approx(2.718));
    }

    SUBCASE("strings") {
        jvalue val1(std::string("hello"));
        jvalue val2("world");

        CHECK(val1.as_string() == "hello");
        CHECK(val2.as_string() == "world");

        auto val1_ptr = val1.try_as_string();
        REQUIRE(val1_ptr);
        CHECK(*val1_ptr == "hello");

        auto val2_ptr = val2.try_as_string();
        REQUIRE(val2_ptr);
        CHECK(*val2_ptr == "world");
    }
}

TEST_CASE("types/jvalue.h: containers") {

    SUBCASE("objects") {
        jvalue obj(jvalue::jobject{});
        obj["a"] = jvalue(1);
        obj["b"] = jvalue(2);
        obj["c"] = jvalue(3);

        CHECK(obj["a"].as_int() == 1);
        CHECK(obj["b"].as_int() == 2);

        obj["c"] = jvalue(3);
        CHECK(obj["c"].as_int() == 3);

        auto obj_ptr = obj.try_as_object();
        REQUIRE(obj_ptr);
        CHECK(obj_ptr->at("a")->as_int() == 1);
    }

    SUBCASE("arrays") {
        jvalue arr(jvalue::jarray{});
        arr[0] = jvalue(1);
        arr[1] = jvalue(2);
        arr[2] = jvalue(3);

        CHECK(arr[0].as_int() == 1);
        CHECK(arr[1].as_int() == 2);
        CHECK(arr[2].as_int() == 3);

        arr[1] = jvalue(42);
        CHECK(arr[1].as_int() == 42);

        arr[5] = jvalue(99);
        CHECK(arr[5].as_int() == 99);

        auto arr_ptr = arr.try_as_array();
        REQUIRE(arr_ptr);
        CHECK(arr_ptr->size() >= 6);
    }

    SUBCASE("nested objects and arrays") {
        jvalue obj(jvalue::jobject{});
        obj["numbers"][0] = jvalue(1);
        obj["numbers"][1] = jvalue(2);
        obj["numbers"][2] = jvalue(3);
        obj["info"]["name"] = jvalue("Chad");
        obj["info"]["age"] = jvalue(30);

        CHECK(obj["numbers"][0].as_int() == 1);
        CHECK(obj["numbers"][2].as_int() == 3);
        CHECK(obj["info"]["name"].as_string() == "Chad");
        CHECK(obj["info"]["age"].as_int() == 30);
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
        jvalue obj{jvalue::jobject{}};
        obj["x"] = jvalue(1);

        auto val1 = obj.clone();
        auto val2 = obj.clone();

        CHECK(*val1 == *val2);
    }
}

TEST_CASE("providers/json.h: serialize") {
    jvalue obj(jvalue::jobject{});
    obj["numbers"][0] = jvalue(1);
    obj["numbers"][1] = jvalue(2);
    obj["numbers"][2] = jvalue(3);
    obj["info"]["name"] = jvalue("Chad");
    obj["info"]["age"] = jvalue(30);
    obj["info"]["skills"][0] = jvalue("C++");
    obj["info"]["skills"][1] = jvalue("Python");
    obj["pi"] = jvalue(3.141592653589793);
    obj["big_number"] = jvalue(9223372036854775807);
    obj["small_number"] = jvalue(-32768);
    obj["float_val"] = jvalue(2.71828);

    SUBCASE("compact JSON") {
        std::string compact = providers::json::serialize(obj);
        std::cout << "Compact JSON:\n" << compact << "\n";

        CHECK(!compact.empty());
        CHECK(compact.find("Chad") != std::string::npos);
        CHECK(compact.find("C++") != std::string::npos);
    }

    SUBCASE("pretty JSON") {
        std::string pretty = providers::json::serialize(obj, true);
        std::cout << "Pretty JSON:\n" << pretty << "\n";

        CHECK(!pretty.empty());
        CHECK(pretty.find('\n') != std::string::npos);
        CHECK(pretty.find("Chad") != std::string::npos);
        CHECK(pretty.find("Python") != std::string::npos);
    }
}

TEST_CASE("providers/json.h: deserialize") {
    std::string json = R"({
        "numbers": [1, 2, 3],
        "info": {
            "name": "Chad",
            "age": 30,
            "skills": ["C++", "Python"]
        },
        "pi": 3.141592653589793,
        "big_number": 9223372036854775807,
        "small_number": -32768,
        "float_val": 2.71828
    })";

    jvalue root = providers::json::deserialize(json);

    const jvalue::jobject* obj = root.try_as_object();
    REQUIRE(obj != nullptr);

    const jvalue::jarray* numbers = obj->at("numbers")->try_as_array();
    REQUIRE(numbers != nullptr);
    CHECK((*numbers)[0]->try_as_int().value() == 1);
    CHECK((*numbers)[1]->try_as_int().value() == 2);
    CHECK((*numbers)[2]->try_as_int().value() == 3);

    const jvalue::jobject* info = root["info"].try_as_object();
    REQUIRE(info != nullptr);

    const std::string* name = info->at("name")->try_as_string();
    REQUIRE(name != nullptr);
    CHECK(*name == "Chad");
    CHECK(info->at("age")->try_as_int().value() == 30);

    CHECK(root["pi"].try_as_double().value() == doctest::Approx(3.141592653589793));
    CHECK(root["big_number"].try_as_long().value() == 9223372036854775807LL);
    CHECK(root["small_number"].try_as_short().value() == -32768);
    CHECK(root["float_val"].try_as_float().value() == doctest::Approx(2.71828f));

    const jvalue::jarray* skills = info->at("skills")->try_as_array();
    REQUIRE(skills != nullptr);
    CHECK(*(*skills)[0]->try_as_string() == "C++");
    CHECK(*(*skills)[1]->try_as_string() == "Python");
}

TEST_CASE("providers/json.h: from_file") {
    fs::path rssPath = fs::absolute("resources");
    fs::path fullPath = rssPath / "sample_min.json";
    std::cout << fullPath << "\n";

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::optional<jvalue> root = providers::json::from_file(fullPath);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::cout << "Took: " << format_duration(end - start) << 'ms.\n';
    CHECK(root.has_value());
}
