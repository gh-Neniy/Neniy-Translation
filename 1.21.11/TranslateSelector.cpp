#include "Aux.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"

namespace {
  std::string TranslateSelectorType(TokenType selector_type) {
    switch (selector_type) {
      case TokenType::AllSelector:
        return "@e";
      case TokenType::AllPlayerSelector:
        return "@a";
      case TokenType::CurrentSelector:
        return "@s";
      case TokenType::NearestPlayerSelector:
        return "@p";
      case TokenType::RandomPlayerSelector:
        return "@r";
      default:
        throw std::logic_error("Internal translation error - unknown selector type in TranslateSelector()");
    }
  }

  std::string TranslateSelectorUnitKey(TokenType key) {
    switch (key) {
      case TokenType::Distance:
        return "distance";
      case TokenType::Sort:
        return "sort";
      case TokenType::Tag:
        return "tag";
      case TokenType::Team:
        return "team";
      default:
        throw std::logic_error("Internal translation error - unknown selector unit key");
    }
  }

  std::string TranslateSelectorUnit(const SelectorUnit& unit, std::string_view source_code) {
    std::string result;
    result.append(TranslateSelectorUnitKey(unit.key));
    result.push_back('=');

    if (std::holds_alternative<BaseToken>(unit.value)) {
      auto value = std::get<BaseToken>(unit.value);
      result.append(Extract(source_code, value));
    } else {
      const auto& value = std::get<DataPtr>(unit.value);
      result.append(TranslateData(value->units, source_code, false));
    }

    return result;
  }
}

std::string TranslateSelector(const Selector& selector, std::string_view source_code) {
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

    result.append(TranslateSelectorUnit(selector.units[i], source_code));
  }

  result.push_back(']');
  return result;
}
