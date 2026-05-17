#include "Aux.hpp"

void TranslateNumericList(NodeView& node_view, const ListType& list, std::string_view suffix) {
  node_view.PushBack('[');

  for (std::size_t i = 0; i < list.size(); ++i) {
    if (i > 0) {
      node_view.PushBack(',');
    }

    node_view.Append(node_view.Extract(list[i].key), suffix);
  }

  node_view.PushBack(']');
}
