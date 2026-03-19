#include "TranslateData.hpp"
#include "TranslateSelector.hpp"

namespace {
  std::string TranslateSelectorType(SelectorType selector_type) {
    switch (selector_type) {
      case SelectorType::All:
        return "@e";
      case SelectorType::AllPlayer:
        return "@a";
      case SelectorType::Current:
        return "@s";
      case SelectorType::NearestPlayer:
        return "@p";
      case SelectorType::RandomPlayer:
        return "@r";
      default:
        throw std::logic_error("Internal translation error - unknown selector type in TranslateSelector()");
    }
  }

  std::string TranslateSelectorUnitKey(TokenType key) {
    switch (key) {
      case TokenType::Distance:
        return "distance";
      case TokenType::Tag:
        return "tag";
      case TokenType::Team:
        return "team";
      default:
        throw std::runtime_error("Translation error - unknown selector unit key");
    }
  }

  std::string TranslateSelectorUnit(const SelectorUnit& unit, std::string_view text) {
    std::string result;
    result.append(TranslateSelectorUnitKey(unit.key));
    result.push_back('=');

    if (std::holds_alternative<BaseToken>(unit.value)) {
      auto value = std::get<BaseToken>(unit.value);
      result.append(text.substr(value.start, value.end - value.start + 1));
    } else {
      const auto& value = std::get<DataPtr>(unit.value);
      result.append(TranslateData(value->units, text, false));
    }

    return result;
  }
}

std::string TranslateSelector(const Selector& selector, std::string_view text) {
  std::string result;
  result.append(TranslateSelectorType(selector.type));

  if (selector.units.empty()) {
    return result;
  }

  result.push_back('[');

  for (int i = 0; i < selector.units.size(); ++i) {
    if (i > 0) {
      result.push_back(',');
    }

    result.append(TranslateSelectorUnit(selector.units[i], text));
  }

  result.push_back(']');
  return result;
}
