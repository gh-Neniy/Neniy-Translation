#pragma once

#include "synt/Text.hpp"
#include "trans/NodeView.hpp"

// Text is passed because of ambiguity of node type (TextNode or SelectorTextNode)

void TranslateText(NodeView& node_view, const Text& text);
