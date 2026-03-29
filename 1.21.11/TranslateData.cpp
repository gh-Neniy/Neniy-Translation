#include "Aux.hpp"
#include "TranslateText.hpp"
#include "TranslateData.hpp"

#include <format>

namespace {
  constexpr std::string_view hide_content = "tooltip_display={hidden_components:[\"attribute_modifiers\",\"enchantments\",\"unbreakable\",\"can_place_on\",\"potion_contents\"]}";

  void AppendUnit(std::string& result, std::string_view unit_text) {
    if (!result.empty()) {
      result.push_back(',');
    }

    result.append(unit_text);
  }

  std::string TranslateLore(const std::vector<Text>& lore, std::string_view source_code) {
    std::string result;

    for (int i = 0; i < lore.size(); ++i) {
      AppendUnit(result, TranslateText(lore[i], source_code));
    }

    return std::format("[{}]", result);
  }

  std::string TranslateCanPlaceOn(BaseToken value, std::string_view source_code, char separator) {
    return std::format("can_place_on{}{{\"blocks\":\"{}\"}}", separator, Extract(source_code, value));
  }

  std::string TranslateEnchantments(const ListType& list, std::string_view source_code, char separator) {
    std::string result = std::format("enchantments{}{{", separator);

    for (int i = 0; i < list.size(); ++i) {
      result.append(std::format("\"{}\":{}", Extract(source_code, list[i].key), Extract(source_code, list[i].value)));
    }

    result.push_back('}');
    return result;
  }

  std::string TranslateItemData(const std::vector<DataUnit>& units, std::string_view source_code, char separator) {
    std::string result;

    for (int i = 0; i < units.size(); ++i) {
      switch (units[i].key.type) {
        case TokenType::CanPlaceOn:
          AppendUnit(result, TranslateCanPlaceOn(std::get<BaseToken>(units[i].value), source_code, separator));
          break;
        case TokenType::Enchantments:
          AppendUnit(result, TranslateEnchantments(std::get<ListType>(units[i].value), source_code, separator));
          break;
        case TokenType::Hide:
          AppendUnit(result, hide_content);
          break;
        case TokenType::Lore: {
          const auto& lore = std::get<std::vector<Text>>(units[i].value);

          AppendUnit(result, std::format("lore{}{}", separator, TranslateLore(lore, source_code)));
          break;
        }
        case TokenType::Name: {
          const auto& name = std::get<Text>(units[i].value);

          AppendUnit(result, std::format("custom_name{}{}", separator, TranslateText(name, source_code)));
          break;
        }
        case TokenType::Potion:
          AppendUnit(result, std::format(
            "potion_contents{}{{potion:{}}}",
            separator, Extract(source_code, std::get<BaseToken>(units[i].value))
          ));
          break;
        case TokenType::PotionColor:
          AppendUnit(result, std::format(
            "potion_contents{}{{custom_color:{}}}",
            separator, Extract(source_code, std::get<BaseToken>(units[i].value))
          ));
          break;
        case TokenType::Shine:
          AppendUnit(result, std::format("enchantment_glint_override{}true", separator));
          break;
        case TokenType::Stack:
          AppendUnit(result, std::format("max_stack_size{}{}", separator, Extract(source_code, std::get<BaseToken>(units[i].value))));
          break;
        case TokenType::Unbreakable:
          AppendUnit(result, "unbreakable={}");
          break;
        default:
          throw std::runtime_error(std::format("Translation error - unknown key {} in item data", Extract(source_code, units[i].key)));
      }
    }

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
    std::string attribute_units;

    for (int i = 0; i < attributes.size(); ++i) {
      switch (attributes[i].key.type) {
        case TokenType::Health:
          AppendUnit(attribute_units, std::format("{{id:\"minecraft:max_health\",base:{}}}", attributes[i].value));
          break;
        case TokenType::NoKnockback:
          AppendUnit(attribute_units, std::format (
            "{{id:\"minecraft:knockback_resistance\",base:{}}}",
            attributes[i].value
          ));

          break;
        default:
          throw std::logic_error(std::format("Internal translation error - unknown attribute {}", Extract(source_code, attributes[i].key)));
      }
    }

    return std::format("equipment:{{{}}}", attribute_units);
  }

