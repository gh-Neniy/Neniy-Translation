#pragma once

#include "synt/Selector.hpp"
#include "trans/NodeView.hpp"

void TranslateSelector(NodeView& node_view, const Selector& selector);
