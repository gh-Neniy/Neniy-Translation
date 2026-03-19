#include "TranslateText.hpp"

#include <format>

namespace {
  std::string TranslateUnit(const TextUnit& unit, std::string_view source_code) {
    std::string result = std::format (
      "text:\"{}\",color:{},italic:{},bold:{}",
      source_code.substr(unit.source.start, unit.source.end - unit.source.start + 1),
      source_code.substr(unit.color.start, unit.color.end - unit.color.start + 1),
      unit.italic,
      unit.bold
    );

    if (unit.hieroglyph) {
      result.append(",font:\"minecraft:alt\"");
    }

    return result;
  }
}

std::string TranslateText(const Text& text, std::string_view source_code) {
  if (text.units.empty()) {
    return "{text:\"\"}";
  }

  std::string result = std::format("{{{}", TranslateUnit(text.units[0], source_code));
  
  if (text.units.size() == 1) {
    result.push_back('}');
    return result;
  }

  result.append(",extra:[");
  
  for (int i = 1; i < text.units.size(); ++i) {
    if (i > 1) {
      result.push_back(',');
    }

    result.append(std::format("{{{}}}", TranslateUnit(text.units[i], source_code)));
  }

  result.append("]}");
  return result;
}
