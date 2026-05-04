#pragma once

#include "synt/Data.hpp"
#include "trans/NodeView.hpp"

// units are passed because of ambiguity

void TranslateBlockData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator);

void TranslateEntityData(NodeView& node_view, const std::vector<DataUnit>& units);

// item data without braces
void TranslateItemData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator);

void TranslateParticleData(NodeView& node_view, const std::vector<DataUnit>& units);
