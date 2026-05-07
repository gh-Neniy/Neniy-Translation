#include "lexic/Token.hpp"
#include "trans/Translate.hpp"
#include "TranslateData.hpp"
#include "TranslateSelector.hpp"
#include "TranslateText.hpp"
#include "Method.hpp"

using namespace std::string_view_literals;

namespace {
  void TranslateBlockData(NodeView& node_view, const std::vector<DataUnit>& units) {
    node_view.PushBack('[');
    std::vector<Text> sign_data = TranslateBlockData(node_view, units, "="sv);
    node_view.PushBack(']');

    if (!sign_data.empty()) {
      if (sign_data.size() != 4) {
        throw std::runtime_error("Translation error - sign data size != 4");
      }
  
      node_view.Append("{front_text:{messages:");
      TranslateLore(node_view, sign_data);
      node_view.Append("}}");
    }
  }

  IndexType TranslateEntity(NodeView& node_view, IndexType entity_pos) {
    const auto& selector = node_view.Get<SelectorNode*>()->selector;

    if (selector.stem.type == TokenType::Identifier) {
      node_view.Append(node_view.Extract(entity_pos));
      ++entity_pos;
    } else {
      TranslateSelector(node_view, selector);
    }

    return entity_pos;
  }

  void TranslateExecuteAs(NodeView& node_view) {
    node_view.Append("as ");
    TranslateEntity(node_view, 0);
  }

  void TranslateExecuteAt(NodeView& node_view) {
    node_view.Append("at ");
    TranslateEntity(node_view, 0);
  }

  void TranslateExecuteBlock(NodeView& node_view) {
    const auto& id_with_data_ptr = node_view.Get<IdWithDataPtrNode*>()->id_with_data_ptr;

    node_view.Append(
      node_view.Extract(0), // if | unless
      " block "sv,
      node_view.Extract(1), " "sv, // x
      node_view.Extract(2), " "sv, // y
      node_view.Extract(3), " "sv, // z
      node_view.Extract(id_with_data_ptr->identifier) // block
    );

    if (!id_with_data_ptr->units.empty()) {
      TranslateBlockData(node_view, id_with_data_ptr->units);
    }
  }

  void TranslateExecuteEntity(NodeView& node_view) {
    node_view.Append(
      node_view.Extract(0), // condition
      " entity "sv
    );

    TranslateEntity(node_view, 1);
  }

  void TranslateExecuteItemsBlock(NodeView& node_view) {
    node_view.Append(
      node_view.Extract(0),         // condition
      " items block ~ ~ ~ "sv,
      node_view.Extract(1), " "sv,  // container
      node_view.Extract(2)          // item name
    );
  }

  void TranslateExecuteItemsEntity(NodeView& node_view) {
    node_view.Append(
      node_view.Extract(0), // condition
      " items entity "sv
    );

    IndexType current_arg = TranslateEntity(node_view, 1);

    node_view.Append(
      " "sv, node_view.Extract(current_arg),    // container
      " "sv, node_view.Extract(current_arg + 1) // item name
    );
  }

  void TranslateExecuteScore(NodeView& node_view) {
    if (node_view.ArgsSize() == 5) { // for operators
      node_view.Append(
        node_view.Extract(0),
        " score "sv,
        node_view.Extract(1), " "sv,  // entity
        node_view.Extract(2), " "sv,  // objective
        node_view.Extract(3), " "sv,  // operator
        node_view.Extract(4), " "sv,  // second entity
        node_view.Extract(2)          // same objective
      );

      return;
    }

    node_view.Append(
      node_view.Extract(0),
      " score "sv
    );

    IndexType current_arg = TranslateEntity(node_view, 1);

    node_view.Append(
      " "sv, node_view.Extract(current_arg),            // objective name
      " matches "sv, node_view.Extract(current_arg + 1) // points range
    );
  }

