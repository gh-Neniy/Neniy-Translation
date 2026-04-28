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

  std::string TranslateBlockDataIfExists(const std::vector<DataUnit>& data_units, std::string_view source_code) {
    if (data_units.empty()) {
      return "";
    }

    auto translated_block_data = TranslateBlockData(data_units, source_code, "=");
    std::string result;

    if (!translated_block_data.first.empty()) {
      result.append(Concat("["sv, Sv(translated_block_data.first), "]"sv));
    }

    if (!translated_block_data.second.empty()) {
      result.append(Concat("{"sv, Sv(translated_block_data.second), "}"sv));
    }

    return result;
  }

  std::string TranslateEntitySubcommand(const NodeView& node_view, IndexType entity_pos) {
    if (entity_pos >= node_view.size()) { // with selector
      return TranslateSelector(node_view);
    }
    
    return static_cast<std::string>(node_view.Extract(entity_pos));
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
      Sv(TranslateEntitySubcommand(node_view, 0))
    );
  }

  std::string TranslateExecuteAt(const NodeView& node_view) {
    return Concat(
      "at "sv,
      Sv(TranslateEntitySubcommand(node_view, 0))
    );
  }

  std::string TranslateExecuteBlock(const NodeView& node_view) {
    const auto& id_with_data_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;

    return Concat (
      node_view.Extract(0), // if | unless
      " block "sv,
      node_view.Extract(1), " "sv, // x
      node_view.Extract(2), " "sv, // y
      node_view.Extract(3), " "sv, // z
      Sv(Concat (
        Extract(node_view.Source(), id_with_data_ptr->identifier), // block
        Sv(TranslateBlockDataIfExists(id_with_data_ptr->units, node_view.Source()))
      ))
    );
  }

  std::string TranslateExecuteEntity(const NodeView& node_view) {
    return Concat(
      node_view.Extract(0), // condition
      " entity "sv,
      Sv(TranslateEntitySubcommand(node_view, 1))
    );
  }

  std::string TranslateExecuteItemsBlock(const NodeView& node_view) {
    return Concat (
      node_view.Extract(0),         // condition
      " items block ~ ~ ~ "sv,
      node_view.Extract(1), " "sv,  // container
      node_view.Extract(2)          // item name
    );
  }

  std::string TranslateExecuteItemsEntity(const NodeView& node_view) {
    std::string result = Concat(node_view.Extract(0), " items entity "sv);
    IndexType current_arg = 1;

    if (node_view.size() == 3) { // with selector
      result.append(TranslateSelector(node_view));
    } else {
      result.append(node_view.Extract(current_arg));
      ++current_arg;
    }

    result.push_back(' ');
    result.append(node_view.Extract(current_arg)); // container
    result.push_back(' ');
    result.append(node_view.Extract(current_arg + 1)); // item name

    return result;
  }

  std::string TranslateExecuteScore(const NodeView& node_view) {
    std::string result = Concat(
      node_view.Extract(0),
      " score "sv
    );

    IndexType current_arg = 1;

    if (node_view.size() == 3) { // with selector
      result.append(TranslateSelector(node_view));
    } else { // == 4, with entity name
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

  std::string TranslateExecuteStoreBossbar(const NodeView& node_view) {
    return Concat (
      "store result bossbar "sv,
      node_view.Extract(0), // bossbar name
      " value"sv
    );
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

  std::string TranslateExecuteFacing(const NodeView& node_view) {
    std::string result = "facing ";

    if (node_view.size() == 0) {
      result.append(TranslateSelector(node_view));
    } else if (node_view.size() == 1) {
      result.append(node_view.Extract(0));
    } else {
      result.append(Concat(node_view.Extract(0), " "sv, node_view.Extract(1), " "sv, node_view.Extract(2)));
    }

    return result;
  }
}

std::string TranslateBossbarAdd(const NodeView& node_view) {
  return Concat (
    "bossbar add "sv,
    node_view.Extract(0), " "sv, // bossbar name
    Sv(TranslateText(node_view.get<TextNode*>()->text, node_view.Source()))
  );
}

std::string TranslateBossbarSet(const NodeView& node_view) {
  std::string result = Concat("bossbar set "sv, node_view.Extract(0), " "sv);

  std::string_view submode = node_view.Extract(1);
  result.append(submode);
  result.push_back(' ');

  if (submode == "players") {
    result.append(TranslateEntitySubcommand(node_view, 2));
  } else { // == color or max
    result.append(node_view.Extract(2));
  }

  return result;
}

std::string TranslateBossbarRemove(const NodeView& node_view) {
  return Concat (
    "bossbar remove "sv,
    node_view.Extract(0) // bossbar name
  );
}

std::string TranslateClear(const NodeView& node_view) {
  std::string result = "clear ";
  IndexType current_arg = 0;

  const auto& selector = node_view.get<SelectorNode*>()->selector;

  if (selector.stem.type != TokenType::Identifier) { // with selector
    result.append(TranslateSelector(selector, node_view.Source()));
  } else {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.push_back(' ');
  result.append(node_view.Extract(current_arg));

  if (node_view.size() == current_arg + 2) {
    result.push_back(' ');
    result.append(node_view.Extract(current_arg + 1));
  }

  return result;
}

std::string TranslateClone(const NodeView& node_view) {
  std::string result = Concat(
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

  if (node_view.size() == 11) {
    result.append(Concat(" "sv, node_view.Extract(10)));
  }

  return result;
}

std::string TranslateDamage(const NodeView& node_view) {
  std::string result = "damage ";
  IndexType current_arg = 0;

  if (node_view.size() == 2) { // with entity name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else {
    result.append(TranslateSelector(node_view));
  }

  result.push_back(' ');
  result.append(node_view.Extract(current_arg));
  result.append(" generic_kill");

  return result;
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
    " "sv, node_view.Extract(current_arg + 1),                // mode
    " value \""sv, node_view.Extract(current_arg + 2), "\""sv // value
  ));

  return result;
}

std::string TranslateEffect(const NodeView& node_view) {
  std::string result = "effect ";
  std::string_view mode = node_view.Extract(0);

  result.append(mode);
  result.push_back(' ');
  
  IndexType current_arg = 1;

  if (node_view.size() == (mode == "clear" ? 3 : 5)) {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else { // == 2 or 4
    result.append(TranslateSelector(node_view));
  }

  // current_arg on effect id
  result.push_back(' ');
  result.append(node_view.Extract(current_arg));

  if (mode == "clear") {
    return result;
  }

  result.push_back(' ');
  result.append(node_view.Extract(current_arg + 1)); // duration
  result.push_back(' ');
  result.append(node_view.Extract(current_arg + 2)); // amplifier
  result.append(" true");

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
      case CommandType::ExecuteBlock:
        result.append(TranslateExecuteBlock(subnode_view));
        break;
      case CommandType::ExecuteEntity:
        result.append(TranslateExecuteEntity(subnode_view));
        break;
      case CommandType::ExecuteFacing:
        result.append(TranslateExecuteFacing(subnode_view));
        break;
      case CommandType::ExecuteItemsBlock:
        result.append(TranslateExecuteItemsBlock(subnode_view));
        break;
      case CommandType::ExecuteItemsEntity:
        result.append(TranslateExecuteItemsEntity(subnode_view));
        break;
      case CommandType::ExecuteScore:
        result.append(TranslateExecuteScore(subnode_view));
        break;
      case CommandType::ExecutePositioned:
        result.append(TranslateExecutePositioned(subnode_view));
        break;
      case CommandType::ExecuteStoreBossbar:
        result.append(TranslateExecuteStoreBossbar(subnode_view));
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
  const auto& id_with_data_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;
  const auto& id = id_with_data_ptr->identifier;
  const auto& units = id_with_data_ptr->units;

  std::string result = Concat(
    "fill "sv,
    node_view.Extract(0), " "sv, // start x
    node_view.Extract(1), " "sv, // start y
    node_view.Extract(2), " "sv, // start z
    node_view.Extract(3), " "sv, // end x
    node_view.Extract(4), " "sv, // end y
    node_view.Extract(5), " "sv, // end z
    Extract(node_view.Source(), id), // block
    Sv(TranslateBlockDataIfExists(units, node_view.Source())), " "sv,
    node_view.Extract(6)  // mode
  );

  if (node_view.size() == 8) {
    result.append(Concat(" "sv, node_view.Extract(7)));
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
  std::string_view rule = node_view.Extract(0);

  if (rule == "natural_regeneration") {
    rule = "natural_health_regeneration";
  }

  return Concat(
    "gamerule "sv,
    rule, " "sv,
    node_view.Extract(1)
  );
}

std::string TranslateGive(const NodeView& node_view) {
  std::string result = "give ";

  auto raw_ptr = node_view.get<SelectorIdWithDataPtrNode*>();
  IndexType current_arg = 0;

  if (raw_ptr->selector.stem.type == TokenType::Identifier) {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else {
    result.append(TranslateSelector(raw_ptr->selector, node_view.Source()));
  }

  result.push_back(' ');
  result.append(Extract(node_view.Source(), raw_ptr->id_with_data_ptr->identifier));

  if (!raw_ptr->id_with_data_ptr->units.empty()) {
    result.append(Concat(
      "["sv,
      Sv(TranslateItemData(raw_ptr->id_with_data_ptr->units, node_view.Source(), "=")),
      "]"sv
    ));
  }

  if (node_view.size() == current_arg + 1) {
    result.push_back(' ');
    result.append(node_view.Extract(current_arg));
  }

  return result;
}

std::string TranslateKill(const NodeView& node_view) {
  return Concat(
    "kill "sv,
    Sv(TranslateEntitySubcommand(node_view, 0))
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
      "{"sv,
      Sv(TranslateParticleData(raw_ptr->units, node_view.Source())),
      "}"sv
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

  auto raw_ptr = node_view.get<TextNode*>();

  if (raw_ptr->text.units.empty()) {
    return result;
  }

  result.push_back(' ');
  result.append(TranslateText(raw_ptr->text, node_view.Source())); // objective name

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
  const auto& id_with_data_ptr = node_view.get<IdWithDataPtrNode*>()->id_with_data_ptr;

  return Concat(
    "setblock "sv,
    node_view.Extract(0), " "sv, // x
    node_view.Extract(1), " "sv, // y
    node_view.Extract(2), " "sv, // z
    Sv(Concat (
      Extract(node_view.Source(), id_with_data_ptr->identifier), // block
      Sv(TranslateBlockDataIfExists(id_with_data_ptr->units, node_view.Source()))
    )), " "sv,
    node_view.Extract(3)         // mode
  );
}

std::string TranslateSpawnpoint(const NodeView& node_view) {
  std::string result = "spawnpoint ";
  IndexType current_arg = 0;

  if (node_view.size() == 3) { // with selector
    result.append(TranslateSelector(node_view));
  } else {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.push_back(' ');
  
  result.append(Concat(
    node_view.Extract(current_arg), " "sv,
    node_view.Extract(current_arg + 1), " "sv,
    node_view.Extract(current_arg + 2)
  ));

  return result;
}

std::string TranslateSpectate(const NodeView& node_view) {
  return Concat(
    "spectate "sv,
    Sv(TranslateEntitySubcommand(node_view, 0))
  );
}

std::string TranslateStopsound(const NodeView& node_view) {
  std::string result = "stopsound ";
  IndexType current_arg = 0;

  if (node_view.size() == 1) { // with selector
    result.append(TranslateSelector(node_view));
  } else { // == 2, with player name
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  }

  result.append(" * ");
  result.append(node_view.Extract(current_arg)); // sound id

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
      Sv(TranslateEntityData(id_with_data_ptr->units, node_view.Source())),
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
  std::string_view rule = node_view.Extract(1);

  if (rule == "friendly_fire") {
    rule = "friendlyFire";
  } else if (rule == "collision") {
    rule = "collisionRule";
  }

  return Concat(
    "team modify "sv,
    node_view.Extract(0), " "sv,
    rule, " "sv,
    node_view.Extract(2)
  );
}

std::string TranslateTellraw(const NodeView& node_view) {
  std::string result = Concat(
    "tellraw "sv,
    Sv(TranslateEntitySubcommand(node_view, 0))
  );

  result.push_back(' ');
  result.append(TranslateText(node_view.get<SelectorTextNode*>()->text, node_view.Source()));

  return result;
}

std::string TranslateTime(const NodeView& node_view) {
  return Concat(
    "time "sv,
    node_view.Extract(0), " "sv,  // mode
    node_view.Extract(1)          // value
  );
}

std::string TranslateTitle(const NodeView& node_view) {
  std::string result = "title ";
  IndexType current_arg = 0;

  const auto& node = node_view.get<SelectorTextNode*>();
  const auto& selector = node->selector;

  if (selector.stem.type == TokenType::Identifier) {
    result.append(node_view.Extract(current_arg));
    ++current_arg;
  } else {
    result.append(TranslateSelector(selector, node_view.Source()));
  }

  result.push_back(' ');
  result.append(node_view.Extract(current_arg));

  result.push_back(' ');
  result.append(TranslateText(node->text, node_view.Source()));

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