  std::string TranslateEquipmentUnit( const IdWithDataPtr& id_with_data_ptr, std::string_view unit_name,
                                      std::string_view source_code) {
    const auto& data_units = id_with_data_ptr->units;

    std::string equipment_unit = std::format (
      "{}:{{id:\"{}\"",
      unit_name,
      Extract(source_code, id_with_data_ptr->identifier)
    );

    if (data_units.empty()) {
      equipment_unit.push_back('}');
      return equipment_unit;
    }
    
    equipment_unit.append(",components:{");
    equipment_unit.append(TranslateItemData(data_units, source_code,':'));
    equipment_unit.append("}}");

    return equipment_unit;
  }

  std::string TranslateEquipment(const std::vector<Equipment>& equipment, std::string_view source_code) {
    std::string result;

    for (int i = 0; i < equipment.size(); ++i) {
      switch (equipment[i].key.type) {
        case TokenType::Chest:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "chest", source_code));
          break;
        case TokenType::Feet:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "feet", source_code));
          break;
        case TokenType::Head:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "head", source_code));
          break;
        case TokenType::LeftHand:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "offhand", source_code));
          break;
        case TokenType::Legs:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "legs", source_code));
          break;
        case TokenType::RightHand:
          AppendUnit(result, TranslateEquipmentUnit(equipment[i].value, "mainhand", source_code));
          break;
        default:
          throw std::logic_error(std::format(
            "Internal translation error - unknown equipment key {}", Extract(source_code, equipment[i].key)
          ));
      }
    }

    return std::format("equipment:{{{}}}", result);
  }

  void ProcessUnit( const DataUnit& unit, std::string& result, std::vector<Attribute>& attributes,
                    std::vector<Equipment>& equipment, std::string_view source_code) {
    switch (unit.key.type) {
      case TokenType::CanGrab:
        AppendUnit(result, "CanPickUpLoot:true");
        break;
      case TokenType::Chest:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Feet:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Head:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Health: {
        auto points = std::get<BaseToken>(unit.value);
        attributes.push_back({unit.key, Extract(source_code, points)});

        AppendUnit(result, std::format("Health:{}", Extract(source_code, points)));
        break;
      }
      case TokenType::Invulnerable:
        AppendUnit(result, "Invulnerable:true");
        break;
      case TokenType::LeftHand:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Legs:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Name: {
        const auto& text = std::get<Text>(unit.value);

        AppendUnit(result, std::format("CustomName:{}", TranslateText(text, source_code)));
        break;
      }
      case TokenType::NameVisible: {
        auto value = std::get<BaseToken>(unit.value);

        AppendUnit(result, std::format("CustomNameVisible:{}", Extract(source_code, value)));
        break;
      }
      case TokenType::NoAI:
        AppendUnit(result, "NoAI:true");
        break;
      case TokenType::NoDespawn:
        AppendUnit(result, "PersistenceRequired:true");
        break;
      case TokenType::NoKnockback:
        attributes.emplace_back(unit.key, "10.0");
        break;
      case TokenType::RightHand:
        equipment.emplace_back(unit.key, get<IdWithDataPtr>(unit.value));
        break;
      case TokenType::Silent:
        AppendUnit(result, "Silent:true");
        break;
      default:
        throw std::runtime_error(std::format("Translation error - unknown key {} in entity data", Extract(source_code, unit.key)));
    }
  }

  std::string TranslateEntityData(const std::vector<DataUnit>& units, std::string_view source_code) {
    std::vector<Attribute> attributes;
    std::vector<Equipment> equipment;

    std::string result;

    for (int i = 0; i < units.size(); ++i) {
      ProcessUnit(units[i], result, attributes, equipment, source_code);
    }

    if (!attributes.empty()) {
      AppendUnit(result, TranslateAttributes(attributes, source_code));
    }
    if (!equipment.empty()) {
      AppendUnit(result, TranslateEquipment(equipment, source_code));
    }

    return result;
  }
}

// without braces
std::string TranslateData(const std::vector<DataUnit>& units, std::string_view text, bool item_data) {
  if (item_data) {
    return TranslateItemData(units, text, '=');
  }
  return TranslateEntityData(units, text);
}
