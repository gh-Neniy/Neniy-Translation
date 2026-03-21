#include "lexic/Token.hpp"
#include "trans/Translate.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"
#include "TranslateText.hpp"
#include "Method.hpp"

#include <format>

namespace {
  std::string TranslateEntitySubcommand(const NodePtr& node_ptr, std::string_view source_code) {
    if (node_ptr->args.empty()) { // with selector
      return TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, source_code);
    }
    
    return static_cast<std::string>(Extract(source_code, node_ptr->args[0]));
  }

  std::string TranslateExecuteAs(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "as {}",
      TranslateEntitySubcommand(node_ptr, source_code)
    );
  }

  std::string TranslateExecuteAt(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "at {}",
      TranslateEntitySubcommand(node_ptr, source_code)
    );
  }

  std::string TranslateExecuteIfBlock(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "if block {} {} {} {}",
      Extract(source_code, node_ptr->args[0]),  // x
      Extract(source_code, node_ptr->args[1]),  // y
      Extract(source_code, node_ptr->args[2]),  // z
      Extract(source_code, node_ptr->args[3])   // block name
    );
  }

  std::string TranslateExecuteIfEntity(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "if entity {}",
      TranslateEntitySubcommand(node_ptr, source_code)
    );
  }

  std::string TranslateExecuteIfScore(const NodePtr& node_ptr, std::string_view source_code) {
    // if score @a data 2
    std::string result = "if score ";

    IndexType current_arg = 0;

    if (node_ptr->args.size() == 2) { // with selector
      result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, source_code));
    } else { // == 3, with entity name
      result.append(Extract(source_code, node_ptr->args[0]));
      ++current_arg;
    }

    result.append(std::format(
      " {} matches {}",
      Extract(source_code, node_ptr->args[current_arg]),    // objective name
      Extract(source_code, node_ptr->args[current_arg + 1]) // points range
    ));

    return result;
  }

  std::string TranslateExecutePositioned(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "positioned {} {} {}",
      Extract(source_code, node_ptr->args[0]),  // x
      Extract(source_code, node_ptr->args[1]),  // y
      Extract(source_code, node_ptr->args[2])   // z
    );
  }
}

std::string TranslateClone(const NodePtr& node_ptr, std::string_view source_code) {
  return std::format (
    "clone {} {} {} {} {} {} {} {} {} {}",
    Extract(source_code, node_ptr->args[0]),  // start x
    Extract(source_code, node_ptr->args[1]),  // start y
    Extract(source_code, node_ptr->args[2]),  // start z
    Extract(source_code, node_ptr->args[3]),  // end x
    Extract(source_code, node_ptr->args[4]),  // end y
    Extract(source_code, node_ptr->args[5]),  // end z
    Extract(source_code, node_ptr->args[6]),  // from x
    Extract(source_code, node_ptr->args[7]),  // from y
    Extract(source_code, node_ptr->args[8]),  // from z
    Extract(source_code, node_ptr->args[9])   // mode
  );
}

std::string TranslateExecute(const NodePtr& node_ptr, std::string_view source_code, std::string_view function_prefix,
                             const Loader& loader) {
  std::string result = "execute";

  auto execute_node_ptr = static_cast<ExecuteNode*>(node_ptr.get());

  for (int i = 0; i < execute_node_ptr->subnodes.size(); ++i) {
    const auto& subnode = execute_node_ptr->subnodes[i];

    result.push_back(' ');
    switch (subnode->command_type) {
      case CommandType::ExecuteAs:
        result.append(TranslateExecuteAs(subnode, source_code));
        break;
      case CommandType::ExecuteAt:
        result.append(TranslateExecuteAt(subnode, source_code));
        break;
      case CommandType::ExecuteIfBlock:
        result.append(TranslateExecuteIfBlock(subnode, source_code));
        break;
      case CommandType::ExecuteIfEntity:
        result.append(TranslateExecuteIfEntity(subnode, source_code));
        break;
      case CommandType::ExecuteIfScore:
        result.append(TranslateExecuteIfScore(subnode, source_code));
        break;
      case CommandType::ExecutePositioned:
        result.append(TranslateExecutePositioned(subnode, source_code));
        break;
      default:
        throw std::logic_error("Internal translation error - unknown execute subcommand type");
    }
  }

  result.append(std::format (
    " run {}",
    ChooseTranslate(execute_node_ptr->main_node, source_code, function_prefix, loader))
  );

  return result;
}

std::string TranslateFill(const NodePtr& node_ptr, std::string_view source_code) {
  return std::format(
    "fill {} {} {} {} {} {} {} {}",
    Extract(source_code, node_ptr->args[0]),  // start x
    Extract(source_code, node_ptr->args[1]),  // start y
    Extract(source_code, node_ptr->args[2]),  // start z
    Extract(source_code, node_ptr->args[3]),  // end x
    Extract(source_code, node_ptr->args[4]),  // end y
    Extract(source_code, node_ptr->args[5]),  // end z
    Extract(source_code, node_ptr->args[6]),  // block
    Extract(source_code, node_ptr->args[7])   // mode
  );
}

