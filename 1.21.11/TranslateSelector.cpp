#include "TranslateData.hpp"

using namespace std::string_view_literals;

namespace {
  void TranslateList(NodeView& node_view, const ListType& list) { // without braces
    for (std::size_t i = 0; i < list.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }
      
      node_view.Append(node_view.Extract(list[i].key), "="sv, node_view.Extract(list[i].value));
    }
  }

  void TranslateSelectorUnit(NodeView& node_view, const SelectorUnit& unit) {
    if (std::holds_alternative<BaseToken>(unit.value)) {
      std::string_view key = node_view.Extract(unit.key);

      if (key == "gm") {
        key = "gamemode";
      }

      node_view.Append(key, "="sv, node_view.Extract(std::get<BaseToken>(unit.value)));
    } else if (std::holds_alternative<DataPtr>(unit.value)) {
      const auto& units = std::get<DataPtr>(unit.value)->units;

      node_view.Append("nbt={");
      TranslateEntityData(node_view, units);
      node_view.PushBack('}');

    } else { // ListType, scores

      node_view.Append("scores={");
      TranslateList(node_view, std::get<ListType>(unit.value));
      node_view.PushBack('}');
    }
  }
}

void TranslateSelector(NodeView& node_view, const Selector& selector) {
  node_view.Append(node_view.Extract(selector.stem));

  if (selector.units.empty()) {
    return;
  }

  node_view.PushBack('[');

  for (std::size_t i = 0; i < selector.units.size(); ++i) {
    if (i > 0) {
      node_view.PushBack(',');
    }

    TranslateSelectorUnit(node_view, selector.units[i]);
  }

  node_view.PushBack(']');
}
