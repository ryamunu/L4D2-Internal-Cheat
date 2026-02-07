#include "BaseClient.h"
#include "../../Features/Vars.h"
#include "../../SDK/EntityCache/entitycache.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../ClientMode/ClientMode.h"
#include "../TerrorPlayer/TerrorPlayer.h"

using namespace Hooks;

void __fastcall BaseClient::LevelInitPreEntity::Detour(void *ecx, void *edx,
                                                       char const *pMapName) {
  Table.Original<FN>(Index)(ecx, edx, pMapName);
}

void __fastcall BaseClient::LevelInitPostEntity::Detour(void *ecx, void *edx) {
  Table.Original<FN>(Index)(ecx, edx);
}

void __fastcall BaseClient::LevelShutdown::Detour(void *ecx, void *edx) {
  Table.Original<FN>(Index)(ecx, edx);
  gEntityCache.Clear();
}

void __fastcall BaseClient::FrameStageNotify::Detour(
    void *ecx, void *edx, ClientFrameStage_t curStage) {
  switch (curStage) {
  case ClientFrameStage_t::FRAME_RENDER_START: {
    auto set_thirdperson_state = [](const bool &state, const bool &alive) {
      if (alive) {
        I::Input->m_fCameraInThirdPerson() = state;
        I::Cvars->FindVar("cam_idealdist")
            ->SetValue(Vars::Visuals::ThirdpersonDist);
        I::Cvars->FindVar("cam_collision")->SetValue(true);
        I::Cvars->FindVar("cam_snapto")->SetValue(true);
        I::Cvars->FindVar("c_thirdpersonshoulder")->SetValue(true);
        I::Cvars->FindVar("c_thirdpersonshoulderaimdist")->SetValue(0.f);
        I::Cvars->FindVar("c_thirdpersonshoulderdist")->SetValue(0.f);
        I::Cvars->FindVar("c_thirdpersonshoulderheight")->SetValue(0.f);
        I::Cvars->FindVar("c_thirdpersonshoulderoffset")->SetValue(0);
      } else {
        I::Input->m_fCameraInThirdPerson() = false;
      }
    };

    static bool bPressed = false;
    static bool bPrevState = false; // Track the previous state of the key

    if (GetAsyncKeyState(Vars::Misc::ThirdpersonKey.m_Var) &
        0x8000) // Key is currently pressed
    {
      if (!bPrevState) // Only toggle when the key was not previously pressed
      {
        Vars::Visuals::Thirdperson = !Vars::Visuals::Thirdperson;
      }
      bPrevState = true;
    } else {
      bPrevState = false; // Reset when the key is released
    }

    set_thirdperson_state(Vars::Visuals::Thirdperson, true);

    break;
  }
  default:
    break;
  }

  Table.Original<FN>(Index)(ecx, edx, curStage);

  switch (curStage) {
  case ClientFrameStage_t::FRAME_NET_UPDATE_START: {
    gEntityCache.Clear();
    break;
  }
  case ClientFrameStage_t::FRAME_NET_UPDATE_END: {
    C_TerrorPlayer *pLocal =
        I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())
            ->As<C_TerrorPlayer *>();
    if (!pLocal || !pLocal->m_iTeamNum())
      return;
    gEntityCache.Fill();
    break;
  }
  }
}

void BaseClient::Init() {
  XASSERT(Table.Init(I::BaseClient) == false);
  XASSERT(Table.Hook(&LevelInitPreEntity::Detour, LevelInitPreEntity::Index) ==
          false);
  XASSERT(Table.Hook(&LevelInitPostEntity::Detour,
                     LevelInitPostEntity::Index) == false);
  XASSERT(Table.Hook(&LevelShutdown::Detour, LevelShutdown::Index) == false);
  XASSERT(Table.Hook(&FrameStageNotify::Detour, FrameStageNotify::Index) ==
          false);
}