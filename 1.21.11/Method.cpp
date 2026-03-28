#include "lexic/Token.hpp"
#include "trans/Translate.hpp"
#include "Aux.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"
#include "TranslateText.hpp"
#include "Method.hpp"

#include <format>

namespace {
  std::string TranslateSelector(const NodeView& node_view) {
    return TranslateSelector(node_view.get<SelectorNode*>()->selector, node_view.Source());
  }

  std::string TranslateEntitySubcommand(const NodeView& node_view) {
    if (node_view.empty()) { // with selector
      return TranslateSelector(node_view);
    }
    
    return static_cast<std::string>(node_view.Extract(0));
  }

  std::string TranslateExecuteAlign(const NodeView& node_view) {
    return std::format (
      "align {}",
      node_view.Extract(0)
    );
  }

  std::string TranslateExecuteAs(const NodeView& node_view) {
    return std::format (
      "as {}",
      TranslateEntitySubcommand(node_view)
    );
  }

  std::string TranslateExecuteAt(const NodeView& node_view) {
    return std::format (
      "at {}",
      TranslateEntitySubcommand(node_view)
    );
  }

  std::string TranslateExecuteBlock(const NodeView& node_view) {
    return std::format (
      "{} block {} {} {} {}",
      node_view.Type() == CommandType::ExecuteIfBlock ? "if" : "unless",
      node_view.Extract(0), // x
      node_view.Extract(1), // y
      node_view.Extract(2), // z
      node_view.Extract(3)  // block name
    );
  }

  std::string TranslateExecuteEntity(const NodeView& node_view) {
    return std::format (
      "{} entity {}",
      node_view.Type() == CommandType::ExecuteIfEntity ? "if" : "unless",
      TranslateEntitySubcommand(node_view)
    );
  }

  std::string TranslateExecuteScore(const NodeView& node_view) {
    std::string result = std::format("{} score ", node_view.Type() == CommandType::ExecuteIfScore ? "if" : "unless");

    IndexType current_arg = 0;

    if (node_view.size() == 2) { // with selector
      result.append(TranslateSelector(node_view));
    } else { // == 3, with entity name
      result.append(node_view.Extract(current_arg));
      ++current_arg;
    }

    result.append(std::format(
      " {} matches {}",
      node_view.Extract(current_arg),     // objective name
      node_view.Extract(current_arg + 1)  // points range
    ));

    return result;
  }

  std::string TranslateExecutePositioned(const NodeView& node_view) {
    return std::format (
      "positioned {} {} {}",
      node_view.Extract(0),  // x
      node_view.Extract(1),  // y
      node_view.Extract(2)   // z
    );
  }

  std::string TranslateExecuteStoreScore(const NodeView& node_view) {
    std::string result = "store result score ";

    IndexType current_arg = 0;

    if (node_view.size() == 1) {
      result.append(TranslateSelector(node_view));
    } else { // == 2, with entity name
      result.append(node_view.Extract(current_arg));
      ++current_arg;
    }

    result.push_back(' ');
    result.append(node_view.Extract(current_arg));

    return result;
  }

  std::string TranslateExecuteStoreEntity(const NodeView& node_view) {
    std::string result = "store result entity ";

    IndexType current_arg = 0;

    if (node_view.size() == 3) { // without selector
      result.append(TranslateSelector(node_view));
    } else { // == 4, with entity name
      result.append(node_view.Extract(current_arg));
      ++current_arg;
    }

    result.append(std::format(
      " {} {} {}",
      node_view.Extract(current_arg),
      node_view.Extract(current_arg + 1),
      node_view.Extract(current_arg + 2)
    ));

    return result;
  }

  std::string TranslateExecuteUninited(const NodeView& node_view) {
    std::string result = "unless score ";

    IndexType current_arg = 0;
    std::string entity;

    if (node_view.size() == 1) { // with selector
      entity = TranslateSelector(node_view);
    } else { // == 2, with entity name
      entity = node_view.Extract(current_arg);
      ++current_arg;
    }

    std::string_view objective = node_view.Extract(current_arg);

    result.append(std::format(
      "{} {} = {} {}",
      entity, objective,
      entity, objective
    ));

    return result;
  }
}

std::string TranslateClone(const NodeView& node_view) {
  return std::format (
    "clone {} {} {} {} {} {} {} {} {} {}",
    node_view.Extract(0),  // start x
    node_view.Extract(1),  // start y
    node_view.Extract(2),  // start z
    node_view.Extract(3),  // end x
    node_view.Extract(4),  // end y
    node_view.Extract(5),  // end z
    node_view.Extract(6),  // from x
    node_view.Extract(7),  // from y
    node_view.Extract(8),  // from z
    node_view.Extract(9)   // mode
  );
}