std::string TranslateFunction(const NodePtr& node_ptr, std::string_view source_code, std::string_view function_prefix) {
  return std::format (
    "function {}:{}",
    function_prefix,
    Extract(source_code, node_ptr->args[0]) // function name
  );
}

std::string TranslateGamemode(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = std::format (
    "gamemode {} ",
    Extract(source_code, node_ptr->args[0]) // mode
  );

  if (node_ptr->args.size() == 2) {
    result.append(Extract(source_code, node_ptr->args[1])); // player name
  } else {
    result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, source_code));
  }

  return result;
}

std::string TranslateGive(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = "give ";

  auto genuine_node_ptr = static_cast<SelectorIdWithDataPtrNode*>(node_ptr.get());

  if (node_ptr->args.size() == 1) {
    result.append(Extract(source_code, node_ptr->args[0]));
  } else {
    result.append(TranslateSelector(genuine_node_ptr->selector, source_code));
  }

  result.push_back(' ');

  result.append(Extract(source_code, genuine_node_ptr->id_with_data_ptr->identifier));
  result.append(std::format("[{}]", TranslateData(genuine_node_ptr->id_with_data_ptr->units, source_code, true)));

  return result;
}

std::string TranslatePlaysound(const NodePtr& node_ptr, std::string_view source_code) {
  IndexType current_arg = 0;

  std::string result = std::format(
    "playsound {} neutral ",
    Extract(source_code, node_ptr->args[current_arg]) // sound id
  );

  ++current_arg;
  if (node_ptr->args.size() == 6) { // with selector
    result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, source_code));
  } else { // == 7, with entity name
    result.append(Extract(source_code, node_ptr->args[current_arg]));
    ++current_arg;
  }

  result.append(std::format (
    " {} {} {} {} {}",
    Extract(source_code, node_ptr->args[current_arg]),      // x
    Extract(source_code, node_ptr->args[current_arg + 1]),  // y
    Extract(source_code, node_ptr->args[current_arg + 2]),  // z
    Extract(source_code, node_ptr->args[current_arg + 3]),  // volume
    Extract(source_code, node_ptr->args[current_arg + 4])   // pitch
  ));

  return result;
}

std::string TranslateSay(const NodePtr& node_ptr, std::string_view source_code) {
  return std::format (
    "say {}",
    Extract(source_code, node_ptr->args[0])
  );
}

std::string TranslateScoreboardObjectives(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = std::format (
    "scoreboard objectives {} {}",
    Extract(source_code, node_ptr->args[0]),  // mode
    Extract(source_code, node_ptr->args[1])   // objective
  );

  auto genuine_node_ptr = static_cast<TextNode*>(node_ptr.get());

  if (genuine_node_ptr->text.units.empty()) {
    return result;
  }

  result.push_back(' ');
  result.append(TranslateText(genuine_node_ptr->text, source_code)); // objective name

  return result;
}

std::string TranslateScoreboardPlayers(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = std::format (
    "scoreboard players {}",
    Extract(source_code, node_ptr->args[0]) // mode
  );

  IndexType current_argument = 1;

  result.push_back(' ');
  if (node_ptr->args.size() == 3) { // with selector
    result.append(TranslateSelector(static_cast<SelectorNode*>(node_ptr.get())->selector, source_code));
  } else { // == 4, with entity name
    result.append(Extract(source_code, node_ptr->args[1]));
    ++current_argument;
  }

  result.append(std::format (
    " {} {}",
    Extract(source_code, node_ptr->args[current_argument]), // objective name
    Extract(source_code, node_ptr->args[current_argument + 1])  // points
  ));

  return result;
}

std::string TranslateSetblock(const NodePtr& node_ptr, std::string_view source_code) {
  return std::format (
    "setblock {} {} {} {} {}",
    Extract(source_code, node_ptr->args[0]),  // block
    Extract(source_code, node_ptr->args[1]),  // x
    Extract(source_code, node_ptr->args[2]),  // y
    Extract(source_code, node_ptr->args[3]),  // z
    Extract(source_code, node_ptr->args[4])  // mode
  );
}

std::string TranslateSummon(const NodePtr& node_ptr, std::string_view source_code) {
  const auto& id_with_data_ptr = static_cast<IdWithDataPtrNode*>(node_ptr.get())->id_with_data_ptr;

  std::string result = std::format (
    "summon {} {} {} {}",
    Extract(source_code, id_with_data_ptr->identifier), // entity name
    Extract(source_code, node_ptr->args[0]),            // x
    Extract(source_code, node_ptr->args[1]),            // y
    Extract(source_code, node_ptr->args[2])             // z
  );

  if (!id_with_data_ptr->units.empty()) {
    result.push_back(' ');
    result.append(std::format("{{{}}}", TranslateData(id_with_data_ptr->units, source_code, false)));
  }

  return result;
}

std::string TranslateTellraw(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = "tellraw ";
  auto selector_text_node_ptr = static_cast<SelectorTextNode*>(node_ptr.get());

  if (node_ptr->args.empty()) {
    result.append(TranslateSelector(selector_text_node_ptr->selector, source_code));
  } else {
    result.append(Extract(source_code, node_ptr->args[0])); // player name
  }

  result.push_back(' ');
  result.append(TranslateText(selector_text_node_ptr->text, source_code));

  return result;
}
