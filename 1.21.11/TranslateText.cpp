#include "main/Concat.hpp"
#include "Aux.hpp"
#include "TranslateText.hpp"

using namespace std::literals;

namespace {
  std::string TranslateUnit(const TextUnit& unit, std::string_view source_code) {
    std::string result = Concat(
      "text:\""sv,
      Extract(source_code, unit.source),
      "\""sv
    );
    
    if (unit.color.start <= unit.color.end) {
      result.append(Concat(
        ",color:\""sv, Extract(source_code, unit.color),
        "\",italic:"sv, unit.italic ? "true"sv : "false"sv
      ));
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

  std::string result = "{";
  result.append(TranslateUnit(text.units[0], source_code));
  
  if (text.units.size() == 1) {
    result.push_back('}');
    return result;
  }

  result.append(",extra:[");
  
  for (int i = 1; i < text.units.size(); ++i) {
    if (i > 1) {
      result.push_back(',');
    }

    result.push_back('{');
    result.append(TranslateUnit(text.units[i], source_code));
    result.push_back('}');
  }

  result.append("]}");
  return result;
}
