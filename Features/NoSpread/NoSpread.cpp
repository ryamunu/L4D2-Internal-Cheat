#include "NoSpread.h"
#include "../Vars.h"

void CFeatures_NoSpread::Run(C_TerrorPlayer *pLocal, C_TerrorWeapon *pWeapon,
                             CUserCmd *cmd) {
  static const auto pfSharedRandomFloat =
      reinterpret_cast<float (*)(const char *, float, float, int)>(
          U::Offsets.m_dwSharedRandomFloat);

  if (!pLocal || !pWeapon || !cmd)
    return;

  Vector vAngle = cmd->viewangles;

  // Visual Recoil Removal
  if (Vars::Visuals::Removals::NoVisualRecoil) {
    vAngle -= pLocal->GetPunchAngle();
  }

  U::Math.ClampAngles(vAngle);
  G::Util.FixMovement(vAngle, cmd);

  cmd->viewangles = vAngle;
}

bool CFeatures_NoSpread::ShouldRun(C_TerrorPlayer *pLocal,
                                   C_TerrorWeapon *pWeapon, CUserCmd *cmd) {
  if (!(cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_USE))
    return false;

  if (pLocal->m_isHangingFromLedge() || pLocal->m_isHangingFromTongue())
    return false;

  switch (pWeapon->GetWeaponID()) {
  case WEAPON_AK47:
  case WEAPON_AWP:
  case WEAPON_DEAGLE:
  case WEAPON_HUNTING_RIFLE:
  case WEAPON_M16A1:
  case WEAPON_M60:
  case WEAPON_MAC10:
  case WEAPON_MILITARY_SNIPER:
  case WEAPON_MP5:
  case WEAPON_PISTOL:
  case WEAPON_SCAR:
  case WEAPON_SCOUT:
  case WEAPON_SSG552:
  case WEAPON_UZI:
    return true;
  default:
    break;
  }

  return false;
}