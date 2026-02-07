#pragma once

#include "../../SDK/SDK.h"

class CFeatures_ESP {
public:
  void Render();
  void DrawSpectatorList(C_TerrorPlayer *pLocal);

private:
  bool GetBounds(C_BaseEntity *pBaseEntity, int &x, int &y, int &w, int &h);

  void Draw3DBox(C_BaseEntity *pEntity, const float flRotate, const Color clr);
  void OffScreenArrows(C_TerrorPlayer *pLocal);
  void DrawSkeleton(C_BaseEntity *pEntity, Color color);
  void DrawCrosshair();
};

namespace F {
inline CFeatures_ESP ESP;
}