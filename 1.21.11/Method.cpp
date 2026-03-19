#include "TranslateData.hpp"
#include "TranslateSelector.hpp"
#include "TranslateText.hpp"
#include "Method.hpp"

#include <format>

std::string TranslateFunction(const NodePtr& node_ptr, std::string_view text, std::string_view function_prefix) {
  auto id = node_ptr->args[0];

  return std::format (
    "function {}:{}",
    function_prefix,
    text.substr(id.start, id.end - id.start + 1)
  );
}

std::string TranslateGamemode(const NodePtr& node_ptr, std::string_view text) {
  auto mode = node_ptr->args[0];

  std::string result = "gamemode ";
  result.append(text.substr(mode.start, mode.end - mode.start + 1));
  result.push_back(' ');

  if (node_ptr->args.size() == 2) {
    auto player = node_ptr->args[1];
    result.append(text.substr(player.start, player.end - player.start + 1));
  } else {
    result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, text));
  }

  return result;
}

std::string TranslateGive(const NodePtr& node_ptr, std::string_view text) {
  std::string result = "give ";
  result.append(TranslateSelector(static_cast<SelectorIdWithDataPtrNode*>(node_ptr.get())->selector, text));
  result.push_back(' ');

  const auto& id_with_data_ptr = static_cast<SelectorIdWithDataPtrNode*>(node_ptr.get())->id_with_data_ptr;
  const auto& id = id_with_data_ptr->identifier;
  result.append(text.substr(id.start, id.end - id.start + 1));
  result.append(std::format("[{}]", TranslateData(id_with_data_ptr->units, text, true)));

  return result;
}

std::string TranslatePlaysound(const NodePtr& node_ptr, std::string_view text) {
  std::string result = "playsound ";

  auto id = node_ptr->args[0];
  result.append(text.substr(id.start, id.end - id.start + 1));

  result.append(" neutral ");
  result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, text));
  for (int i = 1; i < node_ptr->args.size(); ++i) {
    auto coord = node_ptr->args[i];

    result.push_back(' ');
    result.append(text.substr(coord.start, coord.end - coord.start + 1));
  }

  return result;
}

std::string TranslateSay(const NodePtr& node_ptr, std::string_view text) {
  auto msg = node_ptr->args[0];

  return std::format (
    "say {}",
    text.substr(msg.start, msg.end - msg.start + 1)
  );
}

std::string TranslateSummon(const NodePtr& node_ptr, std::string_view text) {
  std::string result = "summon ";

  const auto& id_with_data_ptr = static_cast<IdWithDataPtrNode*>(node_ptr.get())->id_with_data_ptr;
  auto id = id_with_data_ptr->identifier;

  result.append(text.substr(id.start, id.end - id.start + 1));

  for (int i = 0; i < node_ptr->args.size(); ++i) {
    auto coord = node_ptr->args[i];

    result.append(std::format(" {}", text.substr(coord.start, coord.end - coord.start + 1)));
  }

  if (!id_with_data_ptr->units.empty()) {
    result.push_back(' ');
    result.append(std::format("{{{}}}", TranslateData(id_with_data_ptr->units, text, false)));
  }

  return result;
}

std::string TranslateTellraw(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = "tellraw ";
  auto selector_text_node_ptr = static_cast<SelectorTextNode*>(node_ptr.get());

  if (node_ptr->args.empty()) {
    result.append(TranslateSelector(selector_text_node_ptr->selector, source_code));
  } else {
    auto player = node_ptr->args[0];

    result.append(source_code.substr(player.start, player.end - player.start + 1));
  }

  result.push_back(' ');
  result.append(TranslateText(selector_text_node_ptr->text, source_code));

  return result;
}
