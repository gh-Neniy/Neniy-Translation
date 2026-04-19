#include "main/Concat.hpp"
#include "Aux.hpp"
#include "TranslateText.hpp"
#include <cwchar>
#include "TranslateData.hpp"

using namespace std::literals;

namespace {
  constexpr std::string_view hide_content = "tooltip_display={hidden_components:[\"attribute_modifiers\",\"enchantments\",\"unbreakable\",\"can_place_on\",\"potion_contents\"]}";

  std::string TranslateLore(const std::vector<Text>& lore, std::string_view source_code) {
    std::string result = "[";

    for (int i = 0; i < lore.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      result.append(TranslateText(lore[i], source_code));
    }

    result.push_back(']');
    return result;
  }

  std::string TranslateEnchantments(const ListType& list, std::string_view source_code) {
    std::string result = "{";

    for (int i = 0; i < list.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      result.append(Concat(
        "\""sv, Extract(source_code, list[i].key), "\":"sv, Extract(source_code, list[i].value)
      ));
    }

    result.push_back('}');
    return result;
  }

  struct Attribute {
    Token key;
    std::string_view value;
  };

  struct Equipment {
    Token key;
    const IdWithDataPtr& value;
  };

  std::string TranslateAttributes(const std::vector<Attribute>& attributes, std::string_view source_code) {
    std::string result = "attributes:[";

    for (int i = 0; i < attributes.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      switch (attributes[i].key.type) {
        case TokenType::Health:
          result.append(Concat("{id:\"minecraft:max_health\",base:"sv, attributes[i].value, "}"sv));
          break;
        case TokenType::Stability:
          result.append(Concat("{id:\"minecraft:knockback_resistance\",base:"sv, attributes[i].value, "}"sv));
          break;
        default:
          throw std::logic_error(Concat("Internal translation error - unknown attribute "sv, Extract(source_code, attributes[i].key)));
      }
    }

    result.push_back(']');

    return result;
  }

  std::string TranslateEquipmentUnit( const IdWithDataPtr& id_with_data_ptr, std::string_view unit_name,
                                      std::string_view source_code) {
    std::string equipment_unit = Concat(
      unit_name,
      ":{id:\""sv,
      Extract(source_code, id_with_data_ptr->identifier),
      "\""sv
    );

    const auto& data_units = id_with_data_ptr->units;

    if (data_units.empty()) {
      equipment_unit.push_back('}');
      return equipment_unit;
    }
    
    equipment_unit.append(",components:{");
    equipment_unit.append(TranslateItemData(data_units, source_code, ":"sv));
    equipment_unit.append("}}");

    return equipment_unit;
  }

  std::string TranslateEquipment(const std::vector<Equipment>& equipment, std::string_view source_code) {
    std::string result = "equipment:{";

    for (int i = 0; i < equipment.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      switch (equipment[i].key.type) {
        case TokenType::Chest:
          result.append(TranslateEquipmentUnit(equipment[i].value, "chest", source_code));
          break;
        case TokenType::Feet:
          result.append(TranslateEquipmentUnit(equipment[i].value, "feet", source_code));
          break;
        case TokenType::Head:
          result.append(TranslateEquipmentUnit(equipment[i].value, "head", source_code));
          break;
        case TokenType::LeftHand:
          result.append(TranslateEquipmentUnit(equipment[i].value, "offhand", source_code));
          break;
        case TokenType::Legs:
          result.append(TranslateEquipmentUnit(equipment[i].value, "legs", source_code));
          break;
        case TokenType::RightHand:
          result.append(TranslateEquipmentUnit(equipment[i].value, "mainhand", source_code));
          break;
        default:
          throw std::logic_error(Concat(
            "Internal translation error - unknown equipment key "sv, Extract(source_code, equipment[i].key)
          ));
      }
    }

    result.push_back('}');

    return result;
  }

  std::string TranslateTags(const std::vector<std::string_view>& tags, std::string_view source_code) {
    std::string result = "Tags:[";

    for (int i = 0; i < tags.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      result.append(tags[i]);
    }

    result.push_back(']');

    return result;
  }

