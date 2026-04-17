#include "main/Concat.hpp"
#include "lexic/Token.hpp"
#include "trans/Translate.hpp"
#include "Aux.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"
#include "TranslateText.hpp"
#include "Method.hpp"

using namespace std::literals;

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
    return Concat(
      "align "sv,
      node_view.Extract(0)
    );
  }

  std::string TranslateExecuteAs(const NodeView& node_view) {
    return Concat(
      "as "sv,
      Sv(TranslateEntitySubcommand(node_view))
    );
  }

  std::string TranslateExecuteAt(const NodeView& node_view) {
    return Concat(
      "at "sv,
      Sv(TranslateEntitySubcommand(node_view))
    );
  }

  std::string TranslateExecuteBlock(const NodeView& node_view) {
    return Concat(
      node_view.Type() == CommandType::ExecuteIfBlock ? "if"sv : "unless"sv,
      " block "sv,
      node_view.Extract(0), " "sv, // x
      node_view.Extract(1), " "sv, // y
      node_view.Extract(2), " "sv, // z
      node_view.Extract(3)         // block name
    );
  }

  std::string TranslateExecuteEntity(const NodeView& node_view) {
    return Concat(
      node_view.Type() == CommandType::ExecuteIfEntity ? "if"sv : "unless"sv,
      " entity "sv,
      Sv(TranslateEntitySubcommand(node_view))
    );
  }

  std::string TranslateExecuteScore(const NodeView& node_view) {
    std::string result = Concat(
      node_view.Type() == CommandType::ExecuteIfScore ? "if"sv : "unless"sv,
      " score "sv
    );

    IndexType current_arg = 0;

    if (node_view.size() == 2) { // with selector
      result.append(TranslateSelector(node_view));
    } else { // == 3, with entity name
      result.append(node_view.Extract(current_arg));
      ++current_arg;
    }

    result.append(Concat(
      " "sv, node_view.Extract(current_arg),          // objective name
      " matches "sv, node_view.Extract(current_arg + 1) // points range
    ));

    return result;
  }

  std::string TranslateExecutePositioned(const NodeView& node_view) {
    return Concat(
      "positioned "sv,
      node_view.Extract(0), " "sv, // x
      node_view.Extract(1), " "sv, // y
      node_view.Extract(2)         // z
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

    result.append(Concat(
      " "sv, node_view.Extract(current_arg),
      " "sv, node_view.Extract(current_arg + 1),
      " "sv, node_view.Extract(current_arg + 2)
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

    result.append(Concat(
      Sv(entity), " "sv, objective, " = "sv,
      Sv(entity), " "sv, objective
    ));

    return result;
  }
}

std::string TranslateClone(const NodeView& node_view) {
  return Concat(
    "clone "sv,
    node_view.Extract(0), " "sv, // start x
    node_view.Extract(1), " "sv, // start y
    node_view.Extract(2), " "sv, // start z
    node_view.Extract(3), " "sv, // end x
    node_view.Extract(4), " "sv, // end y
    node_view.Extract(5), " "sv, // end z
    node_view.Extract(6), " "sv, // from x
    node_view.Extract(7), " "sv, // from y
    node_view.Extract(8), " "sv, // from z
    node_view.Extract(9)         // mode
  );
}

std::string TranslateData(const NodeView& node_view) {
  std::string_view mode = node_view.Type() == CommandType::DataGet ? "get"sv : "modify"sv;
  std::string result = Concat("data "sv, mode, " entity "sv);
  IndexType current_arg = 0;

  if (node_view.size() == (mode == "get"sv ? 2 : 4)) {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else {
    result.append(TranslateSelector(node_view));
  }

  result.append(Concat(" "sv, node_view.Extract(current_arg))); // data-field
  if (mode == "get"sv) {
    return result;
  }
  
  result.append(Concat(
    " "sv, node_view.Extract(current_arg + 1),      // mode
    " value "sv, node_view.Extract(current_arg + 2) // value
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
  result.append(Concat(
    " run "sv,
    Sv(ChooseTranslate(main_view, function_prefix, loader))
  ));

  return result;
}

std::string TranslateFill(const NodeView& node_view) {
  std::string result = Concat(
    "fill "sv,
    node_view.Extract(0), " "sv, // start x
    node_view.Extract(1), " "sv, // start y
    node_view.Extract(2), " "sv, // start z
    node_view.Extract(3), " "sv, // end x
    node_view.Extract(4), " "sv, // end y
    node_view.Extract(5), " "sv, // end z
    node_view.Extract(6), " "sv, // block
    node_view.Extract(7)         // mode
  );

  if (node_view.size() == 9) {
    result.append(Concat(" "sv, node_view.Extract(8)));
  }

  return result;
}

std::string TranslateFunction(const NodeView& node_view, std::string_view function_prefix) {
  std::string result = "function ";

  if (!function_prefix.empty()) {
    result.append(Concat(function_prefix, ":"sv));
  }

  result.append(node_view.Extract(0));

  return result;
}

std::string TranslateGamemode(const NodeView& node_view) {
  std::string result = Concat(
    "gamemode "sv,
    node_view.Extract(0), // mode
    " "sv
  );

  if (node_view.size() == 2) {
    result.append(node_view.Extract(1)); // player name
  } else {
    result.append(TranslateSelector(node_view));
  }

  return result;
}

std::string TranslateGamerule(const NodeView& node_view) {
  return Concat(
    "gamerule "sv,
    node_view.Extract(0), " "sv,
    node_view.Extract(1)
  );
}

std::string TranslateGive(const NodeView& node_view) {
  std::string result = Concat(
    "give "sv,
    Sv(TranslateEntitySubcommand(node_view))
  );

  auto genuine_node_ptr = node_view.get<SelectorIdWithDataPtrNode*>();
  result.push_back(' ');
  result.append(Extract(node_view.Source(), genuine_node_ptr->id_with_data_ptr->identifier));

  if (!genuine_node_ptr->id_with_data_ptr->units.empty()) {
    result.append(Concat(
      "["sv,
      Sv(TranslateData(genuine_node_ptr->id_with_data_ptr->units, node_view.Source(), true)),
      "]"sv
    ));
  }

  return result;
}

std::string TranslateKill(const NodeView& node_view) {
  return Concat(
    "kill "sv,
    Sv(TranslateEntitySubcommand(node_view))
  );
}

std::string TranslateNative(const NodeView& node_view) {
  BaseToken command = node_view[0];
  ++command.start;
  --command.end;

  return static_cast<std::string>(Extract(node_view.Source(), command));
}

std::string TranslateParticle(const NodeView& node_view) {
  const auto& raw_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;
  std::string particle_name = static_cast<std::string>(Extract(node_view.Source(), raw_ptr->identifier));
  if (!raw_ptr->units.empty()) {
    particle_name.append(Concat(
      "["sv,
      Sv(TranslateData(raw_ptr->units, node_view.Source(), false)),
      "]"sv
    ));
  }

  return Concat(
    "particle "sv,
    Sv(particle_name), " "sv,
    node_view.Extract(0), " "sv, // x
    node_view.Extract(1), " "sv, // y
    node_view.Extract(2), " "sv, // z
    node_view.Extract(3), " "sv, // dx
    node_view.Extract(4), " "sv, // dy
    node_view.Extract(5), " "sv, // dz
    node_view.Extract(6), " "sv, // speed
    node_view.Extract(7), " "sv, // count
    node_view.Extract(8)         // mode
  );
}

std::string TranslatePlaysound(const NodeView& node_view) {
  IndexType current_arg = 0;

  std::string result = Concat(
    "playsound "sv,
    node_view.Extract(current_arg), // sound id
    " neutral "sv
  );

  ++current_arg;
  if (node_view.size() == 6) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 7, with entity name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.append(Concat(
    " "sv, node_view.Extract(current_arg),      // x
    " "sv, node_view.Extract(current_arg + 1),  // y
    " "sv, node_view.Extract(current_arg + 2),  // z
    " "sv, node_view.Extract(current_arg + 3),  // volume
    " "sv, node_view.Extract(current_arg + 4)   // pitch
  ));

  return result;
}

std::string TranslateSay(const NodeView& node_view) {
  return Concat(
    "say "sv,
    node_view.Extract(0)
  );
}

std::string TranslateScoreboardObjectivesAdd(const NodeView& node_view) {
  std::string result = Concat(
    "scoreboard objectives add "sv,
    node_view.Extract(0), " "sv, // objective
    node_view.Extract(1)         // objective type
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
  return Concat(
    "scoreboard objectives setdisplay "sv,
    node_view.Extract(0), " "sv, // mode
    node_view.Extract(1)         // objective
  );
}

std::string TranslateScoreboardPlayers(const NodeView& node_view) {
  std::string result = Concat(
    "scoreboard players "sv,
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

  result.append(Concat(
    " "sv, node_view.Extract(current_argument),    // objective
    " "sv, node_view.Extract(current_argument + 1) // points
  ));

  return result;
}

std::string TranslateSetblock(const NodeView& node_view) {
  return Concat(
    "setblock "sv,
    node_view.Extract(0), " "sv, // block
    node_view.Extract(1), " "sv, // x
    node_view.Extract(2), " "sv, // y
    node_view.Extract(3), " "sv, // z
    node_view.Extract(4)         // mode
  );
}

std::string TranslateSpectate(const NodeView& node_view) {
  return Concat(
    "spectate "sv,
    Sv(TranslateEntitySubcommand(node_view))
  );
}

std::string TranslateStopsound(const NodeView& node_view) {
  std::string result = "stopsound ";
  IndexType current_arg = 0;

  if (node_view.size() == 2) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 3, with player name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.append(Concat(
    " "sv, node_view.Extract(current_arg),
    " "sv, node_view.Extract(current_arg + 1)
  ));

  return result;
}

std::string TranslateSummon(const NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;

  std::string result = Concat(
    "summon "sv,
    Extract(node_view.Source(), id_with_data_ptr->identifier), " "sv, // entity name
    node_view.Extract(0), " "sv, // x
    node_view.Extract(1), " "sv, // y
    node_view.Extract(2)         // z
  );

  if (!id_with_data_ptr->units.empty()) {
    result.push_back(' ');
    result.append(Concat(
      "{"sv,
      Sv(TranslateData(id_with_data_ptr->units, node_view.Source(), false)),
      "}"sv
    ));
  }

  return result;
}

std::string TranslateTag(const NodeView& node_view) {
  std::string result = "tag ";
  IndexType current_arg = 0;

  if (node_view.size() == 2) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 3, with player name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.append(Concat(
    " "sv, node_view.Extract(current_arg),
    " "sv, node_view.Extract(current_arg + 1)
  ));

  return result;
}

std::string TranslateTeamAdd(const NodeView& node_view) {
  return Concat("team add "sv, node_view.Extract(0));
}

std::string TranslateTeamJoin(const NodeView& node_view) {
  std::string result = Concat("team join "sv, node_view.Extract(0), " "sv);

  if (node_view.size() == 1) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 2, with entity name
    result.append(node_view.Extract(1));
  }

  return result;
}

std::string TranslateTeamModify(const NodeView& node_view) {
  return Concat(
    "team modify "sv,
    node_view.Extract(0), " "sv,
    node_view.Extract(1), " "sv,
    node_view.Extract(2)
  );
}

std::string TranslateTellraw(const NodeView& node_view) {
  std::string result = Concat(
    "tellraw "sv,
    Sv(TranslateEntitySubcommand(node_view))
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

      if (raw_ptr->second_selector.stem.type != TokenType::Identifier) {
        result.push_back(' ');
        result.append(TranslateSelector(raw_ptr->second_selector, node_view.Source()));
      }
      
      return result;
    case 1: // one argument is a entity name
      if (raw_ptr->selector.stem.type == TokenType::Identifier) {
        result.append(node_view.Extract(0));

        if (raw_ptr->second_selector.stem.type != TokenType::Identifier) {
          result.push_back(' ');
          result.append(TranslateSelector(raw_ptr->second_selector, node_view.Source()));
        }
      } else {
        result.append(Concat(Sv(TranslateSelector(node_view)), " "sv, node_view.Extract(0)));
      }

      return result;
    case 2: // both arguments are entity names
      result.append(Concat(
        node_view.Extract(0), " "sv,
        node_view.Extract(1)
      ));  

      return result;
    case 3: // only coordinates
      if (raw_ptr->selector.stem.type != TokenType::Identifier) {
        result.append(TranslateSelector(node_view));
        result.push_back(' ');
      }

      result.append(Concat(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2)
      ));

      return result;
    case 4: // one entity name and coordinates
      result.append(Concat(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3)
      ));

      return result;
    case 5: // only coordinates
      if (raw_ptr->selector.stem.type != TokenType::Identifier) {
        result.append(TranslateSelector(node_view));
        result.push_back(' ');
      }

      result.append(Concat(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3), " "sv,
        node_view.Extract(4)
      ));

      return result;
    case 6: // one entity name and 5 coordinates
      result.append(Concat(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3), " "sv,
        node_view.Extract(4), " "sv,
        node_view.Extract(5)
      ));

      return result;
    default:
      throw std::logic_error("Internal translation error - impossible case reached in TranslateTp()");
  }
}
