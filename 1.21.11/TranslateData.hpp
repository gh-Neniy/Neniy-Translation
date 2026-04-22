#pragma once

#include "synt/Data.hpp"

#include <string>

// All functions return data without braces

std::string TranslateBlockData(const std::vector<DataUnit>& units, std::string_view source_code, std::string_view separator);

std::string TranslateEntityData(const std::vector<DataUnit>& units, std::string_view source_code, bool is_summon);

std::string TranslateItemData(const std::vector<DataUnit>& units, std::string_view source_code, std::string_view separator);

std::string TranslateParticleData(const std::vector<DataUnit>& units, std::string_view source_code);
