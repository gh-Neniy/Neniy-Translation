#include "main/Concat.hpp"
#include "TranslateText.hpp"
#include "TranslateData.hpp"

#include <concepts>

using namespace std::string_view_literals;

namespace {
  void TranslateLore(NodeView& node_view, const std::vector<Text>& lore) {
    node_view.PushBack('[');

    for (std::size_t i = 0; i < lore.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      TranslateText(node_view, lore[i]);
    }

    node_view.PushBack(']');
  }

  void TranslateEnchantments(NodeView& node_view, const ListType& list) {
    node_view.PushBack('{');

    for (std::size_t i = 0; i < list.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      node_view.Append("\"minecraft:"sv, node_view.Extract(list[i].key), "\":"sv, node_view.Extract(list[i].value));
    }

    node_view.PushBack('}');
  }

  struct Attribute {
    Token key;
    BaseToken value;
  };

  struct Equipment {
    Token key;
    const IdWithDataPtr& value;
  };

  struct Chance {
    Token key;
    BaseToken chance;
  };

  void TranslateAttributes(NodeView& node_view, const std::vector<Attribute>& attributes) {
    node_view.Append("attributes:[");

    for (std::size_t i = 0; i < attributes.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      switch (attributes[i].key.type) {
        case TokenType::AttackDamage:
          node_view.Append("{id:\"minecraft:attack_damage\",base:"sv, node_view.Extract(attributes[i].value), "}"sv);
          break;
        case TokenType::AttackSpeed:
          node_view.Append("{id:\"minecraft:attack_speed\",base:"sv, node_view.Extract(attributes[i].value), "}"sv);
          break;
        case TokenType::Health:
          node_view.Append("{id:\"minecraft:max_health\",base:"sv, node_view.Extract(attributes[i].value), "}"sv);
          break;
        case TokenType::Stability:
          node_view.Append("{id:\"minecraft:knockback_resistance\",base:"sv, node_view.Extract(attributes[i].value), "}"sv);
          break;
        default:
          throw std::logic_error(Concat("Internal translation error - unknown attribute "sv, node_view.Extract(attributes[i].key)));
      }
    }

    node_view.PushBack(']');
  }

  void TranslateEquipmentUnit(NodeView& node_view, const IdWithDataPtr& id_with_data_ptr, std::string_view unit_name) {
    node_view.Append(unit_name, ":{id:\"minecraft:"sv, node_view.Extract(id_with_data_ptr->identifier), "\""sv);

    const auto& data_units = id_with_data_ptr->units;

    if (data_units.empty()) {
      node_view.PushBack('}');
      return;
    }
    
    node_view.Append(",components:{");
    TranslateItemData(node_view, data_units, ":"sv);
    node_view.Append("}}"sv);
  }

  void TranslateEquipment(NodeView& node_view, const std::vector<Equipment>& equipment) {
    node_view.Append("equipment:{");

    for (std::size_t i = 0; i < equipment.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      switch (equipment[i].key.type) {
        case TokenType::Chest:
          TranslateEquipmentUnit(node_view, equipment[i].value, "chest");
          break;
        case TokenType::Feet:
          TranslateEquipmentUnit(node_view, equipment[i].value, "feet");
          break;
        case TokenType::Head:
          TranslateEquipmentUnit(node_view, equipment[i].value, "head");
          break;
        case TokenType::LeftHand:
          TranslateEquipmentUnit(node_view, equipment[i].value, "offhand");
          break;
        case TokenType::Legs:
          TranslateEquipmentUnit(node_view, equipment[i].value, "legs");
          break;
        case TokenType::RightHand:
          TranslateEquipmentUnit(node_view, equipment[i].value, "mainhand");
          break;
        default:
          throw std::logic_error(Concat(
            "Internal translation error - unknown equipment key "sv, node_view.Extract(equipment[i].key)
          ));
      }
    }

    node_view.PushBack('}');
  }

