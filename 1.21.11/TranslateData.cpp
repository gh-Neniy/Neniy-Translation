#include "main/Concat.hpp"
#include "Aux.hpp"
#include "TranslateText.hpp"
#include "TranslateData.hpp"

using namespace std::literals;

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

    return Concat("["sv, Sv(result), "]"sv);
  }

  std::string TranslateCanPlaceOn(BaseToken value, std::string_view source_code, std::string_view separator) {
    return Concat("can_place_on"sv, separator, "{\"blocks\":\""sv, Extract(source_code, value), "\"}"sv);
  }

  std::string TranslateEnchantments(const ListType& list, std::string_view source_code, std::string_view separator) {
    std::string result = Concat("enchantments"sv, separator, "{"sv);

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

  std::string TranslateItemData(const std::vector<DataUnit>& units, std::string_view source_code, std::string_view separator) {
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

          AppendUnit(result, Concat("lore"sv, separator, Sv(TranslateLore(lore, source_code))));
          break;
        }
        case TokenType::Name: {
          const auto& name = std::get<Text>(units[i].value);

          AppendUnit(result, Concat("custom_name"sv, separator, Sv(TranslateText(name, source_code))));
          break;
        }
        case TokenType::Potion:
          AppendUnit(result, Concat(
            "potion_contents"sv, separator,
            "{potion:"sv, Extract(source_code, std::get<BaseToken>(units[i].value)), "}"sv
          ));
          break;
        case TokenType::PotionColor:
          AppendUnit(result, Concat(
            "potion_contents"sv, separator,
            "{custom_color:"sv, Extract(source_code, std::get<BaseToken>(units[i].value)), "}"sv
          ));
          break;
        case TokenType::Shine:
          AppendUnit(result, Concat("enchantment_glint_override"sv, separator, "true"sv));
          break;
        case TokenType::Stack:
          AppendUnit(result, Concat("max_stack_size"sv, separator, Extract(source_code, std::get<BaseToken>(units[i].value))));
          break;
        case TokenType::Unbreakable:
          AppendUnit(result, "unbreakable={}");
          break;
        default:
          throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, units[i].key), " in item data"sv));
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
          AppendUnit(attribute_units, Concat("{id:\"minecraft:max_health\",base:"sv, attributes[i].value, "}"sv));
          break;
        case TokenType::NoKnockback:
          AppendUnit(attribute_units, Concat(
            "{id:\"minecraft:knockback_resistance\",base:"sv,
            attributes[i].value,
            "}"sv
          ));

          break;
        default:
          throw std::logic_error(Concat("Internal translation error - unknown attribute "sv, Extract(source_code, attributes[i].key)));
      }
    }

    return Concat("equipment:{"sv, Sv(attribute_units), "}"sv);
  }

  std::string TranslateEquipmentUnit( const IdWithDataPtr& id_with_data_ptr, std::string_view unit_name,
                                      std::string_view source_code) {
    const auto& data_units = id_with_data_ptr->units;

    std::string equipment_unit = Concat(
      unit_name,
      ":{id:\""sv,
      Extract(source_code, id_with_data_ptr->identifier),
      "\""sv
    );

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
          throw std::logic_error(Concat(
            "Internal translation error - unknown equipment key "sv, Extract(source_code, equipment[i].key)
          ));
      }
    }

    return Concat("equipment:{"sv, Sv(result), "}"sv);
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

        AppendUnit(result, Concat("Health:"sv, Extract(source_code, points)));
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

        AppendUnit(result, Concat("CustomName:"sv, Sv(TranslateText(text, source_code))));
        break;
      }
      case TokenType::NameVisible: {
        auto value = std::get<BaseToken>(unit.value);

        AppendUnit(result, Concat("CustomNameVisible:"sv, Extract(source_code, value)));
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
        throw std::runtime_error(Concat("Translation error - unknown key "sv, Extract(source_code, unit.key), " in entity data"sv));
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
    return TranslateItemData(units, text, "="sv);
  }
  return TranslateEntityData(units, text);
}
