#include <filesystem>
#include <memory>
#include <optional>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "serris/providers/Json.h"
#include "serris/types/JValue.h"

using namespace serris;

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

TEST_CASE("types/JValue.h: basic types") {

    SUBCASE("null") {
        JValue val;
        CHECK(val.isNull());
    }

    SUBCASE("bools") {
        JValue val1(true);
        JValue val2(false);

        CHECK(val1.asBool() == true);
        CHECK(val2.asBool() == false);

        const std::optional<bool> val1_ptr = val1.tryAsBool();
        const std::optional<bool> val2_ptr = val2.tryAsBool();

        REQUIRE(val1_ptr);
        REQUIRE(val2_ptr);
        CHECK(*val1_ptr == true);
        CHECK(*val2_ptr == false);
    }

    SUBCASE("ints") {
        JValue val(123);

        CHECK(val.asInt() == 123);
        CHECK(val.asShort() == 123);
        CHECK(val.asLong() == 123);

        const std::optional<int> val_ptr = val.tryAsInt();
        REQUIRE(val_ptr);
        CHECK(*val_ptr == 123);
    }

    SUBCASE("floats") {
        JValue val1(3.14f);
        JValue val2(2.718);

        CHECK(val1.asFloat() == doctest::Approx(3.14f));
        CHECK(val1.asDouble() == doctest::Approx(3.14));

        CHECK(val2.asDouble() == doctest::Approx(2.718));
        CHECK(val2.asFloat() == doctest::Approx(2.718f));

        const std::optional<double> val2_ptr = val2.tryAsDouble();
        REQUIRE(val2_ptr);
        CHECK(*val2_ptr == doctest::Approx(2.718));
    }

    SUBCASE("strings") {
        JValue val1(std::string("hello"));
        JValue val2("world");

        CHECK(val1.asString() == "hello");
        CHECK(val2.asString() == "world");

        auto val1_ptr = val1.tryAsString();
        REQUIRE(val1_ptr);
        CHECK(*val1_ptr == "hello");

        auto val2_ptr = val2.tryAsString();
        REQUIRE(val2_ptr);
        CHECK(*val2_ptr == "world");
    }
}

TEST_CASE("types/JValue.h: containers") {
    SUBCASE("objects") {
        JValue obj = JValue::makeObject();
        obj["a"] = JValue(1);
        obj["b"] = JValue(2);
        obj["c"] = JValue(3);

        CHECK(obj["a"].asInt() == 1);
        CHECK(obj["b"].asInt() == 2);

        obj["c"] = JValue(3);
        CHECK(obj["c"].asInt() == 3);

        CHECK(obj["a"].asInt() == 1);
    }

    SUBCASE("arrays") {
        JValue arr = JValue::makeArray();
        arr[0] = JValue(1);
        arr[1] = JValue(2);
        arr[2] = JValue(3);

        CHECK(arr[0].asInt() == 1);
        CHECK(arr[1].asInt() == 2);
        CHECK(arr[2].asInt() == 3);

        arr[1] = JValue(42);
        CHECK(arr[1].asInt() == 42);

        arr[5] = JValue(99);
        CHECK(arr[5].asInt() == 99);
    }

    SUBCASE("nested objects and arrays") {
        JValue obj = JValue::makeObject();
        obj["numbers"][0] = JValue(1);
        obj["numbers"][1] = JValue(2);
        obj["numbers"][2] = JValue(3);
        obj["info"]["name"] = JValue("Chad");
        obj["info"]["age"] = JValue(30);

        CHECK(obj["numbers"][0].asInt() == 1);
        CHECK(obj["numbers"][2].asInt() == 3);
        CHECK(obj["info"]["name"].asString() == "Chad");
        CHECK(obj["info"]["age"].asInt() == 30);
    }
}

TEST_CASE("types/JValue.h: equality") {
    SUBCASE("primitive equality") {
        JValue a(42);
        JValue b(42);
        JValue c(43);

        CHECK(a == b);
        CHECK(a != c);
    }

    SUBCASE("object equality") {
        JValue obj = JValue::makeObject();
        obj["x"] = JValue(1);

        auto val1 = obj.clone();
        auto val2 = obj.clone();

        CHECK(*val1 == *val2);
    }
}

TEST_CASE("providers/json.h: serialize") {
    JValue obj = JValue::makeObject();
    obj["numbers"][0] = JValue(1);
    obj["numbers"][1] = JValue(2);
    obj["numbers"][2] = JValue(3);
    obj["info"]["name"] = JValue("Chad");
    obj["info"]["age"] = JValue(30);
    obj["info"]["skills"][0] = JValue("C++");
    obj["info"]["skills"][1] = JValue("Python");
    obj["pi"] = JValue(3.141592653589793);
    obj["big_number"] = JValue(9223372036854775807);
    obj["small_number"] = JValue(-32768);
    obj["float_val"] = JValue(2.71828);

    SUBCASE("compact JSON") {
        std::string compact = Json::serialize(obj);
        std::cout << "Compact JSON:\n" << compact << "\n";

        CHECK(!compact.empty());
        CHECK(compact.find("Chad") != std::string::npos);
        CHECK(compact.find("C++") != std::string::npos);
    }

    SUBCASE("pretty JSON") {
        std::string pretty = Json::serialize(obj, true);
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

    JValue root = Json::deserialize(json);
    REQUIRE(!root.isNull());

    const JValue* numbers = root.find("numbers");
    REQUIRE(numbers != nullptr);

    CHECK(numbers[0].tryAsInt().value() == 1);
    CHECK(numbers[1].tryAsInt().value() == 2);
    CHECK(numbers[2].tryAsInt().value() == 3);

    const JValue* info = root.find("info");
    REQUIRE(info != nullptr);

    const std::string* name = info->find("name")->tryAsString();
    REQUIRE(name != nullptr);
    CHECK(*name == "Chad");
    CHECK(info->find("age")->tryAsInt().value() == 30);

    CHECK(root["pi"].tryAsDouble().value() == doctest::Approx(3.141592653589793));
    CHECK(root["big_number"].tryAsLong().value() == 9223372036854775807LL);
    CHECK(root["small_number"].tryAsShort().value() == -32768);
    CHECK(root["float_val"].tryAsFloat().value() == doctest::Approx(2.71828f));

    const JValue* skills = info->find("skills");
    REQUIRE(skills != nullptr);
    CHECK(*skills[0].tryAsString() == "C++");
    CHECK(*skills[1].tryAsString() == "Python");
}

TEST_CASE("providers/json.h: from_file") {
    fs::path rssPath = fs::absolute("resources");
    fs::path fullPath = rssPath / "sample_min.json";
    std::cout << fullPath << "\n";

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::optional<JValue> root = Json::fromFile(fullPath);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::cout << "Took: " << format_duration(end - start) << 'ms.\n';
    CHECK(root.has_value());
}