  void TranslateTags(NodeView& node_view, const std::vector<BaseToken>& tags) {
    node_view.Append("Tags:[");

    for (int i = 0; i < tags.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      node_view.Append(node_view.Extract(tags[i]));
    }

    node_view.PushBack(']');
  }

  void TranslateNumericList(NodeView& node_view, const ListType& list) {
    node_view.PushBack('[');

    for (std::size_t i = 0; i < list.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      node_view.Append(node_view.Extract(list[i].key));
    }

    node_view.PushBack(']');
  }

  void TranslateChances(NodeView& node_view, const std::vector<Chance>& chances) {
    node_view.Append("drop_chances:{");

    for (std::size_t i = 0; i < chances.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      switch (chances[i].key.type) {
        case TokenType::ChestChance:
          node_view.Append("chest:"sv, node_view.Extract(chances[i].chance));
          break;
        case TokenType::FeetChance:
          node_view.Append("feet:"sv, node_view.Extract(chances[i].chance));
          break;
        case TokenType::HeadChance:
          node_view.Append("head:"sv, node_view.Extract(chances[i].chance));
          break;
        case TokenType::LeftHandChance:
          node_view.Append("offhand:"sv, node_view.Extract(chances[i].chance));
          break;
        case TokenType::LegsChance:
          node_view.Append("legs:"sv, node_view.Extract(chances[i].chance));
          break;
        case TokenType::RightHandChance:
          node_view.Append("mainhand:"sv, node_view.Extract(chances[i].chance));
          break;
        default:
          throw std::logic_error("Internal translation error - unknown key type in TranslateChances()");
      }
    }

    node_view.PushBack('}');
  }

  template <std::same_as<std::string_view>... StringViews>
  void AppendUnit(NodeView& node_view, bool cond, StringViews... args) {
    if (cond) {
      node_view.PushBack(',');
    }

    node_view.Append(args...);
  }

  void BlockDataSwitch(NodeView& node_view, const DataUnit& unit, std::string_view separator, std::vector<Text>& sign_msgs) {
    bool cond = node_view.Result().back() != '[';

    switch (unit.key.type) {
      case TokenType::Axis:
        AppendUnit(node_view, cond, "axis"sv, separator, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::East:
        AppendUnit(node_view, cond, "east"sv, separator, "true"sv);
        break;
      case TokenType::Facing:
        AppendUnit(node_view, cond, "facing"sv, separator, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Half:
        AppendUnit(node_view, cond, "half"sv, separator, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Level:
        AppendUnit(node_view, cond, "level"sv, separator, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Lit:
        AppendUnit(node_view, cond, "lit"sv, separator, "true"sv);
        break;
      case TokenType::North:
        AppendUnit(node_view, cond, "north"sv, separator, "true"sv);
        break;
      case TokenType::Open:
        AppendUnit(node_view, cond, "open"sv, separator, "true"sv);
        break;
      case TokenType::Powered:
        AppendUnit(node_view, cond, "powered"sv, separator, "true"sv);
        break;
      case TokenType::Sign:
        sign_msgs = std::get<std::vector<Text>>(unit.value);
        break;
      case TokenType::South:
        AppendUnit(node_view, cond, "south"sv, separator, "true"sv);
        break;
      case TokenType::West:
        AppendUnit(node_view, cond, "west"sv, separator, "true"sv);
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, node_view.Extract(unit.key), " in block data"sv));
    }
  }

  void AppendItem(NodeView& node_view, const IdWithDataPtr& id_with_data_ptr, bool about) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    node_view.Append(about ? "i"sv : "I"sv, "tem:{id:\"minecraft:"sv, node_view.Extract(id_with_data_ptr->identifier), "\""sv);
  
    if (!id_with_data_ptr->units.empty()) {
      node_view.Append(",components:{");
      TranslateItemData(node_view, id_with_data_ptr->units, ":");
      node_view.PushBack('}');
    }
  
    node_view.PushBack('}');
  }

  void TranslateBlockState(NodeView& node_view, const DataUnit& unit, bool is_falling_block) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    const auto& id_with_data_ptr = std::get<IdWithDataPtr>(unit.value);
    std::string_view block_state = is_falling_block ? "BlockState" : "block_state";

    node_view.Append(block_state, ":{Name:\"minecraft:"sv, node_view.Extract(id_with_data_ptr->identifier), "\""sv);

    const auto& data_units = id_with_data_ptr->units;

    if (!data_units.empty()) {
      node_view.Append(",Properties:{");
      TranslateBlockData(node_view, data_units, ":");
      node_view.PushBack('}');
    }

    node_view.PushBack('}');
  }

