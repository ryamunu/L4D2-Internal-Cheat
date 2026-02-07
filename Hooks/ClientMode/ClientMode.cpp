#include "ClientMode.h"
#include "../../Features/Aimbot/Aimbot.h"

#include "../../Features/AutoShove/AutoShove.h"
#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Sequence Freezing/SequenceFreezing.h"
#include "../../Features/Vars.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../SDK/EntityCache/entitycache.h"

using namespace Hooks;

bool __fastcall ClientMode::ShouldDrawFog::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}

bool __fastcall ClientMode::CreateMove::Detour(void *ecx, void *edx,
                                               float input_sample_frametime,
                                               CUserCmd *cmd) {
  if (!cmd || !cmd->command_number)
    return Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd);

  if (Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd))
    I::Prediction->SetLocalViewAngles(cmd->viewangles);

  gEntityCache.Clear();

  DWORD pep;
  __asm mov pep, ebp;
  // bool* BSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<char**>(pep)
  // - 0x1D);

  C_TerrorPlayer *pLocal =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())
          ->As<C_TerrorPlayer *>();

  if (pLocal && !pLocal->deadflag()) {
    gEntityCache.Fill();
    C_TerrorWeapon *pWeapon = pLocal->GetActiveWeapon()->As<C_TerrorWeapon *>();

    if (pWeapon) {
      // REMOVED: Teleport exploit (NaN viewangles) - causes instant kick/crash
      // on modern servers The following code was removed for safety: if
      // (Vars::Misc::Teleport) { cmd->viewangles = 3.4028235e38; ... }

      f::misc->run(pLocal, cmd); // run bhop before prediction for obv reasons
      F::EnginePrediction.Start(pLocal, cmd);
      {
        f::SequenceFreezing.Run(cmd, pLocal);
        f::autoshove.run(pLocal, cmd);
        F::Aimbot.Run(pLocal, cmd);
        F::NoSpread.Run(pLocal, pWeapon, cmd);
        F::Visuals.OnFrame();
      }
      F::EnginePrediction.Finish(pLocal, cmd);
      viewangle = cmd->viewangles;
      //*BSendPacket = Vars::DT::Shifted == Vars::Exploits::SpeedHackValue;
    }
  }

  return false;
}

void __fastcall ClientMode::DoPostScreenSpaceEffects::Detour(
    void *ecx, void *edx, const void *pSetup) {
  Table.Original<FN>(Index)(ecx, edx, pSetup);
}

float __fastcall ClientMode::GetViewModelFOV::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"

void __fastcall ClientMode::OverrideView::Detour(void *ecx, void *edx,
                                                 void *View) {
  return Table.Original<FN>(Index)(ecx, edx, View);

  // Table.Original<FN>(Index)(ecx, edx, View);
}
void ClientMode::Init() {
  XASSERT(Table.Init(I::ClientMode) == false);
  XASSERT(Table.Hook(&ShouldDrawFog::Detour, ShouldDrawFog::Index) == false);
  XASSERT(Table.Hook(&CreateMove::Detour, CreateMove::Index) == false);
  XASSERT(Table.Hook(&DoPostScreenSpaceEffects::Detour,
                     DoPostScreenSpaceEffects::Index) == false);
  XASSERT(Table.Hook(&GetViewModelFOV::Detour, GetViewModelFOV::Index) ==
          false);
  XASSERT(Table.Hook(&OverrideView::Detour, OverrideView::Index) == false);
}