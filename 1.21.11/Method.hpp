#pragma once

#include "asset/Loader.hpp"
#include "trans/NodeView.hpp"

#include <filesystem>

extern "C" {
  void TranslateAdvancement(NodeView&);

  void TranslateAttribute(NodeView&);

  void TranslateBossbarAdd(NodeView&);

  void TranslateBossbarSet(NodeView&);

  void TranslateBossbarRemove(NodeView&);

  void TranslateClear(NodeView&);

  void TranslateClone(NodeView&);

  void TranslateDamage(NodeView&);

  void TranslateData(NodeView&);

  void TranslateEffect(NodeView&);

  void TranslateExecute(NodeView&, const Loader&, const std::filesystem::path&);

  void TranslateFill(NodeView&);

  void TranslateFunction(NodeView&, const std::filesystem::path&);

  void TranslateGamemode(NodeView&);

  void TranslateGamerule(NodeView&);

  void TranslateGive(NodeView&);

  void TranslateKill(NodeView&);

  void TranslateNative(NodeView&);

  void TranslateParticle(NodeView&);

  void TranslatePlaysound(NodeView&);

  void TranslateSay(NodeView&);

  void TranslateScoreboardObjectivesAdd(NodeView&);

  void TranslateScoreboardObjectivesSet(NodeView&);

  void TranslateScoreboardPlayers(NodeView&);

  void TranslateSetblock(NodeView&);

  void TranslateSpawnpoint(NodeView&);

  void TranslateSpectate(NodeView&);

  void TranslateStopsound(NodeView&);

  void TranslateSummon(NodeView&);

  void TranslateTag(NodeView&);

  void TranslateTeamAdd(NodeView&);

  void TranslateTeamJoin(NodeView&);

  void TranslateTeamModify(NodeView&);

  void TranslateTellraw(NodeView&);

  void TranslateTime(NodeView&);

  void TranslateTitle(NodeView&);

  void TranslateTp(NodeView&);
}
