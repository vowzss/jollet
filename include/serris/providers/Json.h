#pragma once

#include <string>

#include "serris/Export.h"
#include "serris/types/JType.h"
#include "serris/types/JValue.h"

namespace serris {
    class Json {
      public:
        SERRIS_API static std::string serialize(const JValue&, bool pretty = false);
        SERRIS_API static JValue deserialize(const std::string& str);

        SERRIS_API static std::optional<JValue> fromFile(const std::string& path);
        SERRIS_API static bool toFile();
    };
}