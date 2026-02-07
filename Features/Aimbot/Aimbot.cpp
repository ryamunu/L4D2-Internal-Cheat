#include "Aimbot.h"
#include "../../SDK/EntityCache/entitycache.h"
#include "../../SDK/L4D2/Interfaces/ClientEntityList.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"
#include "../../SDK/L4D2/Interfaces/EngineTrace.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/ModelInfo.h"
#include "../../Util/Math/Math.h"
#include "../../Util/Util.h"
#include "../Vars.h"
#include <algorithm>
#include <vector>

// Layered bone selection for 100% accuracy across all models
bool GetUniversalHitbox(C_BaseEntity *pEntity, int selection, Vector &vOut) {
  C_BaseAnimating *pAnim = pEntity->As<C_BaseAnimating *>();
  if (!pAnim)
    return false;

  // 0: Head, 1: Neck, 2: Chest, 3: Stomach, 4: Pelvis
  int groupMap[] = {1, 1, 2, 3,
                    3}; // Hitbox Groups: 1=Head, 2=Chest, 3=Stomach/Pelvis
  const char *nameMap[] = {"head", "neck", "spine2", "spine1", "pelvis"};
  const char *altNameMap[] = {"Head", "Neck", "Spine_2", "Spine_1", "Pelvis"};
  int indexFallback[] = {10, 9, 5, 3, 2}; // Common L4D2 indices

  // Layer 1: Hitbox Groups (Engine Native)
  if (pAnim->GetHitboxPositionByGroup(groupMap[selection], vOut))
    return true;

  // Layer 2: Name Search (Professional model-independent)
  if (pAnim->GetHitboxPositionByName(nameMap[selection], vOut))
    return true;
  if (pAnim->GetHitboxPositionByName(altNameMap[selection], vOut))
    return true;

  // Layer 3: Index Fallback (Last resort)
  if (pAnim->GetHitboxPosition(indexFallback[selection], vOut))
    return true;

  // Final Layer: World Center
  vOut = pEntity->WorldSpaceCenter();
  return true;
}

void CFeatures_Aimbot::Run(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  if (!Vars::Aimbot::Enabled || !pLocal || !pLocal->IsAlive())
    return;

  bool bIsAiming = (Vars::Aimbot::AimKey.m_Var == 0) ||
                   (GetAsyncKeyState(Vars::Aimbot::AimKey.m_Var) & 0x8000);
  if (!bIsAiming && !Vars::Aimbot::AutoFire)
    return;

  int targetIndex = GetBestTarget(pLocal, pCmd);
  if (targetIndex == -1)
    return;

  IClientEntity *pTargetEnt = I::ClientEntityList->GetClientEntity(targetIndex);
  if (!pTargetEnt)
    return;

  C_BaseEntity *pBaseTarget = pTargetEnt->As<C_BaseEntity *>();
  if (!pBaseTarget)
    return;

  Vector vTargetPos;
  if (!GetUniversalHitbox(pBaseTarget, Vars::Aimbot::Bone, vTargetPos))
    return;

  if (Vars::Aimbot::Prediction) {
    Vector vVel;
    pBaseTarget->EstimateAbsVelocity(vVel);
    INetChannelInfo *nci = I::EngineClient->GetNetChannelInfo();
    vTargetPos += (vVel * (nci ? (nci->GetAvgLatency(0) + nci->GetAvgLatency(1))
                               : 0.0f + GetLerpTime()));
  }

  Vector vAngle = U::Math.CalcAngle(pLocal->EyePosition(), vTargetPos);
  U::Math.ClampAngles(vAngle);

  if (Vars::Aimbot::RSC) {
    Vector vPunch = pLocal->m_vecPunchAngle();
    vAngle.x -= (vPunch.x * (Vars::Aimbot::RSCFactorRateX / 50.0f));
    vAngle.y -= (vPunch.y * (Vars::Aimbot::RSCFactorRateY / 50.0f));
    U::Math.ClampAngles(vAngle);
  }

  if (Vars::Aimbot::Smoothing && !Vars::Aimbot::Silent) {
    Vector vView;
    I::EngineClient->GetViewAngles(vView);
    Vector vDelta = vAngle - vView;
    U::Math.ClampAngles(vDelta);
    vAngle = vView + (vDelta / (std::max)(1.0f, Vars::Aimbot::SmoothFactor));
    U::Math.ClampAngles(vAngle);
  }

  if (Vars::Aimbot::Silent) {
    pCmd->viewangles = vAngle;
    G::Util.FixMovement(vAngle, pCmd);
  } else {
    I::EngineClient->SetViewAngles(vAngle);
  }

  if (Vars::Aimbot::AutoFire)
    pCmd->buttons |= IN_ATTACK;
}

