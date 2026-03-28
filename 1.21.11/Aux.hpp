#pragma once

#include "lexic/Token.hpp"

#include <string_view>

std::string_view Extract(std::string_view source_code, BaseToken token);
