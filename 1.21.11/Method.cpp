#include "lexic/Token.hpp"
#include "synt/Aux.hpp"
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

  std::string TranslateExecuteAlign(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "align {}",
      Extract(source_code, node_ptr->args[0])
    );
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

  std::string TranslateExecuteBlock(const NodePtr& node_ptr, std::string_view source_code, std::string_view condition) {
    return std::format (
      "{} block {} {} {} {}",
      condition,
      Extract(source_code, node_ptr->args[0]),  // x
      Extract(source_code, node_ptr->args[1]),  // y
      Extract(source_code, node_ptr->args[2]),  // z
      Extract(source_code, node_ptr->args[3])   // block name
    );
  }

  std::string TranslateExecuteIfBlock(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteBlock(node_ptr, source_code, "if");
  }

  std::string TranslateExecuteEntity(const NodePtr& node_ptr, std::string_view source_code, std::string_view condition) {
    return std::format (
      "{} entity {}",
      condition,
      TranslateEntitySubcommand(node_ptr, source_code)
    );
  }

  std::string TranslateExecuteIfEntity(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteEntity(node_ptr, source_code, "if");
  }

  std::string TranslateExecuteScore(const NodePtr& node_ptr, std::string_view source_code, std::string_view condition) {
    std::string result = std::format("{} score ", condition);

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

  std::string TranslateExecuteIfScore(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteScore(node_ptr, source_code, "if");
  }

  std::string TranslateExecutePositioned(const NodePtr& node_ptr, std::string_view source_code) {
    return std::format (
      "positioned {} {} {}",
      Extract(source_code, node_ptr->args[0]),  // x
      Extract(source_code, node_ptr->args[1]),  // y
      Extract(source_code, node_ptr->args[2])   // z
    );
  }

  std::string TranslateExecuteStoreScore(const NodePtr& node_ptr, std::string_view source_code) {
    std::string result = "store result score ";

    auto genuine_node_ptr = static_cast<SelectorNode*>(node_ptr.get());
    IndexType current_arg = 0;

    if (node_ptr->args.size() == 1) {
      result.append(TranslateSelector(genuine_node_ptr->selector, source_code));
    } else { // == 2, with entity name
      result.append(Extract(source_code, node_ptr->args[current_arg]));
      ++current_arg;
    }

    result.push_back(' ');
    result.append(Extract(source_code, node_ptr->args[current_arg]));

    return result;
  }

  std::string TranslateExecuteStoreEntity(const NodePtr& node_ptr, std::string_view source_code) {
    std::string result = "store result entity ";
    // ex store ent {} Example.Field[0][1][2] float 0.1

    auto genuine_node_ptr = static_cast<SelectorNode*>(node_ptr.get());
    IndexType current_arg = 0;

    if (node_ptr->args.size() == 3) { // without selector
      result.append(TranslateSelector(genuine_node_ptr->selector, source_code));
    } else { // == 4, with entity name
      result.append(Extract(source_code, node_ptr->args[current_arg]));
      ++current_arg;
    }

    result.append(std::format(
      " {} {} {}",
      Extract(source_code, node_ptr->args[current_arg]),
      Extract(source_code, node_ptr->args[current_arg + 1]),
      Extract(source_code, node_ptr->args[current_arg + 2])
    ));

    return result;
  }

  std::string TranslateExecuteUninited(const NodePtr& node_ptr, std::string_view source_code) {
    std::string result = "unless score ";

    auto genuine_node_ptr = static_cast<SelectorNode*>(node_ptr.get());
    IndexType current_arg = 0;
    std::string entity;

    if (node_ptr->args.size() == 1) { // with selector
      entity = TranslateSelector(genuine_node_ptr->selector, source_code);
    } else { // == 2, with entity name
      entity = Extract(source_code, node_ptr->args[current_arg]);
      ++current_arg;
    }

    std::string_view objective = Extract(source_code, node_ptr->args[current_arg]);

    result.append(std::format(
      "{} {} = {} {}",
      entity, objective,
      entity, objective
    ));

    return result;
  }

  std::string TranslateExecuteUnlessBlock(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteBlock(node_ptr, source_code, "unless");
  }

  std::string TranslateExecuteUnlessEntity(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteEntity(node_ptr, source_code, "unless");
  }

  std::string TranslateExecuteUnlessScore(const NodePtr& node_ptr, std::string_view source_code) {
    return TranslateExecuteScore(node_ptr, source_code, "unless");
  }

  Result<std::string> TranslateDataShared(const NodePtr& node_ptr, std::string_view source_code, std::string_view mode) {
    std::string result = std::format("data {} entity ", mode);
    IndexType current_arg = 0;

    auto genuine_node_ptr = static_cast<SelectorNode*>(node_ptr.get());

    if (node_ptr->args.size() == (mode == "get" ? 2 : 4)) {
      result.append(Extract(source_code, node_ptr->args[current_arg]));
      ++current_arg;
    } else {
      result.append(TranslateSelector(genuine_node_ptr->selector, source_code));
    }

    result.append(std::format(" {}", Extract(source_code, genuine_node_ptr->args[current_arg])));
    return {result, current_arg};
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

std::string TranslateDataGet(const NodePtr& node_ptr, std::string_view source_code) {
  return TranslateDataShared(node_ptr, source_code, "get").data;
}

std::string TranslateDataModify(const NodePtr& node_ptr, std::string_view source_code) {
  auto translated_data_shared = TranslateDataShared(node_ptr, source_code, "modify");

  std::string result = translated_data_shared.data;
  IndexType current_arg = translated_data_shared.end + 1;

  result.append(std::format (
    " {} value {}",
    Extract(source_code, node_ptr->args[current_arg]),
    Extract(source_code, node_ptr->args[current_arg + 1])
  ));

  return result;  
}

std::string TranslateExecute(const NodePtr& node_ptr, std::string_view source_code, std::string_view function_prefix,
                             const Loader& loader) {
  std::string result = "execute";

  auto execute_node_ptr = static_cast<ExecuteNode*>(node_ptr.get());

  for (int i = 0; i < execute_node_ptr->subnodes.size(); ++i) {
    const auto& subnode = execute_node_ptr->subnodes[i];

    result.push_back(' ');
    switch (subnode->command_type) {
      case CommandType::ExecuteAlign:
        result.append(TranslateExecuteAlign(subnode, source_code));
        break;
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
      case CommandType::ExecuteStoreEntity:
        result.append(TranslateExecuteStoreEntity(subnode, source_code));
        break;
      case CommandType::ExecuteStoreScore:
        result.append(TranslateExecuteStoreScore(subnode, source_code));
        break;
      case CommandType::ExecuteUninited:
        result.append(TranslateExecuteUninited(subnode, source_code));
        break;
      case CommandType::ExecuteUnlessBlock:
        result.append(TranslateExecuteUnlessBlock(subnode, source_code));
        break;
      case CommandType::ExecuteUnlessEntity:
        result.append(TranslateExecuteUnlessEntity(subnode, source_code));
        break;
      case CommandType::ExecuteUnlessScore:
        result.append(TranslateExecuteUnlessScore(subnode, source_code));
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
  std::string result = "function ";

  if (!function_prefix.empty()) {
    result.append(std::format("{}:", function_prefix));
  }

  result.append(Extract(source_code, node_ptr->args[0]));

  return result;
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

  if (!genuine_node_ptr->id_with_data_ptr->units.empty()) {
    result.append(std::format("[{}]", TranslateData(genuine_node_ptr->id_with_data_ptr->units, source_code, true)));
  }

  return result;
}

std::string TranslateKill(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = "kill ";

  auto genuine_node_ptr = static_cast<SelectorNode*>(node_ptr.get());

  if (node_ptr->args.empty()) { // with selector
    result.append(TranslateSelector(genuine_node_ptr->selector, source_code));
  } else {
    result.append(Extract(source_code, node_ptr->args[0]));
  }

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

std::string TranslateScoreboardObjectivesAdd(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = std::format (
    "scoreboard objectives add {} {}",
    Extract(source_code, node_ptr->args[0]),  // objective
    Extract(source_code, node_ptr->args[1])   // objective type
  );

  auto genuine_node_ptr = static_cast<TextNode*>(node_ptr.get());

  if (genuine_node_ptr->text.units.empty()) {
    return result;
  }

  result.push_back(' ');
  result.append(TranslateText(genuine_node_ptr->text, source_code)); // objective name

  return result;
}

std::string TranslateScoreboardObjectivesSet(const NodePtr& node_ptr, std::string_view source_code) {
  return std::format (
    "scoreboard objectives setdisplay {} {}",
    Extract(source_code, node_ptr->args[0]),
    Extract(source_code, node_ptr->args[1])
  );
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

std::string TranslateTp(const NodePtr& node_ptr, std::string_view source_code) {
  std::string result = "tp ";

  auto genuine_node_ptr = static_cast<DoubleSelectorNode*>(node_ptr.get());

  switch (node_ptr->args.size()) {
    case 0:
      result.append(std::format(
        "{} {}",
        TranslateSelector(genuine_node_ptr->first_selector, source_code),
        TranslateSelector(genuine_node_ptr->second_selector, source_code)
      ));

      break;
    case 1: // only first argument can be selector, used like that
      result.append(std::format(
        "{} {}",
        TranslateSelector(genuine_node_ptr->first_selector, source_code),
        Extract(source_code, node_ptr->args[0])
      ));

      break;
    case 2:
      result.append(std::format(
        "{} {}",
        Extract(source_code, node_ptr->args[0]),
        Extract(source_code, node_ptr->args[1])
      ));  

      break;
    case 3:
      result.append(std::format(
        "{} {} {} {}",
        TranslateSelector(genuine_node_ptr->first_selector, source_code),
        Extract(source_code, node_ptr->args[0]),
        Extract(source_code, node_ptr->args[1]),
        Extract(source_code, node_ptr->args[2])
      ));

      break;
    case 4:
      result.append(std::format(
        "{} {} {} {}",
        Extract(source_code, node_ptr->args[0]),
        Extract(source_code, node_ptr->args[1]),
        Extract(source_code, node_ptr->args[2]),
        Extract(source_code, node_ptr->args[3])
      ));

      break;
    case 5: // only when rotation is passed, selector in first argument
      result.append(std::format(
        "{} {} {} {} {} {}",
        TranslateSelector(genuine_node_ptr->first_selector, source_code),
        Extract(source_code, node_ptr->args[0]),
        Extract(source_code, node_ptr->args[1]),
        Extract(source_code, node_ptr->args[2]),
        Extract(source_code, node_ptr->args[3]),
        Extract(source_code, node_ptr->args[4])
      ));

      break;
    default:
      throw std::logic_error("Internal translation error - imposible case reached in TranslateTp()");
  }

  return result;
}
