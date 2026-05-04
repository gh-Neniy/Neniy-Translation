#include "TranslateText.hpp"

using namespace std::string_view_literals;

namespace {
  void TranslateUnit(NodeView& node_view, const TextUnit& unit) {
    node_view.Append("text:\""sv, node_view.Extract(unit.source), "\""sv);
    
    if (unit.color.start <= unit.color.end) {
      node_view.Append(",color:\""sv, node_view.Extract(unit.color), "\""sv);
    }
    
    node_view.Append(",italic:"sv, unit.italic ? "true"sv : "false"sv);

    if (unit.bold) {
      node_view.Append(",bold:true");
    }

    if (unit.hieroglyph) {
      node_view.Append(",font:\"minecraft:alt\"");
    }
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
