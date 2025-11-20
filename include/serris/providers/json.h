#pragma once

#include <string>

#include "serris/config.h"
#include "serris/types/jvalue.h"

namespace serris::providers {
    class json {
      public:
        SERRIS_API static std::string serialize(const types::jvalue&, bool pretty = false);
        SERRIS_API static types::jvalue deserialize(const std::string& str);
    };
}