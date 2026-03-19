#pragma once

#include "synt/Node.hpp"

#include <string>

extern "C" {
  std::string TranslateFunction(const NodePtr& node_ptr, std::string_view text, std::string_view function_prefix);

  std::string TranslateGamemode(const NodePtr& node_ptr, std::string_view text);

  std::string TranslateGive(const NodePtr& node_ptr, std::string_view text);

  std::string TranslatePlaysound(const NodePtr& node_ptr, std::string_view text);

  std::string TranslateSay(const NodePtr& node_ptr, std::string_view text);

  std::string TranslateSummon(const NodePtr& node_ptr, std::string_view text);

  std::string TranslateTellraw(const NodePtr& node_ptr, std::string_view text);
}
