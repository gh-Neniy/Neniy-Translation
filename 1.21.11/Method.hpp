#pragma once

#include "synt/Node.hpp"

#include <string>

extern "C" {
  std::string TranslateClone(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateDataGet(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateDataModify(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateExecute(const NodePtr& node_ptr, std::string_view source_code, std::string_view function_prefix,
                               const Loader& loader);

  std::string TranslateFill(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateFunction(const NodePtr& node_ptr, std::string_view source_code, std::string_view function_prefix);

  std::string TranslateGamemode(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateGive(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateKill(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslatePlaysound(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateSay(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateScoreboardObjectivesAdd(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateScoreboardObjectivesSet(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateScoreboardPlayers(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateSetblock(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateSummon(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateTellraw(const NodePtr& node_ptr, std::string_view source_code);

  std::string TranslateTp(const NodePtr& node_ptr, std::string_view source_code);
}
