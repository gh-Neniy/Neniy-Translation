#include "Aux.hpp"
#include "main/Concat.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"

using namespace std::literals;

namespace {
  std::string TranslateList(const ListType& list, std::string_view source_code) { // without braces
    std::string result;

    for (int i = 0; i < list.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }
      
      result.append(Concat(Extract(source_code, list[i].key), "="sv, Extract(source_code, list[i].value)));
    }

    return result;
  }

  std::string TranslateSelectorUnit(const SelectorUnit& unit, std::string_view source_code) {
    std::string result;

    if (std::holds_alternative<BaseToken>(unit.value)) {
      std::string_view key = Extract(source_code, unit.key);

      if (key == "gm") {
        key = "gamemode";
      }

      result.append(key);
      result.push_back('=');

      auto value = std::get<BaseToken>(unit.value);
      result.append(Extract(source_code, value));
    } else if (std::holds_alternative<DataPtr>(unit.value)) {
      result.append("nbt=");

      const auto& value = std::get<DataPtr>(unit.value);
      result.push_back('{');
      result.append(TranslateEntityData(value->units, source_code)); // nbt={...}
      result.push_back('}');
    } else { // ListType, scores
      result.append(Concat("scores={"sv, Sv(TranslateList(std::get<ListType>(unit.value), source_code)), "}"sv));
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