  void TranslateExecutePositioned(NodeView& node_view) {
    node_view.Append(
      "positioned "sv,
      node_view.Extract(0), " "sv, // x
      node_view.Extract(1), " "sv, // y
      node_view.Extract(2)         // z
    );
  }

  void TranslateExecuteStoreScore(NodeView& node_view) {
    node_view.Append("store result score ");

    IndexType current_arg = TranslateEntity(node_view, 0);

    node_view.Append(" "sv, node_view.Extract(current_arg));
  }

  void TranslateExecuteStoreBossbar(NodeView& node_view) {
    node_view.Append(
      "store result bossbar "sv,
      node_view.Extract(0), // bossbar name
      " value"sv
    );
  }

  void TranslateExecuteStoreEntity(NodeView& node_view) {
    node_view.Append("store result entity ");

    IndexType current_arg = TranslateEntity(node_view, 0);

    node_view.Append(
      " "sv, node_view.Extract(current_arg),
      " "sv, node_view.Extract(current_arg + 1),
      " "sv, node_view.Extract(current_arg + 2)
    );
  }

  void TranslateExecuteStoreStorage(NodeView& node_view) {
    node_view.Append(
      "store result storage "sv,
      node_view.Extract(0), " "sv,
      node_view.Extract(1), " "sv,
      node_view.Extract(2), " "sv,
      node_view.Extract(3)
    );
  }

  void TranslateExecuteUninited(NodeView& node_view) {
    node_view.Append("unless score ");

    IndexType initial_size = node_view.Result().size();
    IndexType current_arg = TranslateEntity(node_view, 0);

    std::string entity = node_view.Result().substr(initial_size);
    std::string_view objective = node_view.Extract(current_arg);

    node_view.Append(
      " "sv, objective, " = "sv,
      Sv(entity), " "sv, objective
    );
  }

  void TranslateExecuteFacing(NodeView& node_view) {
    node_view.Append("facing ");

    const auto& selector = node_view.Get<SelectorNode*>()->selector;

    if (selector.stem.type != TokenType::Identifier) {
      TranslateSelector(node_view, selector);
    } else if (node_view.ArgsSize() == 1) {
      node_view.Append(node_view.Extract(0));
    } else {
      node_view.Append(node_view.Extract(0), " "sv, node_view.Extract(1), " "sv, node_view.Extract(2));
    }
  }
}

