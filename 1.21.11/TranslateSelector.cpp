#include "Aux.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"

namespace {
  std::string TranslateSelectorUnit(const SelectorUnit& unit, std::string_view source_code) {
    std::string result;

    if (std::holds_alternative<BaseToken>(unit.value)) {
      result.append(Extract(source_code, unit.key));
      result.push_back('=');

      auto value = std::get<BaseToken>(unit.value);
      result.append(Extract(source_code, value));
    } else {
      result.append("nbt=");

      const auto& value = std::get<DataPtr>(unit.value);
      result.push_back('{');
      result.append(TranslateEntityData(value->units, source_code)); // nbt={...}
      result.push_back('}');
    }

    return result;
  }
}

std::string TranslateSelector(const Selector& selector, std::string_view source_code) {
  std::string result;
  result.append(Extract(source_code, selector.stem));

  if (selector.units.empty()) {
    return result;
  }

  result.push_back('[');

  for (int i = 0; i < selector.units.size(); ++i) {
    if (i > 0) {
      result.push_back(',');
    }

    result.append(TranslateSelectorUnit(selector.units[i], source_code));
  }

  result.push_back(']');
  return result;
}
