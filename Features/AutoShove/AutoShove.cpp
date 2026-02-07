#include "AutoShove.h"
#include "../../SDK/EntityCache/entitycache.h"
#include "../Vars.h"

// Helper to check for specials
bool IsSpecialInfected(int id) {
  return (id == G::ClassID.Hunter || id == G::ClassID.Smoker ||
          id == G::ClassID.Boomer || id == G::ClassID.Jockey ||
          id == G::ClassID.Spitter || id == G::ClassID.Charger);
}

bool CAutoShove::CanShove(C_TerrorPlayer *pLocal, C_BaseEntity *pInfected) {
  if (!pLocal || !pInfected)
    return false;

  const auto pCC = pInfected->GetClientClass();
  if (!pCC)
    return false;
  int id = pCC->m_ClassID;

  // GLOBAL FILTERS
  if (id == G::ClassID.Infected && !Vars::AutoShove::Infected)
    return false;
  if (id == G::ClassID.Witch)
    return false; // Never shove witch unless explicitly requested (user asked
                  // to remove)
  if (id == G::ClassID.Tank && !Vars::AutoShove::Tank)
    return false;

  // Special Infected
  if (IsSpecialInfected(id)) {
    if (!Vars::AutoShove::Specials)
      return false;

    // Granular
    if (id == G::ClassID.Hunter && !Vars::AutoShove::Hunter)
      return false;
    if (id == G::ClassID.Smoker && !Vars::AutoShove::Smoker)
      return false;
    if (id == G::ClassID.Jockey && !Vars::AutoShove::Jockey)
      return false;
    if (id == G::ClassID.Charger && !Vars::AutoShove::Charger)
      return false;
    if (id == G::ClassID.Boomer && !Vars::AutoShove::Boomer)
      return false;
    if (id == G::ClassID.Spitter && !Vars::AutoShove::Spitter)
      return false;
  }

  // Survivor check
  if (id == G::ClassID.CTerrorPlayer && pInfected->m_iTeamNum() == 2)
    return false;

  // Range Check
  Vector localPos = pLocal->GetAbsOrigin();
  Vector targetPos = pInfected->GetAbsOrigin();
  float dist = (localPos - targetPos).Length();

  if (dist > 80.0f)
    return false;

  return true;
}

C_BaseEntity *CAutoShove::GetBestTarget(C_TerrorPlayer *pLocal) {
  C_BaseEntity *BestEntity = nullptr;
  float BestScore = 180.0f;

  auto ProcessGroup = [&](const std::vector<IClientEntity *> &group) {
    for (const auto pEnt : group) {
      if (!pEnt)
        continue;
      C_BaseEntity *pBaseEnt = pEnt->As<C_BaseEntity *>();
      if (!pBaseEnt || !CanShove(pLocal, pBaseEnt))
        continue;

      Vector vAngle = U::Math.CalcAngle(pLocal->EyePosition(),
                                        pBaseEnt->WorldSpaceCenter());
      Vector vViewAngles;
      I::EngineClient->GetViewAngles(vViewAngles);
      float fov = U::Math.GetFovBetween(vViewAngles, vAngle);

      if (fov < BestScore) {
        BestScore = fov;
        BestEntity = pBaseEnt;
      }
    }
  };

  if (Vars::AutoShove::Infected)
    ProcessGroup(gEntityCache.GetGroup(EGroupType::INFECTED));
  if (Vars::AutoShove::Specials)
    ProcessGroup(gEntityCache.GetGroup(EGroupType::SPECIAL_INFECTED));
  if (Vars::AutoShove::Tank)
    ProcessGroup(gEntityCache.GetGroup(EGroupType::TANK));

  return BestEntity;
}

void CAutoShove::run(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  if (!Vars::AutoShove::Enabled || !pLocal || !pLocal->IsAlive())
    return;
  if (pLocal->m_iTeamNum() < 2 || pLocal->m_isGhost())
    return;

  auto pWeapon = pLocal->GetActiveWeapon();
  if (!pWeapon)
    return;

  if (I::GlobalVars->curtime < pWeapon->m_flNextSecondaryAttack())
    return;

  C_BaseEntity *pEntity = GetBestTarget(pLocal);
  if (!pEntity)
    return;

  // Check visibility using MASK_SOLID (not MASK_SHOT - traces through objects)
  trace_t trace;
  Ray_t ray;
  CTraceFilter filter;
  filter.pSkip = pLocal;
  ray.Init(pLocal->EyePosition(), pEntity->WorldSpaceCenter());
  I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

  if (trace.m_pEnt != pEntity && trace.fraction < 0.95f)
    return;

  // SILENT AIM FIX: Store original angles before aiming
  Vector vOriginalAngles = pCmd->viewangles;

  // Calculate aim angles
  Vector vAimAngles =
      U::Math.CalcAngle(pLocal->EyePosition(), pEntity->WorldSpaceCenter());
  U::Math.ClampAngles(vAimAngles);

  // Set viewangles for shove
  pCmd->viewangles = vAimAngles;

  // FIX MOVEMENT GLITCH: Rotate movement vectors back to original direction
  G::Util.FixMovement(vOriginalAngles, pCmd);

  pCmd->buttons |= IN_ATTACK2;
}
