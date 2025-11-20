#pragma once

#include <string>

#include "serris/config.h"
#include "serris/types/jvalue.h"

namespace serris::providers {
    class json {
      public:
        SERRIS_API static std::string stringify(const types::jvalue&, bool pretty = false);

      private:
        static std::string stringify_compact(const types::jvalue&);
        static std::string stringify_pretty(const types::jvalue&, int indent);

        static std::string escape(const std::string&);
    };
}