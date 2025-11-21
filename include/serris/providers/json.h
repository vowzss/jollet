#pragma once

#include <string>

#include "serris/config.h"
#include "serris/types/jvalue.h"

using namespace serris::types;

namespace serris::providers {
    class json {
      public:
        SERRIS_API static std::string serialize(const jvalue&, bool pretty = false);
        SERRIS_API static jvalue deserialize(const std::string& str);

        SERRIS_API static std::optional<jvalue> from_file(const std::string& path);
        SERRIS_API static bool to_file();
    };
}