std::string TranslateData(const NodeView& node_view) {
  std::string_view mode = node_view.Type() == CommandType::DataGet ? "get" : "modify";
  std::string result = std::format("data {} entity ", mode);
  IndexType current_arg = 0;

  if (node_view.size() == (mode == "get" ? 2 : 4)) {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else {
    result.append(TranslateSelector(node_view));
  }

  result.append(std::format(" {}", node_view.Extract(current_arg))); // data-field
  if (mode == "get") {
    return result;
  }
  
  result.append(std::format (
    " {} value {}",
    node_view.Extract(current_arg + 1), // mode
    node_view.Extract(current_arg + 2)  // value
  ));

  return result;
}

std::string TranslateExecute(const NodeView& node_view, std::string_view function_prefix, const Loader& loader) {
  std::string result = "execute";

  auto execute_node_ptr = node_view.get<ExecuteNode*>();

  for (int i = 0; i < execute_node_ptr->subnodes.size(); ++i) {
    NodeView subnode_view(execute_node_ptr->subnodes[i], node_view.Source());

    result.push_back(' ');
    switch (subnode_view.Type()) {
      case CommandType::ExecuteAlign:
        result.append(TranslateExecuteAlign(subnode_view));
        break;
      case CommandType::ExecuteAs:
        result.append(TranslateExecuteAs(subnode_view));
        break;
      case CommandType::ExecuteAt:
        result.append(TranslateExecuteAt(subnode_view));
        break;
      case CommandType::ExecuteIfBlock:
        result.append(TranslateExecuteBlock(subnode_view));
        break;
      case CommandType::ExecuteIfEntity:
        result.append(TranslateExecuteEntity(subnode_view));
        break;
      case CommandType::ExecuteIfScore:
        result.append(TranslateExecuteScore(subnode_view));
        break;
      case CommandType::ExecutePositioned:
        result.append(TranslateExecutePositioned(subnode_view));
        break;
      case CommandType::ExecuteStoreEntity:
        result.append(TranslateExecuteStoreEntity(subnode_view));
        break;
      case CommandType::ExecuteStoreScore:
        result.append(TranslateExecuteStoreScore(subnode_view));
        break;
      case CommandType::ExecuteUninited:
        result.append(TranslateExecuteUninited(subnode_view));
        break;
      case CommandType::ExecuteUnlessBlock:
        result.append(TranslateExecuteBlock(subnode_view));
        break;
      case CommandType::ExecuteUnlessEntity:
        result.append(TranslateExecuteEntity(subnode_view));
        break;
      case CommandType::ExecuteUnlessScore:
        result.append(TranslateExecuteScore(subnode_view));
        break;
      default:
        throw std::logic_error("Internal translation error - unknown execute subcommand type");
    }
  }

  NodeView main_view(execute_node_ptr->main_node, node_view.Source());
  result.append(std::format (
    " run {}",
    ChooseTranslate(main_view, function_prefix, loader))
  );

  return result;
}

std::string TranslateFill(const NodeView& node_view) {
  std::string result = std::format (
    "fill {} {} {} {} {} {} {} {}",
    node_view.Extract(0),  // start x
    node_view.Extract(1),  // start y
    node_view.Extract(2),  // start z
    node_view.Extract(3),  // end x
    node_view.Extract(4),  // end y
    node_view.Extract(5),  // end z
    node_view.Extract(6),  // block
    node_view.Extract(7)   // mode
  );

  if (node_view.size() == 9) {
    result.append(std::format(" {}", node_view.Extract(8)));
  }

  return result;
}

std::string TranslateFunction(const NodeView& node_view, std::string_view function_prefix) {
  std::string result = "function ";

  if (!function_prefix.empty()) {
    result.append(std::format("{}:", function_prefix));
  }

  result.append(node_view.Extract(0));

  return result;
}

std::string TranslateGamemode(const NodeView& node_view) {
  std::string result = std::format (
    "gamemode {} ",
    node_view.Extract(0) // mode
  );

  if (node_view.size() == 2) {
    result.append(node_view.Extract(1)); // player name
  } else {
    result.append(TranslateSelector(node_view));
  }

  return result;
}

std::string TranslateGive(const NodeView& node_view) {
  std::string result = std::format (
    "give {}",
    TranslateEntitySubcommand(node_view)
  );

  auto genuine_node_ptr = node_view.get<SelectorIdWithDataPtrNode*>();
  result.push_back(' ');
  result.append(Extract(node_view.Source(), genuine_node_ptr->id_with_data_ptr->identifier));

  if (!genuine_node_ptr->id_with_data_ptr->units.empty()) {
    result.append(std::format("[{}]", TranslateData(genuine_node_ptr->id_with_data_ptr->units, node_view.Source(), true)));
  }

  return result;
}

std::string TranslateKill(const NodeView& node_view) {
  return std::format (
    "kill {}",
    TranslateEntitySubcommand(node_view)
  );
}

std::string TranslatePlaysound(const NodeView& node_view) {
  IndexType current_arg = 0;

  std::string result = std::format(
    "playsound {} neutral ",
    node_view.Extract(current_arg) // sound id
  );

  ++current_arg;
  if (node_view.size() == 6) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 7, with entity name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.append(std::format (
    " {} {} {} {} {}",
    node_view.Extract(current_arg),      // x
    node_view.Extract(current_arg + 1),  // y
    node_view.Extract(current_arg + 2),  // z
    node_view.Extract(current_arg + 3),  // volume
    node_view.Extract(current_arg + 4)   // pitch
  ));

  return result;
}

std::string TranslateSay(const NodeView& node_view) {
  return std::format (
    "say {}",
    node_view.Extract(0)
  );
}

std::string TranslateScoreboardObjectivesAdd(const NodeView& node_view) {
  std::string result = std::format (
    "scoreboard objectives add {} {}",
    node_view.Extract(0),  // objective
    node_view.Extract(1)   // objective type
  );

  auto genuine_node_ptr = node_view.get<TextNode*>();

  if (genuine_node_ptr->text.units.empty()) {
    return result;
  }

  result.push_back(' ');
  result.append(TranslateText(genuine_node_ptr->text, node_view.Source())); // objective name

  return result;
}

std::string TranslateScoreboardObjectivesSet(const NodeView& node_view) {
  return std::format (
    "scoreboard objectives setdisplay {} {}",
    node_view.Extract(0), // mode
    node_view.Extract(1)  // objective
  );
}

std::string TranslateScoreboardPlayers(const NodeView& node_view) {
  std::string result = std::format (
    "scoreboard players {}",
    node_view.Extract(0) // mode
  );

  IndexType current_argument = 1;

  result.push_back(' ');
  if (node_view.size() == 3) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 4, with entity name
    result.append(node_view.Extract(current_argument));
    ++current_argument;
  }

  result.append(std::format (
    " {} {}",
    node_view.Extract(current_argument),    // objective
    node_view.Extract(current_argument + 1) // points
  ));

  return result;
}