  void EntityDataSwitch(NodeView& node_view, const DataUnit& unit, std::vector<Attribute>& attributes,
                        std::vector<Equipment>& equipment, std::vector<BaseToken>& tags, std::vector<Chance>& chances) {
    bool cond = node_view.Result().back() != '{';

    switch (unit.key.type) {
      case TokenType::About:
        AppendItem(node_view, std::get<IdWithDataPtr>(unit.value), true);
        break;
      case TokenType::AttackDamage:
        attributes.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::AttackSpeed:
        attributes.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Block:
        TranslateBlockState(node_view, unit, true);
        break;
      case TokenType::CanGrab:
        AppendUnit(node_view, cond, "CanPickUpLoot:1b"sv);
        break;
      case TokenType::Chest:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::ChestChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Crit:
        AppendUnit(node_view, cond, "crit:1b"sv);
        break;
      case TokenType::Feet:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::FeetChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Head:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::HeadChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Health: {
        auto points = std::get<BaseToken>(unit.value);
        attributes.emplace_back(unit.key, points);

        AppendUnit(node_view, cond, "Health:"sv, node_view.Extract(points));
        break;
      }
      case TokenType::Id: // id for block_display
        TranslateBlockState(node_view, unit, false);
        break;
      case TokenType::InGround:
        AppendUnit(node_view, cond, "inGround:1b"sv);
        break;
      case TokenType::Invisible:
        AppendUnit(node_view, cond, "Invisible:1b"sv);
        break;
      case TokenType::Invulnerable:
        AppendUnit(node_view, cond, "Invulnerable:1b"sv);
        break;
      case TokenType::Item:
        AppendItem(node_view, std::get<IdWithDataPtr>(unit.value), false);
        break;
      case TokenType::LeftHand:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::LeftHandChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Legs:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::LegsChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::LootTable:
        AppendUnit(node_view, cond, "DeathLootTable:\""sv, node_view.Extract(std::get<BaseToken>(unit.value)), "\""sv);
        break;
      case TokenType::Name:
        AppendUnit(node_view, cond, "CustomName:"sv);
        TranslateText(node_view, std::get<Text>(unit.value));
        break;
      case TokenType::NameVisible:
        AppendUnit(node_view, cond, "CustomNameVisible:1b"sv);
        break;
      case TokenType::NoAI:
        AppendUnit(node_view, cond, "NoAI:1b"sv);
        break;
      case TokenType::NoDespawn:
        AppendUnit(node_view, cond, "PersistenceRequired:1b"sv);
        break;
      case TokenType::NoGravity:
        AppendUnit(node_view, cond, "NoGravity:1b"sv);
        break;
      case TokenType::PickupDelay:
        AppendUnit(node_view, cond, "PickupDelay:"sv, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Stability:
        attributes.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::RightHand:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::RightHandChance:
        chances.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Rotation:
        AppendUnit(node_view, cond, "Rotation:"sv);
        TranslateNumericList(node_view, std::get<ListType>(unit.value));
        break;
      case TokenType::Silent:
        AppendUnit(node_view, cond, "Silent:1b"sv);
        break;
      case TokenType::Size:
        AppendUnit(node_view, cond, "Size:"sv, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Tag:
        tags.push_back(std::get<BaseToken>(unit.value));
        break;
      case TokenType::TeleportDuration:
        AppendUnit(node_view, cond, "teleport_duration:"sv, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, node_view.Extract(unit.key), " in entity data"sv));
    }
  }

  struct PotionUnit {
    Token key;
    BaseToken value;
  };

  struct AttributeModifier { // for items
    Token key;
    BaseToken value;
  };

  void ItemDataSwitch(NodeView& node_view, const DataUnit& unit, std::string_view separator,
                      std::vector<PotionUnit>& potion_contents, std::vector<AttributeModifier>& attribute_modifiers, bool cond) {
    switch (unit.key.type) {
      case TokenType::AttackDamage:
        attribute_modifiers.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::AttackSpeed:
        attribute_modifiers.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::CanPlaceOn:
        AppendUnit(node_view, cond, "can_place_on"sv, separator, "{\"blocks\":\""sv, node_view.Extract(std::get<BaseToken>(unit.value)), "\"}"sv);
        break;
      case TokenType::Enchantments:
        AppendUnit(node_view, cond, "enchantments"sv, separator);
        TranslateEnchantments(node_view, std::get<ListType>(unit.value));
        break;
      case TokenType::Hide:
        AppendUnit(node_view, cond, "tooltip_display"sv, separator, "{hidden_components:[\"attribute_modifiers\",\"enchantments\",\"unbreakable\",\"can_place_on\",\"potion_contents\"]}"sv);
        break;
      case TokenType::Lore:
        AppendUnit(node_view, cond, "lore"sv, separator);
        TranslateLore(node_view, std::get<std::vector<Text>>(unit.value));
        break;
      case TokenType::Name:
        AppendUnit(node_view, cond, "custom_name"sv, separator);
        TranslateText(node_view, std::get<Text>(unit.value));
        break;
      case TokenType::Potion:
        potion_contents.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::PotionColor:
        potion_contents.emplace_back(unit.key, std::get<BaseToken>(unit.value));
        break;
      case TokenType::Shine:
        AppendUnit(node_view, cond, "enchantment_glint_override"sv, separator, "1b"sv);
        break;
      case TokenType::Stack:
        AppendUnit(node_view, cond, "max_stack_size"sv, separator, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Tag:
        AppendUnit(node_view, cond, 
          separator == "=" ? "custom_data" : "\"minecraft:custom_data\""sv, separator,
          "{tag:"sv, node_view.Extract(std::get<BaseToken>(unit.value)), "}"sv
        );
        break;
      case TokenType::Unbreakable:
        AppendUnit(node_view, cond, "unbreakable"sv, separator, "{}"sv);
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, node_view.Extract(unit.key), " in item data"sv));
    }
  }

