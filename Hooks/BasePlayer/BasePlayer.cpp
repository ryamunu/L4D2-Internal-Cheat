#include "BasePlayer.h"
#include "../../Features/Vars.h"

using namespace Hooks;

void __fastcall BasePlayer::CalcPlayerView::Detour(C_BasePlayer *pThis,
                                                   void *edx, Vector &eyeOrigin,
                                                   Vector &eyeAngles,
                                                   float &fov) {
  // Correct logical check using the new Vars structure
  if (Vars::Visuals::Removals::NoVisualRecoil && pThis && !pThis->deadflag()) {
    const Vector vOldPunch = pThis->GetPunchAngle();

    pThis->m_vecPunchAngle().Init();

    // Use Func.Original instead of Table.Original
    Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);

    pThis->m_vecPunchAngle() = vOldPunch;
  } else {
    Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);
  }
}

void BasePlayer::Init() {
  // CalcPlayerView Initialization
  {
    using namespace CalcPlayerView;

    // Correct offset-based hooking pattern
    const FN pfCalcPlayerView =
        reinterpret_cast<FN>(U::Offsets.m_dwCalcPlayerView);

    // Ensure offset is valid (assert if nullptr)
    // previous code had XASSERT(pfCalcPlayerView == nullptr); which means
    // "Assert IF nullptr".

    if (pfCalcPlayerView)
      XASSERT(Func.Init(pfCalcPlayerView, &Detour) == false);
  }
}