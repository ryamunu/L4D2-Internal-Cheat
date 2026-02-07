#pragma once
#include "../SDK.h"
#include <map>
#include <vector>

enum struct EGroupType {
  INFECTED,
  CTERRORPLAYER,
  SPECIAL_INFECTED,
  WITCH,
  NON_AI,
  ITEM,
  TANK,
  MAX
};

class CEntityCache {
private:
  std::map<EGroupType, std::vector<IClientEntity *>> m_vecGroups;
  void UpdateFriends();

public:
  C_TerrorPlayer *m_pLocal = nullptr;
  C_BaseCombatWeapon *m_pLocalWeapon = nullptr;
  C_BaseEntity *m_pObservedTarget = nullptr;

  void Fill();
  void Clear();
  const std::vector<IClientEntity *> &GetGroup(const EGroupType &Group);

  bool Friends[129] = {false};
};

inline CEntityCache gEntityCache;