  void ParticleDataSwitch(NodeView& node_view, const DataUnit& unit) {
    bool cond = node_view.Result().back() != '{';

    switch (unit.key.type) {
      case TokenType::Block:
        AppendUnit(node_view, cond, "block_state:"sv, node_view.Extract(std::get<IdWithDataPtr>(unit.value)->identifier));
        break;
      case TokenType::FromColor:
        AppendUnit(node_view, cond, "from_color:"sv);
        TranslateNumericList(node_view, std::get<ListType>(unit.value));
        break;
      case TokenType::Item:
        AppendUnit(node_view, cond, "item:"sv, node_view.Extract(std::get<IdWithDataPtr>(unit.value)->identifier));
        break;
      case TokenType::Scale:
        AppendUnit(node_view, cond, "scale:"sv, node_view.Extract(std::get<BaseToken>(unit.value)));
        break;
      case TokenType::ToColor:
        AppendUnit(node_view, cond, "to_color:"sv);
        TranslateNumericList(node_view, std::get<ListType>(unit.value));
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, node_view.Extract(unit.key), " in particle data"sv));
    }
  }

  void TranslatePotionContents(NodeView& node_view, const std::vector<PotionUnit>& potion_contents, std::string_view separator) {
    node_view.Append("potion_contents"sv, separator, "{"sv);

    for (std::size_t i = 0; i < potion_contents.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      switch (potion_contents[i].key.type) {
        case TokenType::Potion:
          node_view.Append("potion:"sv, node_view.Extract(potion_contents[i].value));
          break;
        case TokenType::PotionColor:
          node_view.Append("custom_color:"sv, node_view.Extract(potion_contents[i].value));
          break;
        default:
          throw std::logic_error (
            Concat("Internal translation error - unknown potion unit "sv, node_view.Extract(potion_contents[i].key))
          );
      }
    }

    node_view.PushBack('}');
  }

  void TranslateAttributeModifiers(NodeView& node_view, const std::vector<AttributeModifier>& attribute_modifiers,
                                          std::string_view separator) {
    node_view.Append("attribute_modifiers"sv, separator, "["sv);

    for (std::size_t i = 0; i < attribute_modifiers.size(); ++i) {
      if (i > 0) {
        node_view.PushBack(',');
      }

      std::string_view attribute_name = node_view.Extract(attribute_modifiers[i].key);

      node_view.Append(
        "{type:\"minecraft:"sv, attribute_name,
        "\",amount:"sv, node_view.Extract(attribute_modifiers[i].value),
        ",operation:\"add_value\",slot:\"mainhand\",id:\"base_"sv, attribute_name, "\"}"sv
      );
    }

    node_view.PushBack(']');
  }
}

