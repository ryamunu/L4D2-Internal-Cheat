#include "GameUtil.h"

void CGlobal_GameUtil::FixMovement(const Vector vAngle, CUserCmd *cmd) {
  Vector vMove = {cmd->forwardmove, cmd->sidemove, cmd->upmove}, vMoveAng;
  U::Math.VectorAngles(vMove, vMoveAng);

  const float flSpeed = ::sqrtf(vMove.x * vMove.x + vMove.y * vMove.y);
  const float flYaw = DEG2RAD(vAngle.y - cmd->viewangles.y + vMoveAng.y);

  cmd->forwardmove = (::cosf(flYaw) * flSpeed);
  cmd->sidemove = (::sinf(flYaw) * flSpeed);
}

void CGlobal_GameUtil::Trace(const Vector &start, const Vector &end,
                             unsigned int mask, ITraceFilter *filter,
                             trace_t *trace) {
  Ray_t ray = {start, end};
  I::EngineTrace->TraceRay(ray, mask, filter, trace);
}

bool CGlobal_GameUtil::W2S(const Vector vWorld, Vector &vScreen) {
  return !(I::DebugOverlay->ScreenPosition(vWorld, vScreen));
}

bool CGlobal_GameUtil::IsOnScreen(const Vector vWorld) {
  Vector vScreen;
  return W2S(vWorld, vScreen);
}

bool CGlobal_GameUtil::IsValidTeam(const int nTeam) {
  return ((nTeam == TEAM_SURVIVOR) || (nTeam == TEAM_INFECTED));
}

bool CGlobal_GameUtil::IsInfectedAlive(const int nSolidFlags,
                                       const int nSequence, int classindex) {
  // Simplified: Let's use basic health and solid flags as fallback
  if (nSolidFlags & FSOLID_NOT_SOLID)
    return false;

  // In L4D2, dead sequences usually start above 300
  if (nSequence >= 305)
    return false;

  return true;
}

bool CGlobal_GameUtil::IsInvalid(C_TerrorPlayer *pLocal,
                                 IClientEntity *target) {
  if (!target)
    return false;
  int hit = HITGROUP_HEAD;
  ClientClass *pCC = target->GetClientClass();
  if (!pCC)
    return true;
  int classId = pCC->m_ClassID;

  if (classId == G::ClassID.Witch) {
    if (!target->As<C_BaseEntity *>()->IsAlive())
      return true;
  } else if (classId == G::ClassID.Tank || classId == G::ClassID.Boomer ||
             classId == G::ClassID.Jockey || classId == G::ClassID.Smoker ||
             classId == G::ClassID.Hunter || classId == G::ClassID.Spitter ||
             classId == G::ClassID.Charger) {
    C_BaseEntity *pEnt = target->As<C_BaseEntity *>();
    if (!pEnt || !pEnt->IsAlive())
      return true;

    // Additional check for player-controlled entities
    if (classId == G::ClassID.CTerrorPlayer) {
      C_TerrorPlayer *pPlayer = target->As<C_TerrorPlayer *>();
      if (pPlayer && pPlayer->deadflag())
        return true;
    }
  } else if (classId == G::ClassID.Infected) {
    C_BaseEntity *pEnt = target->As<C_BaseEntity *>();
    if (!pEnt || !pEnt->IsAlive())
      return true;
  } else if (classId == G::ClassID.CTerrorPlayer ||
             classId == G::ClassID.SurvivorBot) {
    C_TerrorPlayer *pPlayer = target->As<C_TerrorPlayer *>();
    if (!pPlayer || !pPlayer->IsAlive() || pPlayer->deadflag())
      return true;
  }
  /*
  C_BaseAnimating* pAnimating = target->As<C_BaseAnimating*>();
  Vector entityposition;
  if (!pAnimating->GetHitboxPositionByGroup(hit, entityposition))
  {
          return true;
  }
  */
  return false;
}
Color CGlobal_GameUtil::GetHealthColor(const int nHealth,
                                       const int nMaxHealth) {
  if (nHealth > nMaxHealth)
    return {44u, 130u, 201u, 255u};

  const int nCurHP = U::Math.Max(0, U::Math.Min(nHealth, nMaxHealth));

  return {U::Math.Min((510 * (nMaxHealth - nCurHP)) / nMaxHealth, 200),
          U::Math.Min((510 * nCurHP) / nMaxHealth, 200), 0u, 255u};
}

IMaterial *CGlobal_GameUtil::CreateMaterial(const char *const szVars) {
  static int nCreated = 0;

  char szOut[DT_MAX_STRING_BUFFERSIZE];
  sprintf_s(szOut, sizeof(szOut), _("pol_mat_%i.vmt"), nCreated++);

  char szMat[DT_MAX_STRING_BUFFERSIZE];
  sprintf_s(szMat, sizeof(szMat), szVars);

  KeyValues *pKvals = new KeyValues;

  G::KeyVals.Init(pKvals, (char *)szOut);
  G::KeyVals.LoadFromBuffer(pKvals, szOut, szMat);

  IMaterial *pMat = I::MaterialSystem->CreateMaterial(szOut, pKvals);

  if (!IsErrorMaterial(pMat))
    pMat->AddRef();

  return pMat;
}