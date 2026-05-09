#include "TranslateText.hpp"

using namespace std::string_view_literals;

namespace {
  void TranslateUnit(NodeView& node_view, const TextUnit& unit) {
    node_view.Append("text:\""sv, node_view.Extract(unit.source), "\""sv);
    
    if (unit.color.start <= unit.color.end) {
      node_view.Append(",color:\""sv, node_view.Extract(unit.color), "\""sv);
    }
    
    node_view.Append(
      ",italic:"sv, unit.italic ? "true"sv : "false"sv,
      ",bold:"sv, unit.bold ? "true"sv : "false"sv,
      ",font:"sv, unit.hieroglyph ? "\"minecraft:alt\""sv : "\"minecraft:uniform\""sv
    );
  }
}

void TranslateText(NodeView& node_view, const Text& text) {
  if (text.units.empty()) {
    node_view.Append("{text:\"\"}");
    return;
  }

  node_view.PushBack('{');
  TranslateUnit(node_view, text.units[0]);
  
  if (text.units.size() == 1) {
    node_view.PushBack('}');
    return;
  }

  node_view.Append(",extra:[");
  
  for (int i = 1; i < text.units.size(); ++i) {
    if (i > 1) {
      node_view.PushBack(',');
    }

    node_view.PushBack('{');
    TranslateUnit(node_view, text.units[i]);
    node_view.PushBack('}');
  }

  node_view.Append("]}");
}

void TranslateLore(NodeView& node_view, const std::vector<Text>& lore) {
  node_view.PushBack('[');

  for (std::size_t i = 0; i < lore.size(); ++i) {
    if (i > 0) {
      node_view.PushBack(',');
    }

    TranslateText(node_view, lore[i]);
  }

  node_view.PushBack(']');
}