void TranslateBlockData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator) {
  std::vector<Text> sign_msgs;

  node_view.PushBack('[');

  for (const auto& unit : units) {
    BlockDataSwitch(node_view, unit, separator, sign_msgs); 
  }

  node_view.PushBack(']');

  if (!sign_msgs.empty()) {
    node_view.Append("{front_text:{messages:");
    TranslateLore(node_view, sign_msgs);
    node_view.Append("}}");
  }
}

void TranslateEntityData(NodeView& node_view, const std::vector<DataUnit>& units) {
  std::vector<Attribute> attributes;
  std::vector<Equipment> equipment;
  std::vector<BaseToken> tags;
  std::vector<Chance> chances;

  node_view.PushBack('{');

  for (const auto& unit : units) {
    EntityDataSwitch(node_view, unit, attributes, equipment, tags, chances);
  }

  if (!attributes.empty()) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    TranslateAttributes(node_view, attributes);
  }
  if (!equipment.empty()) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    TranslateEquipment(node_view, equipment);
  }
  if (!tags.empty()) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    TranslateTags(node_view, tags);
  }
  if (!chances.empty()) {
    if (node_view.Result().back() != '{') {
      node_view.PushBack(',');
    }

    TranslateChances(node_view, chances);
  }

  node_view.PushBack('}');
}

void TranslateItemData(NodeView& node_view, const std::vector<DataUnit>& units, std::string_view separator) {
  std::vector<PotionUnit> potion_contents;
  std::vector<AttributeModifier> attribute_modifiers;
  bool comma_required = false;
  std::size_t initial_size;

  for (const auto& unit : units) {
    if (!comma_required) {
      initial_size = node_view.Result().size();
    }

    ItemDataSwitch(node_view, unit, separator, potion_contents, attribute_modifiers, comma_required);

    if (!comma_required && node_view.Result().size() > initial_size) {
      comma_required = true;
    }
  }

  if (!potion_contents.empty()) {
    if (comma_required) {
      node_view.PushBack(',');
    }

    TranslatePotionContents(node_view, potion_contents, separator);
    comma_required = true;
  }
  if (!attribute_modifiers.empty()) {
    if (comma_required) {
      node_view.PushBack(',');
    }

    TranslateAttributeModifiers(node_view, attribute_modifiers, separator);
  }
}

void TranslateParticleData(NodeView& node_view, const std::vector<DataUnit>& units) {
  node_view.PushBack('{');

  for (const auto& unit : units) {
    ParticleDataSwitch(node_view, unit);
  }

  node_view.PushBack('}');
}
