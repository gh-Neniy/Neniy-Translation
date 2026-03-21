#pragma once

#include "synt/Data.hpp"

#include <string>

std::string TranslateData(const std::vector<DataUnit>& units, std::string_view source_code, bool item_data);
