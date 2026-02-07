#include "Misc.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../Vars.h"

void CMisc::Bunnyhop(CUserCmd *pCmd, C_TerrorPlayer *pLocal) {
  if (!Vars::Misc::Bunnyhop)
    return;

  // Chance Logic
  // If Chance is 100, always execute. If less, random roll.
  if (Vars::Misc::BhopChance < 100) {
    if ((rand() % 100) > Vars::Misc::BhopChance)
      return;
  }

  if (pLocal->m_MoveType() == MOVETYPE_LADDER ||
      pLocal->m_MoveType() == MOVETYPE_NOCLIP)
    return;

  if (!(pLocal->m_fFlags() & FL_ONGROUND)) {
    pCmd->buttons &= ~IN_JUMP;
  }
}

// AutoStrafe implementation can go here if needed, keeping it simple for now as
// requested.
void CMisc::AutoStrafe(CUserCmd *pCmd, C_TerrorPlayer *pLocal) {
  if (!Vars::Misc::AutoStrafe)
    return;
  if (pLocal->m_MoveType() == MOVETYPE_LADDER ||
      pLocal->m_MoveType() == MOVETYPE_NOCLIP)
    return;
  if (pLocal->m_fFlags() & FL_ONGROUND)
    return;

  // Simple Legit auto strafe
  if (pCmd->mousedx > 1 || pCmd->mousedx < -1) {
    pCmd->sidemove = pCmd->mousedx < 0.f ? -450.f : 450.f;
  }
}

// Thirdperson logic is mostly in BaseClient/FrameStageNotify for the camera
// override, but we might need to handle offset here if implemented per frame?
// The previous implementation used Misc::Thirdperson to handle offsets.
// We moved it to Visuals namespace.
void CMisc::Thirdperson() {
  if (!Vars::Visuals::Thirdperson || !I::Input)
    return;

  I::Input->m_vecCameraOffset().z =
      Vars::Visuals::HeightOffset; // Example usage
}

void CMisc::movement(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  Bunnyhop(pCmd, pLocal);
  AutoStrafe(pCmd, pLocal);
}

void CMisc::run(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  if (!pLocal || !pCmd)
    return;

  movement(pLocal, pCmd);
  Thirdperson();
  // Other misc features could go here (RankRevealer usually just runs on
  // drawing or event)
}