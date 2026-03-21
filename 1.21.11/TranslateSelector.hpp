#pragma once

#include "synt/Selector.hpp"

#include <string>

std::string TranslateSelector(const Selector& selector, std::string_view source_code);