std::string TranslateSetblock(const NodeView& node_view) {
  return std::format (
    "setblock {} {} {} {} {}",
    node_view.Extract(0), // block
    node_view.Extract(1), // x
    node_view.Extract(2), // y
    node_view.Extract(3), // z
    node_view.Extract(4)  // mode
  );
}

std::string TranslateSummon(const NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;

  std::string result = std::format (
    "summon {} {} {} {}",
    Extract(node_view.Source(), id_with_data_ptr->identifier), // entity name
    node_view.Extract(0), // x
    node_view.Extract(1), // y
    node_view.Extract(2)  // z
  );

  if (!id_with_data_ptr->units.empty()) {
    result.push_back(' ');
    result.append(std::format("{{{}}}", TranslateData(id_with_data_ptr->units, node_view.Source(), false)));
  }

  return result;
}

std::string TranslateTellraw(const NodeView& node_view) {
  std::string result = std::format (
    "tellraw {}",
    TranslateEntitySubcommand(node_view)
  );

  result.push_back(' ');
  result.append(TranslateText(node_view.get<SelectorTextNode*>()->text, node_view.Source()));

  return result;
}

std::string TranslateTp(const NodeView& node_view) {
  std::string result = "tp ";

  auto raw_ptr = node_view.get<DoubleSelectorNode*>();

  switch (node_view.size()) {
    case 0: // one or two selectors
      result.append(TranslateSelector(node_view));

      if (raw_ptr->second_selector.type != TokenType::Identifier) {
        result.push_back(' ');
        result.append(TranslateSelector(raw_ptr->second_selector, node_view.Source()));
      }
      
      return result;
    case 1: // one argument is a entity name
      if (raw_ptr->selector.type == TokenType::Identifier) {
        result.append(node_view.Extract(0));

        if (raw_ptr->second_selector.type != TokenType::Identifier) {
          result.push_back(' ');
          result.append(TranslateSelector(raw_ptr->second_selector, node_view.Source()));
        }
      } else {
        result.append(std::format("{} {}", TranslateSelector(node_view), node_view.Extract(0)));
      }

      return result;
    case 2: // both arguments are entity names
      result.append(std::format(
        "{} {}",
        node_view.Extract(0),
        node_view.Extract(1)
      ));  

      return result;
    case 3: // only coordinates
      if (raw_ptr->selector.type != TokenType::Identifier) {
        result.append(TranslateSelector(node_view));
        result.push_back(' ');
      }

      result.append(std::format (
        "{} {} {}",
        node_view.Extract(0),
        node_view.Extract(1),
        node_view.Extract(2)
      ));

      return result;
    case 4: // one entity name and coordinates
      result.append(std::format(
        "{} {} {} {}",
        node_view.Extract(0),
        node_view.Extract(1),
        node_view.Extract(2),
        node_view.Extract(3)
      ));

      return result;
    case 5: // only coordinates
      if (raw_ptr->selector.type != TokenType::Identifier) {
        result.append(TranslateSelector(node_view));
        result.push_back(' ');
      }

      result.append(std::format(
        "{} {} {} {} {}",
        node_view.Extract(0),
        node_view.Extract(1),
        node_view.Extract(2),
        node_view.Extract(3),
        node_view.Extract(4)
      ));

      return result;
    case 6: // one entity name and 5 coordinates
      result.append(std::format(
        "{} {} {} {} {} {}",
        node_view.Extract(0),
        node_view.Extract(1),
        node_view.Extract(2),
        node_view.Extract(3),
        node_view.Extract(4),
        node_view.Extract(5)
      ));

      return result;
    default:
      throw std::logic_error("Internal translation error - imposible case reached in TranslateTp()");
  }
}
