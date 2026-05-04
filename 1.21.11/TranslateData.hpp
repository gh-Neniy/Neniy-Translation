#pragma once

#include "synt/Data.hpp"
#include "trans/NodeView.hpp"

// All functions write data without braces
// Units are passed because of ambiguity

// std::vector<Text> for sign data
std::vector<Text> TranslateBlockData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator);

void TranslateEntityData(NodeView& node_view, const std::vector<DataUnit>& units);

void TranslateItemData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator);

void TranslateParticleData(NodeView& node_view, const std::vector<DataUnit>& units);