int CFeatures_Aimbot::GetBestTarget(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  int bestTarget = -1;
  float bestFOV = Vars::Aimbot::Fov;
  Vector vEye = pLocal->EyePosition();
  Vector vAngles;
  I::EngineClient->GetViewAngles(vAngles);

  auto ScanGroup = [&](EGroupType group) {
    const auto &entities = gEntityCache.GetGroup(group);
    if (group == EGroupType::INFECTED && Vars::Aimbot::IgnoreInfected)
      return;
    if (group == EGroupType::CTERRORPLAYER && Vars::Aimbot::IgnoreSurvivors)
      return;
    if (group == EGroupType::WITCH && Vars::Aimbot::IgnoreWitch)
      return;
    if (group == EGroupType::TANK && Vars::Aimbot::IgnoreTank)
      return;
    if (group == EGroupType::SPECIAL_INFECTED && Vars::Aimbot::IgnoreSpecial)
      return;

    for (auto pEnt : entities) {
      if (!pEnt || pEnt->IsDormant() || pEnt->entindex() == pLocal->entindex())
        continue;
      C_BaseEntity *pBase = pEnt->As<C_BaseEntity *>();
      if (!pBase || !pBase->IsAlive())
        continue;

      if (group == EGroupType::SPECIAL_INFECTED || group == EGroupType::TANK) {
        C_TerrorPlayer *pP = pBase->As<C_TerrorPlayer *>();
        if (pP && pP->m_isGhost())
          continue;
      }

      Vector vPos;
      if (!GetUniversalHitbox(pBase, Vars::Aimbot::Bone, vPos))
        continue;

      float fov = U::Math.GetFovBetween(vAngles, U::Math.CalcAngle(vEye, vPos));
      if (fov < bestFOV) {
        if (Vars::Aimbot::VisibilityCheck &&
            !IsVisible(pLocal, vEye, vPos, pBase))
          continue;
        bestFOV = fov;
        bestTarget = pEnt->entindex();
      }
    }
  };

  ScanGroup(EGroupType::CTERRORPLAYER);
  ScanGroup(EGroupType::SPECIAL_INFECTED);
  ScanGroup(EGroupType::INFECTED);
  ScanGroup(EGroupType::TANK);
  ScanGroup(EGroupType::WITCH);

  return bestTarget;
}

bool CFeatures_Aimbot::IsVisible(C_TerrorPlayer *pLocal, const Vector &vStart,
                                 const Vector &vEnd, IClientEntity *pTarget) {
  if (!pTarget)
    return true;
  trace_t tr;
  Ray_t ray;
  ray.Init(vStart, vEnd);
  CTraceFilter filter;
  filter.pSkip = pLocal;
  I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
  return (tr.m_pEnt == pTarget || tr.fraction > 0.98f);
}

float CFeatures_Aimbot::GetLerpTime() {
  static auto cl_interp = I::Cvars->FindVar("cl_interp");
  static auto cl_interp_ratio = I::Cvars->FindVar("cl_interp_ratio");
  static auto cl_updaterate = I::Cvars->FindVar("cl_updaterate");
  if (!cl_interp || !cl_interp_ratio || !cl_updaterate)
    return 0.01f;
  return (std::max)((float)cl_interp->GetFloat(),
                    (float)cl_interp_ratio->GetFloat() /
                        cl_updaterate->GetFloat());
}
