#include "synt/Aux.hpp"
#include "TranslateText.hpp"

#include <format>

namespace {
  std::string TranslateUnit(const TextUnit& unit, std::string_view source_code) {
    std::string result = std::format (
      "text:\"{}\"",
      Extract(source_code, unit.source)
    );
    
    if (unit.color.start <= unit.color.end) {
      result.append(std::format (
        ",color:{}",
        Extract(source_code, unit.color)
      ));
    }

    if (unit.italic) {
      result.append(",italic:true");
    }
    if (unit.bold) {
      result.append(",bold:true");
    }
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
