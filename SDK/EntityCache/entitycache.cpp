#include "entitycache.h"
#include "../L4D2/Interfaces/ClientEntityList.h"
#include "../L4D2/Interfaces/EngineClient.h"
#include "../L4D2/Interfaces/IConVar.h"
#include "../L4D2/Interfaces/ModelInfo.h"
#include <cctype>
#include <cstring>

void ResolveClasses() {}

void CEntityCache::Fill() {
  Clear();
  IClientEntity *pLocalHost =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
  if (!pLocalHost)
    return;

  C_TerrorPlayer *_pLocal = pLocalHost->As<C_TerrorPlayer *>();
  if (_pLocal) {
    m_pLocal = _pLocal;
    int maxEntities = I::ClientEntityList->GetMaxEntities();

    for (int n = 1; n <= maxEntities; n++) {
      if (n == I::EngineClient->GetLocalPlayer())
        continue;

      IClientEntity *pEntity = I::ClientEntityList->GetClientEntity(n);
      if (!pEntity || pEntity->IsDormant())
        continue;

      ClientClass *pCC = pEntity->GetClientClass();
      if (!pCC)
        continue;

      int classID = pCC->m_ClassID;
      C_BaseEntity *pBaseEnt = pEntity->As<C_BaseEntity *>();
      if (!pBaseEnt)
        continue;

      // Get team number for proper classification
      int team = pBaseEnt->m_iTeamNum();

      // TEAM 2 = Survivors, TEAM 3 = Infected
      // Infected team (including player-controlled specials) go to zombie
      // groups
      if (team == 3) {
        if (!pBaseEnt->IsAlive())
          continue;

        if (classID == G::ClassID.Tank)
          m_vecGroups[EGroupType::TANK].push_back(pEntity);
        else if (classID == G::ClassID.Witch)
          m_vecGroups[EGroupType::WITCH].push_back(pEntity);
        else if (classID == G::ClassID.Infected)
          m_vecGroups[EGroupType::INFECTED].push_back(pEntity);
        else if (classID == G::ClassID.Hunter || classID == G::ClassID.Smoker ||
                 classID == G::ClassID.Boomer || classID == G::ClassID.Jockey ||
                 classID == G::ClassID.Spitter ||
                 classID == G::ClassID.Charger ||
                 classID == G::ClassID.CTerrorPlayer)
          m_vecGroups[EGroupType::SPECIAL_INFECTED].push_back(pEntity);
        continue;
      }

      // TEAM 2 = Survivors only
      if (team == 2) {
        if (pBaseEnt->IsAlive())
          m_vecGroups[EGroupType::CTERRORPLAYER].push_back(pEntity);
        continue;
      }

      // Fallback: Use engine checks for entities without proper team
      if (pBaseEnt->IsZombie()) {
        if (!pBaseEnt->IsAlive())
          continue;

        if (classID == G::ClassID.Tank)
          m_vecGroups[EGroupType::TANK].push_back(pEntity);
        else if (classID == G::ClassID.Witch)
          m_vecGroups[EGroupType::WITCH].push_back(pEntity);
        else if (classID == G::ClassID.Infected)
          m_vecGroups[EGroupType::INFECTED].push_back(pEntity);
        else
          m_vecGroups[EGroupType::SPECIAL_INFECTED].push_back(pEntity);
        continue;
      }

      // 2. CLASS-ID FALLBACKS FOR CHARACTERS
      if (classID == G::ClassID.CTerrorPlayer ||
          classID == G::ClassID.SurvivorBot || classID == G::ClassID.Infected ||
          classID == G::ClassID.Tank || classID == G::ClassID.Witch ||
          classID == G::ClassID.Hunter || classID == G::ClassID.Smoker ||
          classID == G::ClassID.Boomer || classID == G::ClassID.Jockey ||
          classID == G::ClassID.Spitter || classID == G::ClassID.Charger) {
        continue;
      }

      // 3. ACTUAL ITEMS (SUPER STRICT)
      // FAILURE PROTECTION: If it has an owner or moveparent, it's NOT a pickup
      // item.
      if (pBaseEnt->m_hOwnerEntity().IsValid() || pBaseEnt->moveparent() > 0)
        continue;

      if (classID == G::ClassID.FirstAidKit ||
          classID == G::ClassID.PainPills ||
          classID == G::ClassID.Item_Adrenaline ||
          classID == G::ClassID.WeaponSpawn ||
          classID == G::ClassID.WeaponAmmoSpawn ||
          classID == G::ClassID.WeaponMeleeSpawn ||
          classID == G::ClassID.WeaponPistolSpawn ||
          classID == G::ClassID.WeaponRifleSpawn ||
          classID == G::ClassID.WeaponShotgunSpawn ||
          classID == G::ClassID.WeaponSubMachineGunSpawn ||
          classID == G::ClassID.WeaponSniperRifleSpawn ||
          classID == 1 ||   // Molotov (L4D2 ID)
          classID == 115 || // PipeBomb (L4D2 ID)
          classID == 125    // VomitJar (L4D2 ID)
      ) {
        m_vecGroups[EGroupType::ITEM].push_back(pEntity);
        continue;
      }
    }
    UpdateFriends();
  }
}

bool IsPlayerOnSteamFriendList(IClientEntity *pPlayer) {
  player_info_t pi = {};
  if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) &&
      pi.friendsid) {
    CSteamID steamID{pi.friendsid, 1, k_EUniversePublic,
                     k_EAccountTypeIndividual};
    return I::steamfriends002->HasFriend(steamID, k_EFriendFlagImmediate);
  }
  return false;
}

void CEntityCache::UpdateFriends() {
  static size_t CurSize, OldSize;
  const auto Players = GetGroup(EGroupType::CTERRORPLAYER);
  CurSize = Players.size();
  if (CurSize != OldSize) {
    for (const auto &Player : Players)
      Friends[Player->entindex()] = IsPlayerOnSteamFriendList(Player);
  }
  OldSize = CurSize;
}

void CEntityCache::Clear() {
  m_pLocal = nullptr;
  m_pLocalWeapon = nullptr;
  m_pObservedTarget = nullptr;
  for (auto &Group : m_vecGroups)
    Group.second.clear();
}

const std::vector<IClientEntity *> &
CEntityCache::GetGroup(const EGroupType &Group) {
  return m_vecGroups[Group];
}