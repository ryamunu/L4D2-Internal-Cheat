#pragma once
#include "../../SDK/SDK.h"

class CFeatures_Aimbot {
private:
  int GetBestTarget(C_TerrorPlayer *pLocal, CUserCmd *pCmd);
  bool GetBestHitbox(C_TerrorPlayer *pLocal, C_TerrorPlayer *pTarget,
                     Vector &vOut);
  bool IsVisible(C_TerrorPlayer *pLocal, const Vector &vStart,
                 const Vector &vEnd, IClientEntity *pTarget);
  float GetLerpTime();

public:
  void Run(C_TerrorPlayer *pLocal, CUserCmd *pCmd);
};

namespace F {
inline CFeatures_Aimbot Aimbot;
}