void TranslateAdvancement(NodeView& node_view) {
  node_view.Append("advancement grant ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(" only "sv, node_view.Extract(current_arg)); // advancement name
}

void TranslateBossbarAdd(NodeView& node_view) {
  node_view.Append(
    "bossbar add "sv,
    node_view.Extract(0), " "sv // bossbar name
  );
  
  TranslateText(node_view, node_view.Get<TextNode*>()->text);
}

void TranslateBossbarSet(NodeView& node_view) {
  std::string_view submode = node_view.Extract(1);

  node_view.Append(
    "bossbar set "sv,
    node_view.Extract(0), " "sv, // bossbar name
    submode, " "sv
  );

  if (submode == "players") {
    TranslateEntity(node_view, 2);
  } else { // == color or max
    node_view.Append(node_view.Extract(2));
  }
}

void TranslateBossbarRemove(NodeView& node_view) {
  node_view.Append(
    "bossbar remove "sv,
    node_view.Extract(0) // bossbar name
  );
}

void TranslateClear(NodeView& node_view) {
  node_view.Append("clear ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(" "sv, node_view.Extract(current_arg)); // item

  if (node_view.ArgsSize() == current_arg + 2) {
    node_view.Append(" "sv, node_view.Extract(current_arg + 1)); // count
  }
}

void TranslateClone(NodeView& node_view) {
  node_view.Append(
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

  if (node_view.ArgsSize() == 11) {
    node_view.Append(" "sv, node_view.Extract(10));
  }
}

void TranslateDamage(NodeView& node_view) {
  node_view.Append("damage ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(
    " "sv, node_view.Extract(current_arg), // count
    " generic_kill"sv
  );
}

void TranslateData(NodeView& node_view) {
  std::string_view mode = node_view.Type() == CommandType::DataGet ? "get"sv : "modify"sv;

  node_view.Append("data "sv, mode, " entity "sv);

  IndexType current_arg = TranslateEntity(node_view, 0);

  std::string_view data_field = node_view.Extract(current_arg);

  if (data_field == "loot_table") {
    data_field = "DeathLootTable";
  }

  node_view.Append(" "sv, data_field);

  if (mode == "get") {
    return;
  }

  node_view.Append(
    " "sv, node_view.Extract(current_arg + 1),                // mode
    " value \""sv, node_view.Extract(current_arg + 2), "\""sv // value
  );
}

void TranslateEffect(NodeView& node_view) {
  std::string_view mode = node_view.Extract(0);

  node_view.Append("effect "sv, mode, " "sv);

  IndexType current_arg = TranslateEntity(node_view, 1);

  node_view.Append(" "sv, node_view.Extract(current_arg)); // effect name

  if (mode == "clear") {
    return;
  }

  node_view.Append(
    " "sv, node_view.Extract(current_arg + 1), // duration
    " "sv, node_view.Extract(current_arg + 2), // amplifier
    " true"sv
  );
}

void TranslateExecute(NodeView& node_view, std::string_view function_prefix, const Loader& loader) {
  node_view.Append("execute");

  auto raw_ptr = node_view.Get<ExecuteNode*>();

  for (std::size_t i = 0; i < raw_ptr->subnodes.size(); ++i) {
    NodeView subnode_view(node_view.Result(), raw_ptr->subnodes[i], node_view.Source());

    node_view.PushBack(' ');

    switch (subnode_view.Type()) {
      case CommandType::ExecuteAlign:
        node_view.Append("align "sv, subnode_view.Extract(0));
        break;
      case CommandType::ExecuteAnchored:
        node_view.Append("anchored "sv, subnode_view.Extract(0));
        break;
      case CommandType::ExecuteAs:
        TranslateExecuteAs(subnode_view);
        break;
      case CommandType::ExecuteAt:
        TranslateExecuteAt(subnode_view);
        break;
      case CommandType::ExecuteBlock:
        TranslateExecuteBlock(subnode_view);
        break;
      case CommandType::ExecuteEntity:
        TranslateExecuteEntity(subnode_view);
        break;
      case CommandType::ExecuteFacing:
        TranslateExecuteFacing(subnode_view);
        break;
      case CommandType::ExecuteItemsBlock:
        TranslateExecuteItemsBlock(subnode_view);
        break;
      case CommandType::ExecuteItemsEntity:
        TranslateExecuteItemsEntity(subnode_view);
        break;
      case CommandType::ExecuteScore:
        TranslateExecuteScore(subnode_view);
        break;
      case CommandType::ExecutePositioned:
        TranslateExecutePositioned(subnode_view);
        break;
      case CommandType::ExecuteStoreBossbar:
        TranslateExecuteStoreBossbar(subnode_view);
        break;
      case CommandType::ExecuteStoreEntity:
        TranslateExecuteStoreEntity(subnode_view);
        break;
      case CommandType::ExecuteStoreStorage:
        TranslateExecuteStoreStorage(subnode_view);
        break;
      case CommandType::ExecuteStoreScore:
        TranslateExecuteStoreScore(subnode_view);
        break;
      case CommandType::ExecuteUninited:
        TranslateExecuteUninited(subnode_view);
        break;
      default:
        throw std::logic_error("Internal translation error - unknown execute subcommand type");
    }
  }

  node_view.Append(" run ");

  NodeView main_view(node_view.Result(), raw_ptr->main_node, node_view.Source());

  ChooseTranslate(main_view, function_prefix, loader);
}

void TranslateFill(NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.Get<IdWithDataPtrNode*>()->id_with_data_ptr;

  node_view.Append(
    "fill "sv,
    node_view.Extract(0), " "sv, // start x
    node_view.Extract(1), " "sv, // start y
    node_view.Extract(2), " "sv, // start z
    node_view.Extract(3), " "sv, // end x
    node_view.Extract(4), " "sv, // end y
    node_view.Extract(5), " "sv, // end z
    node_view.Extract(id_with_data_ptr->identifier) // block
  );

  if (!id_with_data_ptr->units.empty()) {
    TranslateBlockData(node_view, id_with_data_ptr->units);
  }

  node_view.Append(" "sv, node_view.Extract(6)); // mode

  if (node_view.ArgsSize() == 8) {
    node_view.Append(" "sv, node_view.Extract(7));
  }
}

void TranslateFunction(NodeView& node_view, std::string_view function_prefix) {
  node_view.Append("function ");

  std::string_view function_body = node_view.Extract(0);

  if (!function_prefix.empty()) {
    if (function_body.find(':') != std::string_view::npos) {
      throw std::runtime_error("Translation error - double prefix in function body");
    }

    node_view.Append(function_prefix, ":"sv);

  } else if (function_body.find(':') == std::string_view::npos) {
    throw std::runtime_error("Translation error - no prefix found in function body");
  }

  node_view.Append(function_body);

  if (node_view.ArgsSize() == 2) {
    node_view.Append(" with storage "sv, node_view.Extract(1));
  }
}

void TranslateGamemode(NodeView& node_view) {
  node_view.Append(
    "gamemode "sv,
    node_view.Extract(0), " "sv // mode
  );

  TranslateEntity(node_view, 1);
}

void TranslateGamerule(NodeView& node_view) {
  std::string_view rule = node_view.Extract(0);

  if (rule == "natural_regeneration") {
    rule = "natural_health_regeneration";
  }

  node_view.Append(
    "gamerule "sv,
    rule, " "sv,
    node_view.Extract(1) // value
  );
}

void TranslateGive(NodeView& node_view) {
  node_view.Append("give ");

  IndexType current_arg = TranslateEntity(node_view, 0);
  const auto& id_with_data_ptr = node_view.Get<SelectorIdWithDataPtrNode*>()->id_with_data_ptr;

  node_view.Append(" "sv, node_view.Extract(id_with_data_ptr->identifier));

  if (!id_with_data_ptr->units.empty()) {
    node_view.PushBack('[');
    TranslateItemData(node_view, id_with_data_ptr->units, "=");
    node_view.PushBack(']');
  }

  if (node_view.ArgsSize() == current_arg + 1) {
    node_view.Append(" "sv, node_view.Extract(current_arg));
  }
}

void TranslateKill(NodeView& node_view) {
  node_view.Append("kill ");
  
  TranslateEntity(node_view, 0);
}

void TranslateNative(NodeView& node_view) {
  BaseToken command = node_view[0];
  ++command.start;
  --command.end;

  node_view.Append(node_view.Extract(command));
}

void TranslateParticle(NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.Get<IdWithDataPtrNode*>()->id_with_data_ptr;

  node_view.Append("particle "sv, node_view.Extract(id_with_data_ptr->identifier));

  if (!id_with_data_ptr->units.empty()) {
    node_view.PushBack('{');
    TranslateParticleData(node_view, id_with_data_ptr->units);
    node_view.PushBack('}');
  }

  node_view.Append(
    " "sv, node_view.Extract(0),  // x
    " "sv, node_view.Extract(1),  // y
    " "sv, node_view.Extract(2),  // z
    " "sv, node_view.Extract(3),  // dx
    " "sv, node_view.Extract(4),  // dy
    " "sv, node_view.Extract(5),  // dz
    " "sv, node_view.Extract(6),  // speed
    " "sv, node_view.Extract(7),  // count
    " "sv, node_view.Extract(8)   // mode
  );
}

void TranslatePlaysound(NodeView& node_view) {
  node_view.Append(
    "playsound "sv,
    node_view.Extract(0), // sound id
    " neutral "sv
  );

  IndexType current_arg = TranslateEntity(node_view, 1);

  node_view.Append(
    " "sv, node_view.Extract(current_arg),      // x
    " "sv, node_view.Extract(current_arg + 1),  // y
    " "sv, node_view.Extract(current_arg + 2),  // z
    " "sv, node_view.Extract(current_arg + 3),  // volume
    " "sv, node_view.Extract(current_arg + 4)   // pitch
  );
}

void TranslateSay(NodeView& node_view) {
  node_view.Append(
    "say "sv,
    node_view.Extract(0)
  );
}

void TranslateScoreboardObjectivesAdd(NodeView& node_view) {
  node_view.Append(
    "scoreboard objectives add "sv,
    node_view.Extract(0), " "sv, // objective
    node_view.Extract(1)         // objective type
  );

  const auto& text = node_view.Get<TextNode*>()->text;

  if (text.units.empty()) {
    return;
  }

  node_view.PushBack(' ');
  TranslateText(node_view, text); // objective name
}

void TranslateScoreboardObjectivesSet(NodeView& node_view) {
  node_view.Append(
    "scoreboard objectives setdisplay "sv,
    node_view.Extract(0), " "sv, // mode
    node_view.Extract(1)         // objective
  );
}

void TranslateScoreboardPlayers(NodeView& node_view) {
  std::string_view mode = node_view.Extract(0);

  if (mode == "opr") {
    mode = "operation";
  }

  node_view.Append(
    "scoreboard players "sv,
    mode, " "sv
  );

  IndexType current_arg = TranslateEntity(node_view, 1);

  node_view.Append(" "sv, node_view.Extract(current_arg)); // objective

  if (mode == "reset" || mode == "get") {
    return;
  }

  node_view.Append(" "sv, node_view.Extract(current_arg + 1));

  if (mode == "operation") {
    node_view.Append(
      " "sv, node_view.Extract(current_arg + 2),  // second entity
      " "sv, node_view.Extract(current_arg)       // same objective
    );
  }
}

void TranslateSetblock(NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.Get<IdWithDataPtrNode*>()->id_with_data_ptr;
  
  node_view.Append(
    "setblock "sv,
    node_view.Extract(0), " "sv, // x
    node_view.Extract(1), " "sv, // y
    node_view.Extract(2), " "sv, // z
    node_view.Extract(id_with_data_ptr->identifier) // block
  );

  if (!id_with_data_ptr->units.empty()) {
    TranslateBlockData(node_view, id_with_data_ptr->units);
  }
  
  node_view.Append(" "sv, node_view.Extract(3)); // mode
}

void TranslateSpawnpoint(NodeView& node_view) {
  node_view.Append("spawnpoint ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(
    " "sv, node_view.Extract(current_arg),
    " "sv, node_view.Extract(current_arg + 1),
    " "sv, node_view.Extract(current_arg + 2)
  );
}

void TranslateSpectate(NodeView& node_view) {
  node_view.Append("spectate ");

  TranslateEntity(node_view, 0);
}

void TranslateStopsound(NodeView& node_view) {
  node_view.Append("stopsound ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(" * "sv, node_view.Extract(current_arg)); // sound id
}

void TranslateSummon(NodeView& node_view) {
  const auto& id_with_data_ptr = node_view.Get<IdWithDataPtrNode*>()->id_with_data_ptr;

  node_view.Append(
    "summon "sv,
    node_view.Extract(id_with_data_ptr->identifier), " "sv, // entity name
    node_view.Extract(0), " "sv, // x
    node_view.Extract(1), " "sv, // y
    node_view.Extract(2)         // z
  );

  if (!id_with_data_ptr->units.empty()) {
    node_view.Append(" {");
    TranslateEntityData(node_view, id_with_data_ptr->units);
    node_view.PushBack('}');
  }
}

void TranslateTag(NodeView& node_view) {
  node_view.Append("tag ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(
    " "sv, node_view.Extract(current_arg),
    " "sv, node_view.Extract(current_arg + 1)
  );
}

void TranslateTeamAdd(NodeView& node_view) {
  node_view.Append("team add "sv, node_view.Extract(0));
}

void TranslateTeamJoin(NodeView& node_view) {
  node_view.Append("team join "sv, node_view.Extract(0), " "sv);

  TranslateEntity(node_view, 1);
}

void TranslateTeamModify(NodeView& node_view) {
  std::string_view rule = node_view.Extract(1);

  if (rule == "friendly_fire") {
    rule = "friendlyFire";
  } else if (rule == "collision") {
    rule = "collisionRule";
  }

  node_view.Append(
    "team modify "sv,
    node_view.Extract(0), " "sv,
    rule, " "sv,
    node_view.Extract(2)
  );
}

void TranslateTellraw(NodeView& node_view) {
  node_view.Append("tellraw ");
  
  TranslateEntity(node_view, 0);

  node_view.PushBack(' ');

  TranslateText(node_view, node_view.Get<SelectorTextNode*>()->text);
}

void TranslateTime(NodeView& node_view) {
  node_view.Append(
    "time "sv,
    node_view.Extract(0), " "sv,  // mode
    node_view.Extract(1)          // value
  );
}

void TranslateTitle(NodeView& node_view) {
  node_view.Append("title ");

  IndexType current_arg = TranslateEntity(node_view, 0);

  node_view.Append(" "sv, node_view.Extract(current_arg), " "sv); // mode

  TranslateText(node_view, node_view.Get<SelectorTextNode*>()->text);
}

void TranslateTp(NodeView& node_view) {
  node_view.Append("tp ");

  auto raw_ptr = node_view.Get<DoubleSelectorNode*>();

  switch (node_view.ArgsSize()) {
    case 0: // one or two selectors
      TranslateSelector(node_view, raw_ptr->selector);

      if (raw_ptr->second_selector.stem.type != TokenType::Identifier) {
        node_view.PushBack(' ');
        TranslateSelector(node_view, raw_ptr->second_selector);
      }
      
      break;
    case 1: // one argument is a entity name
      if (raw_ptr->selector.stem.type == TokenType::Identifier) {
        node_view.Append(node_view.Extract(0));

        if (raw_ptr->second_selector.stem.type != TokenType::Identifier) {
          node_view.PushBack(' ');
          TranslateSelector(node_view, raw_ptr->second_selector);
        }
      } else {
        TranslateSelector(node_view, raw_ptr->selector);
        node_view.Append(" "sv, node_view.Extract(0));
      }

      break;
    case 2: // both arguments are entity names
      node_view.Append(
        node_view.Extract(0), " "sv,
        node_view.Extract(1)
      );  

      break;
    case 3: // only coordinates
      if (raw_ptr->selector.stem.type != TokenType::Identifier) {
        TranslateSelector(node_view, raw_ptr->selector);
        node_view.PushBack(' ');
      }

      node_view.Append(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2)
      );

      break;
    case 4: // one entity name and coordinates
      node_view.Append(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3)
      );

      break;
    case 5: // only coordinates
      if (raw_ptr->selector.stem.type != TokenType::Identifier) {
        TranslateSelector(node_view, raw_ptr->selector);
        node_view.PushBack(' ');
      }

      node_view.Append(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3), " "sv,
        node_view.Extract(4)
      );

      break;
    case 6: // one entity name and 5 coordinates
      node_view.Append(
        node_view.Extract(0), " "sv,
        node_view.Extract(1), " "sv,
        node_view.Extract(2), " "sv,
        node_view.Extract(3), " "sv,
        node_view.Extract(4), " "sv,
        node_view.Extract(5)
      );

      break;
    default:
      throw std::logic_error("Internal translation error - impossible case reached in TranslateTp()");
  }
}
