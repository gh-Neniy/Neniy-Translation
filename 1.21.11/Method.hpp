#pragma once

#include "asset/Loader.hpp"
#include "trans/NodeView.hpp"

#include <string>

extern "C" {
  std::string TranslateClear(const NodeView&);

  std::string TranslateClone(const NodeView&);

  std::string TranslateDamage(const NodeView&);

  std::string TranslateData(const NodeView&);

  std::string TranslateEffect(const NodeView&);

  std::string TranslateExecute(const NodeView&, std::string_view function_prefix, const Loader& loader);

  std::string TranslateFill(const NodeView&);

  std::string TranslateFunction(const NodeView&, std::string_view function_prefix);

  std::string TranslateGamemode(const NodeView&);

  std::string TranslateGamerule(const NodeView&);

  std::string TranslateGive(const NodeView&);

  std::string TranslateKill(const NodeView&);

  std::string TranslateNative(const NodeView&);

  std::string TranslateParticle(const NodeView&);

  std::string TranslatePlaysound(const NodeView&);

  std::string TranslateSay(const NodeView&);

  std::string TranslateScoreboardObjectivesAdd(const NodeView&);

  std::string TranslateScoreboardObjectivesSet(const NodeView&);

  std::string TranslateScoreboardPlayers(const NodeView&);

  std::string TranslateSetblock(const NodeView&);

  std::string TranslateSpawnpoint(const NodeView&);

  std::string TranslateSpectate(const NodeView&);

  std::string TranslateStopsound(const NodeView&);

  std::string TranslateSummon(const NodeView&);

  std::string TranslateTag(const NodeView&);

  std::string TranslateTeamAdd(const NodeView&);

  std::string TranslateTeamJoin(const NodeView&);

  std::string TranslateTeamModify(const NodeView&);

  std::string TranslateTellraw(const NodeView&);

  std::string TranslateTime(const NodeView&);

  std::string TranslateTp(const NodeView&);
}