  std::string TranslateNumericList(const ListType& list, std::string_view source_code) {
    std::string result = "[";

    for (int i = 0; i < list.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      result.append(Extract(source_code, list[i].key));
    }

    result.push_back(']');

    return result;
  }

  void AppendUnit(std::string& result, std::string_view to_append) {
    if (!result.empty()) {
      result.push_back(',');
    }

    result.append(to_append);
  }

  void BlockDataSwitch(const DataUnit& unit, std::string& result, std::string_view source_code, std::string_view separator) {
    switch (unit.key.type) {
      case TokenType::East:
        AppendUnit(result, Concat("east"sv, separator, "true"sv));
        break;
      case TokenType::Facing:
        AppendUnit(result, Concat("facing"sv, separator, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Half:
        AppendUnit(result, Concat("half"sv, separator, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Level:
        AppendUnit(result, Concat("level"sv, separator, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Lit:
        AppendUnit(result, Concat("lit"sv, separator, "true"sv));
        break;
      case TokenType::North:
        AppendUnit(result, Concat("north"sv, separator, "true"sv));
        break;
      case TokenType::Open:
        AppendUnit(result, Concat("open"sv, separator, "true"sv));
        break;
      case TokenType::Powered:
        AppendUnit(result, Concat("powered"sv, separator, "true"sv));
        break;
      case TokenType::South:
        AppendUnit(result, Concat("south"sv, separator, "true"sv));
        break;
      case TokenType::West:
        AppendUnit(result, Concat("west"sv, separator, "true"sv));
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, unit.key), " in block data"sv));
    }
  }

  void EntityDataSwitch(const DataUnit& unit, std::string& result, std::string_view source_code,
                        std::vector<Attribute>& attributes, std::vector<Equipment>& equipment, std::vector<std::string_view>& tags) {
    switch (unit.key.type) {
      case TokenType::CanGrab:
        AppendUnit(result, "CanPickUpLoot:1b");
        break;
      case TokenType::Chest:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Crit:
        AppendUnit(result, "crit:1b");
        break;
      case TokenType::Feet:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Head:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Health: {
        auto points = std::get<BaseToken>(unit.value);
        attributes.emplace_back(unit.key, Extract(source_code, points));

        AppendUnit(result, Concat("Health:"sv, Extract(source_code, points)));
        break;
      }
      case TokenType::Id: {
        if (!result.empty()) {
          result.push_back(',');
        }

        const auto& id_with_data_ptr = std::get<IdWithDataPtr>(unit.value);

        result.append(Concat("BlockState:{Name:\""sv, Extract(source_code, id_with_data_ptr->identifier)));

        const auto& data_units = id_with_data_ptr->units;
        if (!data_units.empty()) {
          result.append(Concat (
            ",Properties:{"sv, Sv(TranslateBlockData(data_units, source_code, ":")), "}"sv
          ));
        }

        result.append("\"}");
        break;
      }
      case TokenType::InGround:
        AppendUnit(result, "inGround:1b");
        break;
      case TokenType::Invisible:
        AppendUnit(result, "Invisible:1b");
        break;
      case TokenType::Invulnerable:
        AppendUnit(result, "Invulnerable:1b");
        break;
      case TokenType::Item: {
        if (!result.empty()) {
          result.push_back(',');
        }

        const auto& id_with_data_ptr = std::get<IdWithDataPtr>(unit.value);

        result.append(Concat("nbt={item:{id:"sv, Extract(source_code, id_with_data_ptr->identifier)));

        if (!id_with_data_ptr->units.empty()) {
          result.push_back(',');
          result.append(TranslateItemData(id_with_data_ptr->units, source_code, ":"));
        }

        result.append("}}");
        break;
      }
      case TokenType::LeftHand:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Legs:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::LootTable:
        AppendUnit(result, Concat("DeathLootTable:"sv, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Name: {
        const auto& text = std::get<Text>(unit.value);

        AppendUnit(result, Concat("CustomName:"sv, Sv(TranslateText(text, source_code))));
        break;
      }
      case TokenType::NameVisible: {
        AppendUnit(result, "CustomNameVisible:1b");
        break;
      }
      case TokenType::NoAI:
        AppendUnit(result, "NoAI:1b");
        break;
      case TokenType::NoDespawn:
        AppendUnit(result, "PersistenceRequired:1b");
        break;
      case TokenType::NoGravity:
        AppendUnit(result, "NoGravity:1b");
        break;
      case TokenType::PickupDelay:
        AppendUnit(result, Concat("PickupDelay:"sv, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Stability: {
        attributes.emplace_back(unit.key, Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      }
      case TokenType::RightHand:
        equipment.emplace_back(unit.key, std::get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Rotation:
        AppendUnit(result, Concat("Rotation:"sv, Sv(TranslateNumericList(std::get<ListType>(unit.value), source_code))));
        break;
      case TokenType::Silent:
        AppendUnit(result, "Silent:1b");
        break;
      case TokenType::Tag:
        tags.push_back(Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      case TokenType::TeleportDuration:
        AppendUnit(result, Concat("teleport_duration:"sv, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, unit.key), " in entity data"sv));
    }
  }

  struct PotionUnit {
    Token key;
    std::string_view value;
  };

  struct AttributeModifier {
    Token key;
    std::string_view value;
  };

  void ItemDataSwitch(const DataUnit& unit, std::string& result, std::string_view source_code, std::string_view separator,
                      std::vector<PotionUnit>& potion_contents, std::vector<AttributeModifier>& attribute_modifiers) {
    switch (unit.key.type) {
      case TokenType::AttackDamage:
        attribute_modifiers.emplace_back(unit.key, Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      case TokenType::AttackSpeed:
        attribute_modifiers.emplace_back(unit.key, Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      case TokenType::CanPlaceOn:
        AppendUnit(result, Concat (
          "can_place_on"sv, separator, "{\"blocks\":\""sv, Extract(source_code, std::get<BaseToken>(unit.value)), "\"}"sv
        ));

        break;
      case TokenType::Enchantments:
        AppendUnit(result, Concat (
          "enchantments"sv, separator, Sv(TranslateEnchantments(std::get<ListType>(unit.value), source_code))
        ));

        break;
      case TokenType::Hide:
        AppendUnit(result, hide_content);
        break;
      case TokenType::Lore: {
        const auto& lore = std::get<std::vector<Text>>(unit.value);

        AppendUnit(result, Concat("lore"sv, separator, Sv(TranslateLore(lore, source_code))));
        break;
      }
      case TokenType::Name: {
        const auto& name = std::get<Text>(unit.value);

        AppendUnit(result, Concat("custom_name"sv, separator, Sv(TranslateText(name, source_code))));
        break;
      }
      case TokenType::Potion:
        potion_contents.emplace_back(unit.key, Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      case TokenType::PotionColor:
        potion_contents.emplace_back(unit.key, Extract(source_code, std::get<BaseToken>(unit.value)));
        break;
      case TokenType::Shine:
        AppendUnit(result, Concat("enchantment_glint_override"sv, separator, "1b"sv));
        break;
      case TokenType::Stack:
        AppendUnit(result, Concat("max_stack_size"sv, separator, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::Tag:
        AppendUnit(result, Concat (
          "components:{custom_data:{tag:"sv, Extract(source_code, std::get<BaseToken>(unit.value)), "}}"sv
        ));
        break;
      case TokenType::Unbreakable:
        AppendUnit(result, "unbreakable={}");
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, unit.key), " in item data"sv));
    }
  }

  void ParticleDataSwitch(const DataUnit& unit, std::string& result, std::string_view source_code) {
    switch (unit.key.type) {
      case TokenType::Block:
        AppendUnit(result, Concat("item:"sv, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::FromColor:
        AppendUnit(result, Concat("from_color:"sv, Sv(TranslateNumericList(std::get<ListType>(unit.value), source_code))));
        break;
      case TokenType::Item:
        AppendUnit(result, Concat("item:"sv, Extract(source_code, std::get<IdWithDataPtr>(unit.value)->identifier)));
        break;
      case TokenType::Scale:
        AppendUnit(result, Concat("scale:"sv, Extract(source_code, std::get<BaseToken>(unit.value))));
        break;
      case TokenType::ToColor:
        AppendUnit(result, Concat("to_color:"sv, Sv(TranslateNumericList(std::get<ListType>(unit.value), source_code))));
        break;
      default:
        throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, unit.key), " in particle data"sv));
    }
  }

  std::string TranslatePotionContents(const std::vector<PotionUnit>& potion_contents, std::string_view source_code,
                                      std::string_view separator) {
    std::string result = Concat("potion_contents"sv, separator, "{"sv);

    for (int i = 0; i < potion_contents.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      switch (potion_contents[i].key.type) {
        case TokenType::Potion:
          result.append(Concat("potion:"sv, potion_contents[i].value));
          break;
        case TokenType::PotionColor:
          result.append(Concat("custom_color:"sv, potion_contents[i].value));
          break;
        default:
          throw std::logic_error (
            Concat("Internal translation error - unknown potion unit "sv, Extract(source_code, potion_contents[i].key))
          );
      }
    }

    result.push_back('}');

    return result;
  }

  std::string TranslateAttributeModifiers(const std::vector<AttributeModifier>& attribute_modifiers, std::string_view source_code,
                                          std::string_view separator) {
    std::string result = Concat("attribute_modifiers"sv, separator, "["sv);

    for (int i = 0; i < attribute_modifiers.size(); ++i) {
      if (i > 0) {
        result.push_back(',');
      }

      std::string_view attribute_name = Extract(source_code, attribute_modifiers[i].key);

      result.append(Concat (
        "{type:\""sv, attribute_name,
        "\",amount:"sv, attribute_modifiers[i].value,
        ",operator:\"add_value\",slot:\"mainhand\",id:\"base_"sv, attribute_name, "\"}"sv
      ));
    }

    result.push_back(']');

    return result;
  }
}

std::string TranslateBlockData(const std::vector<DataUnit>& units, std::string_view source_code, std::string_view separator) {
  std::string result;

  for (const auto& unit : units) {
    BlockDataSwitch(unit, result, source_code, separator); 
  }

  return result;
}

std::string TranslateEntityData(const std::vector<DataUnit>& units, std::string_view source_code) {
  std::vector<Attribute> attributes;
  std::vector<Equipment> equipment;
  std::vector<std::string_view> tags;

  std::string result;

  for (const auto& unit : units) {
    EntityDataSwitch(unit, result, source_code, attributes, equipment, tags);
  }

  if (!attributes.empty()) {
    AppendUnit(result, TranslateAttributes(attributes, source_code));
  }
  if (!equipment.empty()) {
    AppendUnit(result, TranslateEquipment(equipment, source_code));
  }
  if (!tags.empty()) {
    AppendUnit(result, TranslateTags(tags, source_code));
  }

  return result;
}

std::string TranslateItemData(const std::vector<DataUnit>& units, std::string_view source_code, std::string_view separator) {
  std::vector<PotionUnit> potion_contents;
  std::vector<AttributeModifier> attribute_modifiers;
  
  std::string result;

  for (const auto& unit : units) {
    ItemDataSwitch(unit, result, source_code, separator, potion_contents, attribute_modifiers);
  }

  if (!potion_contents.empty()) {
    AppendUnit(result, TranslatePotionContents(potion_contents, source_code, separator));
  }
  if (!attribute_modifiers.empty()) {
    AppendUnit(result, TranslateAttributeModifiers(attribute_modifiers, source_code, separator));
  }

  return result;
}

std::string TranslateParticleData(const std::vector<DataUnit>& units, std::string_view source_code) {
  std::string result;

  for (const auto& unit : units) {
    ParticleDataSwitch(unit, result, source_code);
  }

  return result;
}
